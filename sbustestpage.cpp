#include "sbustestpage.h"
#include "ui_page_serial_sbustest.h"
#include "serialcommunicator.h"
#include "sbusprotocolparser.h"
#include "crsfchannelwidget.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QTextCursor>
#include <QDebug>
#include <QThread>
#include <QDir>
#include <QCoreApplication>
#include <QHeaderView>
#include <QFont>
#include <cstring>

// =====================================================================
// SbusTestPage implementation
// =====================================================================

SbusTestPage::SbusTestPage(SerialCommunicator *comm, QWidget *parent)
    : QWidget(parent), ui(new Ui::PageSerialSbusTest), m_comm(comm)
{
    m_pageWidget = new QWidget();
    ui->setupUi(m_pageWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pageWidget);

    // SBUS baud rates (100000 is standard for SBUS)
    QStringList baudRates = {
        "100000", "9600", "19200", "38400", "57600", "115200",
        "230400", "420000", "460800", "921600"
    };
    ui->comboBaud->addItems(baudRates);
    ui->comboBaud->setCurrentText("100000");

    // Signal connections
    connect(ui->btnRefresh, &QPushButton::clicked, this, &SbusTestPage::onRefreshPorts);
    connect(ui->btnConnect, &QPushButton::clicked, this, &SbusTestPage::onToggleConnection);
    connect(ui->btnSend, &QPushButton::clicked, this, &SbusTestPage::onSendData);
    connect(ui->btnClearRecv, &QPushButton::clicked, this, &SbusTestPage::onClearRecv);
    connect(ui->btnClearRecvData, &QPushButton::clicked, this, &SbusTestPage::onClearRecv);
    connect(ui->btnSaveRecv, &QPushButton::clicked, this, &SbusTestPage::onSaveRecv);
    connect(ui->btnResetStats, &QPushButton::clicked, this, &SbusTestPage::onResetStats);
    connect(ui->btnTplChannels, &QPushButton::clicked, this, &SbusTestPage::onTplChannels);
    connect(ui->btnTplCustom, &QPushButton::clicked, this, &SbusTestPage::onTplCustom);

    connect(ui->spinStopAt, SIGNAL(valueChanged(int)), this, SLOT(onStopAtChanged(int)));
    connect(ui->btnStopAtEnable, &QPushButton::toggled, this, &SbusTestPage::onStopAtToggle);
    ui->spinStopAt->setToolTip("到达帧数上限后自动断开");

    connect(m_comm, &SerialCommunicator::statusChanged, this, &SbusTestPage::onSerialStatus);
    connect(m_comm, &SerialCommunicator::dataReceived, this, &SbusTestPage::onDataReceived);

    m_stopAtFrameCount = 0;
    m_stoppedByLimit = false;
    ui->btnStopAtEnable->setChecked(false);
    m_rxTotalBytes = 0;
    m_validFrames = 0;
    m_invalidFrames = 0;

    onRefreshPorts();
    onResetStats();

    // default send: neutral channels frame
    onTplChannels();

    // SBUS raw log init
    initLogFile();
    m_logFlushTimer = new QTimer(this);
    m_logFlushTimer->setSingleShot(false);
    m_logFlushTimer->setInterval(200);
    connect(m_logFlushTimer, &QTimer::timeout, this, &SbusTestPage::flushLogBuffer);
    m_logFlushTimer->start();

    // Frame alignment retry timer: SBUS frames can span multiple serial
    // callbacks; periodically re-scan the buffer to pick up a completed frame.
    m_processTimer = new QTimer(this);
    m_processTimer->setSingleShot(false);
    m_processTimer->setInterval(10);
    connect(m_processTimer, &QTimer::timeout, this, &SbusTestPage::onProcessTimer);
    m_processTimer->start();

    // Analyzer UI panels (channel waveform + packet list + flags)
    initAnalyzerUI();
}

SbusTestPage::~SbusTestPage()
{
    if (m_logFlushTimer) m_logFlushTimer->stop();
    if (m_processTimer) m_processTimer->stop();
    closeLogFile();
    delete ui;
}

// =====================================================================
// Async log
// =====================================================================
void SbusTestPage::initLogFile()
{
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDir);
    m_logFilePath = logDir + "/sbus_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".bin";
    m_logFile = new QFile(m_logFilePath, this);
    if (!m_logFile->open(QIODevice::WriteOnly)) {
        qWarning("SbusLog: failed to open %s", qPrintable(m_logFile->errorString()));
        m_logEnabled = false;
        return;
    }
    m_logEnabled = true;
    m_logBytesWritten = 0;
    m_logQueue.clear();
    qInfo("SbusLog: started -> %s", qPrintable(m_logFilePath));
}

void SbusTestPage::closeLogFile()
{
    if (m_logFile && m_logFile->isOpen()) {
        flushLogBuffer();
        m_logFile->close();
        qInfo("SbusLog: closed, %lld bytes -> %s", m_logBytesWritten, qPrintable(m_logFilePath));
    }
    m_logEnabled = false;
}

void SbusTestPage::enqueueLogLine(const QString &line) { Q_UNUSED(line); }

void SbusTestPage::enqueueRawFrame(const QByteArray &frame)
{
    if (!m_logEnabled || !m_logFile || !m_logFile->isOpen()) return;
    m_logQueue.enqueue(frame);
    if (m_logQueue.size() > 10000) {
        int drop = m_logQueue.size() / 10;
        for (int i = 0; i < drop; ++i) m_logQueue.dequeue();
    }
}

void SbusTestPage::flushLogBuffer()
{
    if (!m_logEnabled || !m_logFile || !m_logFile->isOpen()) return;
    if (m_logQueue.isEmpty()) return;
    QByteArray batch;
    while (!m_logQueue.isEmpty()) batch.append(m_logQueue.dequeue());
    qint64 w = m_logFile->write(batch);
    if (w > 0) m_logBytesWritten += w;
    m_logFile->flush();
}

// =====================================================================
// Serial port ops
// =====================================================================
void SbusTestPage::onRefreshPorts()
{
    QString current = ui->comboPort->currentText();
    ui->comboPort->clear();
    for (const auto &info : QSerialPortInfo::availablePorts())
        ui->comboPort->addItem(info.portName());
    if (!current.isEmpty()) {
        int idx = ui->comboPort->findText(current);
        if (idx >= 0) ui->comboPort->setCurrentIndex(idx);
    }
}

void SbusTestPage::onToggleConnection()
{
    if (m_comm->isConnected()) {
        m_comm->disconnect();
        updateConnectionState();
    } else {
        QString port = ui->comboPort->currentText();
        if (port.isEmpty()) {
            QMessageBox::warning(this, tr("Prompt"), tr("请先选择端口"));
            return;
        }
        int baud = ui->comboBaud->currentText().toInt();
        if (m_comm->connectToPort(port, baud))
            updateConnectionState();
    }
}

void SbusTestPage::updateConnectionState()
{
    bool connected = m_comm->isConnected();
    ui->btnConnect->setText(connected ? tr("断开") : tr("连接"));
    ui->labelStatus->setText(connected ? tr("已连接") : tr("未连接"));
    ui->labelStatus->setStyleSheet(connected
        ? "color:green;font-weight:bold;font-size:12pt;"
        : "color:red;font-weight:bold;font-size:12pt;");
}

void SbusTestPage::onSerialStatus(const QString &msg)
{
    updateConnectionState();
    emit serialStatusChanged(msg);
}

// =====================================================================
// Send
// =====================================================================
void SbusTestPage::onSendData()
{
    if (!m_comm->isConnected()) {
        QMessageBox::warning(this, tr("Prompt"), tr("请先连接串口"));
        return;
    }
    QString input = ui->textSend->toPlainText().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, tr("Prompt"), tr("请输入要发送的数据"));
        return;
    }
    QString hex = input.simplified().remove(' ');
    QByteArray data = QByteArray::fromHex(hex.toLatin1());
    if (data.isEmpty()) {
        QMessageBox::warning(this, tr("Prompt"), tr("无效的十六进制数据"));
        return;
    }

    int count = ui->spinRepeatCount->value();
    int interval = ui->spinRepeatInterval->value();
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    for (int i = 0; i < count; ++i) {
        m_comm->sendData(data);
        enqueueRawFrame(data);
        ui->textRecv->append(QString("[TX] [%1] %2")
                                 .arg(timestamp)
                                 .arg(QString::fromLatin1(data.toHex(' ').toUpper())));
        if (i < count - 1 && interval > 0)
            QThread::msleep(interval);
    }
    if (ui->checkAutoScroll->isChecked()) {
        QTextCursor c = ui->textRecv->textCursor();
        c.movePosition(QTextCursor::End);
        ui->textRecv->setTextCursor(c);
    }
}

void SbusTestPage::onTplChannels()
{
    // Build a neutral 16-channel SBUS frame and fill the send box
    uint16_t ch[16];
    for (int i = 0; i < 16; ++i) ch[i] = SBUS_CH_MID;  // 992 neutral
    QByteArray frame = SbusProtocolParser::buildFrame(ch);
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void SbusTestPage::onTplCustom()
{
    // Example frame raw (from a capture): start 0x0F + data + flags 0x00 + end 0x00
    uint16_t ch[16] = {1000,1500,1000,1500,1000,1500,1000,1500,
                       1000,1500,1000,1500,1000,1500,1000,1500};
    QByteArray frame = SbusProtocolParser::buildFrame(ch);
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void SbusTestPage::onClearRecv() { ui->textRecv->clear(); }

void SbusTestPage::onSaveRecv()
{
    QString path = QFileDialog::getSaveFileName(this, tr("保存接收数据"), "sbus_recv.txt", tr("文本文件 (*.txt)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(ui->textRecv->toPlainText().toUtf8());
}

void SbusTestPage::onResetStats()
{
    m_packetCount = 0;
    m_rxTotalBytes = 0;
    m_validFrames = 0;
    m_invalidFrames = 0;
    m_intervalSum = 0;
    m_intervalMin = 999999;
    m_intervalMax = 0;
    m_lastRxTime = 0;
    if (m_analyzerText) m_analyzerText->clear();
    if (m_lblAnalyzerPktCount) m_lblAnalyzerPktCount->setText("包数: 0");
    if (m_lblIntervalPattern) m_lblIntervalPattern->setText("模式: 等待数据...");
    if (m_lblIntervalMin) m_lblIntervalMin->setText("最小间隔: --");
    if (m_lblIntervalMax) m_lblIntervalMax->setText("最大间隔: --");
    if (m_lblIntervalAvg) m_lblIntervalAvg->setText("平均间隔: --");
    updateIntervalDisplay();
}

void SbusTestPage::onStopAtChanged(int value) { m_stopAtFrameCount = value; }

void SbusTestPage::onStopAtToggle(bool checked)
{
    if (checked) m_stoppedByLimit = false;
}

// =====================================================================
// Receive data handling
// =====================================================================
void SbusTestPage::onDataReceived(const QByteArray &data)
{
    if (data.isEmpty()) return;
    m_rxBuffer.append(data);
    m_rxTotalBytes += data.size();
    if (m_lblRxBytes) m_lblRxBytes->setText(QString("接收: %1 bytes").arg(m_rxTotalBytes));

    processRxData();
}

void SbusTestPage::processRxData()
{
    while (m_rxBuffer.size() >= SBUS_FRAME_SIZE) {
        const quint8 *raw = reinterpret_cast<const quint8*>(m_rxBuffer.constData());
        int bufSize = m_rxBuffer.size();

        // Find a valid start byte
        int frameStart = -1;
        for (int i = 0; i < bufSize; ++i) {
            if (SbusProtocolParser::isStartByte(raw[i])
                && i + SBUS_FRAME_SIZE <= bufSize) {
                // verify end byte matches expected polarity
                quint8 end = raw[i + SBUS_FRAME_SIZE - 1];
                bool okEnd = (raw[i] == SBUS_START_BYTE && end == SBUS_END_BYTE)
                          || (raw[i] == SBUS_START_BYTE_INV && end == SBUS_END_BYTE_INV);
                if (okEnd) { frameStart = i; break; }
            }
        }
        if (frameStart < 0) {
            // 没有完整可解析的 SBUS 帧。
            // 如果缓冲已经累积很多字节却始终解不出标准帧，说明很可能是
            // 波特率不对或设备输出的不是标准 SBUS（25字节）——给出诊断提示，
            // 并清空缓冲避免无限累积（不再打印原始大块刷屏）。
            if (bufSize >= SBUS_FRAME_SIZE * 3) {
                if (!m_warnedNonSbus) {
                    m_warnedNonSbus = true;
                    QString tip = QString("[!] 已收到 %1 字节，但无法解析为标准的25字节 SBUS 帧。\n"
                                          "    请检查：① 波特率是否为 100000 (SBUS专用)；\n"
                                          "    ② 设备输出的是否为 SBUS（而非 iBUS/CRSF 等）；\n"
                                          "    ③ 接线/信号反相是否正确。")
                                      .arg(m_rxTotalBytes);
                    ui->textRecv->append(tip);
                    enqueueLogLine(tip);
                }
                m_rxBuffer.clear();
            } else {
                // 数据还少，保留缓冲等后续补齐；只丢弃确定是杂波的前导字节
                int keepFrom = -1;
                for (int i = qMax(0, bufSize - SBUS_FRAME_SIZE + 1); i < bufSize; ++i) {
                    if (SbusProtocolParser::isStartByte(raw[i])) { keepFrom = i; break; }
                }
                if (keepFrom > 0) m_rxBuffer.remove(0, keepFrom);
            }
            break;
        }

        QByteArray frame = m_rxBuffer.mid(frameStart, SBUS_FRAME_SIZE);
        ParsedSbusFrame parsed = SbusProtocolParser::parseFrame(frame);
        m_rxBuffer.remove(0, frameStart + SBUS_FRAME_SIZE);

        // stop-at
        if (ui->btnStopAtEnable->isChecked() && !m_stoppedByLimit) {
            if (m_validFrames + m_invalidFrames + 1 >= m_stopAtFrameCount) {
                m_stoppedByLimit = true;
                if (m_comm->isConnected()) m_comm->disconnect();
                ui->textRecv->append(QString("[!] 已到达帧数上限 %1，自动断开").arg(m_stopAtFrameCount));
            }
        }

        if (parsed.valid) {
            m_validFrames++;
            m_packetCount++;
            m_warnedNonSbus = false;   // 解出有效帧，说明协议对了，复位诊断提示

            qint64 now = QDateTime::currentMSecsSinceEpoch();
            qint64 intervalMs = (m_lastRxTime > 0) ? (now - m_lastRxTime) : 0;
            m_lastRxTime = now;
            if (intervalMs > 0) {
                m_intervalSum += intervalMs;
                if (intervalMs < m_intervalMin) m_intervalMin = intervalMs;
                if (intervalMs > m_intervalMax) m_intervalMax = intervalMs;
            }

            // update channel data + flags
            memcpy(m_currentChannels, parsed.channelsPwm, sizeof(m_currentChannels));
            m_hasRcData = true;
            updateChannelWidget();

            // analyzer line
            appendAnalyzerPacket(m_packetCount, parsed, intervalMs);

            // 完整解析包打印到接收栏：一包 = 一个完整 SBUS 帧（25字节）
            // 格式规整为两行：帧信息行 + 16通道值行，方便逐包查看
            QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            QString chRaw;
            for (int i = 0; i < 16; ++i)
                chRaw += QString("%1").arg(parsed.channels[i], 4) + " ";
            QString parsedLine = QString("[%1] ▸ 第%2包 SBUS(极:%3) 间隔%4ms 标志=0x%5 ch17=%6 ch18=%7 丢帧=%8 失败保护=%9")
                                     .arg(ts)
                                     .arg(m_packetCount, 5)
                                     .arg(parsed.inverted ? "反相" : "正相")
                                     .arg(intervalMs)
                                     .arg(parsed.flagsByte, 2, 16, QLatin1Char('0'))
                                     .arg(parsed.ch17).arg(parsed.ch18)
                                     .arg(parsed.frameLost).arg(parsed.failsafe);
            QString chLine = QString("      CH: %1| %2| %3| %4| %5| %6| %7| %8")
                                 .arg(parsed.channels[0], 4).arg(parsed.channels[1], 4)
                                 .arg(parsed.channels[2], 4).arg(parsed.channels[3], 4)
                                 .arg(parsed.channels[4], 4).arg(parsed.channels[5], 4)
                                 .arg(parsed.channels[6], 4).arg(parsed.channels[7], 4)
                             + QString("\n      CH: %1| %2| %3| %4| %5| %6| %7| %8")
                                 .arg(parsed.channels[8], 4).arg(parsed.channels[9], 4)
                                 .arg(parsed.channels[10], 4).arg(parsed.channels[11], 4)
                                 .arg(parsed.channels[12], 4).arg(parsed.channels[13], 4)
                                 .arg(parsed.channels[14], 4).arg(parsed.channels[15], 4);
            QString hexLine = QString("      HEX: %1").arg(parsed.hexDump);
            ui->textRecv->append(parsedLine);
            ui->textRecv->append(chLine);
            ui->textRecv->append(hexLine);
            ui->textRecv->append(QString());   // 空行分隔，让每包清晰独立
            enqueueLogLine(parsedLine);
            enqueueLogLine(chLine);
            enqueueLogLine(hexLine);

            // flag byte status
            if (m_lblFlags) {
                QString s = QString("标志: ch17=%1 ch18=%2 丢帧=%3 失败保护=%4")
                                .arg(parsed.ch17).arg(parsed.ch18)
                                .arg(parsed.frameLost).arg(parsed.failsafe);
                m_lblFlags->setText(s);
                m_lblFlags->setStyleSheet(parsed.failsafe || parsed.frameLost
                    ? "color:red;font-weight:bold;"
                    : "color:green;");
            }
            enqueueRawFrame(frame);
        } else {
            m_invalidFrames++;
        }
    }
    updateIntervalDisplay();
}

// =====================================================================
// Periodic re-scan: SBUS frames often span multiple serial callbacks.
// The buffer may hold a frame whose start byte arrived but whose tail is
// still pending; re-run processRxData so a just-completed frame is picked
// up even if onDataReceived isn't triggered again right away.
// =====================================================================
void SbusTestPage::onProcessTimer()
{
    // Only re-scan when we have data buffered and not already mid-parse.
    if (m_rxBuffer.size() >= SBUS_FRAME_SIZE)
        processRxData();
}

// =====================================================================
// Analyzer UI (built in code, appended into the .ui page layout)
// =====================================================================
void SbusTestPage::initAnalyzerUI()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(m_pageWidget->layout());
    if (!mainLayout) return;

    // ---- 1. Packet analyzer panel ----
    m_groupAnalyzer = new QGroupBox("SBUS 协议分析仪", this);
    QVBoxLayout *analyzerLayout = new QVBoxLayout(m_groupAnalyzer);

    QHBoxLayout *intervalLayout = new QHBoxLayout();
    m_lblIntervalPattern = new QLabel("模式: 等待数据...", this);
    m_lblIntervalPattern->setStyleSheet("font-weight:bold;color:#00BFFF;");
    m_lblAnalyzerPktCount = new QLabel("包数: 0", this);
    m_lblIntervalMin = new QLabel("最小间隔: --", this);
    m_lblIntervalMax = new QLabel("最大间隔: --", this);
    m_lblIntervalAvg = new QLabel("平均间隔: --", this);
    m_lblRxBytes = new QLabel("接收: 0 bytes", this);
    m_lblRxBytes->setStyleSheet("color:#E67E22;font-weight:bold;");
    intervalLayout->addWidget(m_lblIntervalPattern);
    intervalLayout->addWidget(m_lblAnalyzerPktCount);
    intervalLayout->addWidget(m_lblIntervalMin);
    intervalLayout->addWidget(m_lblIntervalMax);
    intervalLayout->addWidget(m_lblIntervalAvg);
    intervalLayout->addWidget(m_lblRxBytes);
    intervalLayout->addStretch();

    QPushButton *btnClearAnalyzer = new QPushButton("清空分析", this);
    connect(btnClearAnalyzer, &QPushButton::clicked, this, &SbusTestPage::onResetStats);
    intervalLayout->addWidget(btnClearAnalyzer);
    analyzerLayout->addLayout(intervalLayout);

    m_analyzerText = new QTextEdit(this);
    m_analyzerText->setReadOnly(true);
    m_analyzerText->setFont(QFont("Courier New", 9));
    m_analyzerText->setMinimumHeight(120);
    m_analyzerText->setMaximumHeight(220);
    m_analyzerText->setStyleSheet("QTextEdit { background-color:#1E1E1E; color:#D4D4D4; }");
    analyzerLayout->addWidget(m_analyzerText);

    mainLayout->insertWidget(mainLayout->count(), m_groupAnalyzer);

    // ---- 2. Flags panel ----
    m_groupFlags = new QGroupBox("SBUS 标志位 (Flag Byte)", this);
    QVBoxLayout *flagsLayout = new QVBoxLayout(m_groupFlags);
    m_lblFlags = new QLabel("标志: 等待数据...", this);
    m_lblFlags->setStyleSheet("font-weight:bold;");
    flagsLayout->addWidget(m_lblFlags);
    mainLayout->insertWidget(mainLayout->count(), m_groupFlags);

    // ---- 3. Channel waveform panel ----
    m_groupChannels = new QGroupBox("RC 通道 (16ch, PWM us)", this);
    QVBoxLayout *chLayout = new QVBoxLayout(m_groupChannels);

    m_channelWidget = new CrsfChannelWidget(this);
    m_channelWidget->setMinimumHeight(200);
    chLayout->addWidget(m_channelWidget);

    m_channelTable = new QTableWidget(2, 8, this);
    m_channelTable->setFont(QFont("Courier New", 9));
    m_channelTable->setMinimumHeight(60);
    m_channelTable->setMaximumHeight(80);
    m_channelTable->horizontalHeader()->setStretchLastSection(true);
    m_channelTable->verticalHeader()->setVisible(false);
    QStringList hLabels;
    for (int i = 1; i <= 8; ++i) hLabels << QString("CH%1").arg(i);
    m_channelTable->setHorizontalHeaderLabels(hLabels);
    m_channelTable->setVerticalHeaderLabels({"1-8", "9-16"});
    m_channelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_channelTable->setSelectionMode(QAbstractItemView::NoSelection);
    for (int row = 0; row < 2; ++row)
        for (int col = 0; col < 8; ++col) {
            int chIdx = row * 8 + col;
            if (chIdx < 16) {
                QTableWidgetItem *item = new QTableWidgetItem("1500");
                item->setTextAlignment(Qt::AlignCenter);
                m_channelTable->setItem(row, col, item);
            }
        }
    chLayout->addWidget(m_channelTable);
    mainLayout->insertWidget(mainLayout->count(), m_groupChannels);
}

void SbusTestPage::appendAnalyzerPacket(int seq, const ParsedSbusFrame &parsed, qint64 intervalMs)
{
    if (!m_analyzerText) return;

    // Detect channel activity: any channel moved from neutral?
    bool moving = false;
    for (int i = 0; i < 16; ++i) {
        if (parsed.channels[i] < SBUS_CH_MID - 5 || parsed.channels[i] > SBUS_CH_MID + 5) {
            moving = true;
            break;
        }
    }
    QString act = moving ? "●" : "○";

    QString line = QString("[%1] #%2 %3 间隔:%4ms 极:%5 %6")
                       .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                       .arg(seq, 5)
                       .arg(act)
                       .arg(intervalMs)
                       .arg(parsed.inverted ? "反相" : "正相")
                       .arg(parsed.hexDump);
    m_analyzerText->append(line);
    if (m_analyzerText->document()->blockCount() > 500)
        m_analyzerText->clear();

    if (m_lblAnalyzerPktCount)
        m_lblAnalyzerPktCount->setText(QString("包数: %1").arg(seq));
    if (m_lblIntervalPattern) {
        // ~14ms = 70Hz typical SBUS frame rate
        if (m_packetCount > 10 && m_intervalSum / m_packetCount > 0) {
            double avg = static_cast<double>(m_intervalSum) / m_packetCount;
            double hz = 1000.0 / avg;
            m_lblIntervalPattern->setText(QString("模式: %1ms ≈ %2Hz")
                                              .arg(avg, 0, 'f', 1).arg(hz, 0, 'f', 0));
        }
    }
}

void SbusTestPage::updateChannelWidget()
{
    if (!m_channelWidget || !m_hasRcData) return;
    m_channelWidget->updateChannels(m_currentChannels);

    if (m_channelTable) {
        for (int row = 0; row < 2; ++row)
            for (int col = 0; col < 8; ++col) {
                int chIdx = row * 8 + col;
                if (chIdx < 16 && m_channelTable->item(row, col)) {
                    m_channelTable->item(row, col)->setText(QString::number(m_currentChannels[chIdx]));
                }
            }
    }
}

void SbusTestPage::updateIntervalDisplay()
{
    if (!m_lblIntervalMin) return;
    if (m_packetCount > 1) {
        m_lblIntervalMin->setText(QString("最小间隔: %1ms").arg(m_intervalMin));
        m_lblIntervalMax->setText(QString("最大间隔: %1ms").arg(m_intervalMax));
        m_lblIntervalAvg->setText(QString("平均间隔: %1ms").arg(m_intervalSum / m_packetCount));
    }
}
