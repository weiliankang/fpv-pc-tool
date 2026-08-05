#include "mavlinktestpage.h"
#include "ui_page_serial_mavlinktest.h"
#include "serialcommunicator.h"
#include "mavlinkprotocolparser.h"

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

// =====================================================================
// MavlinkTestPage implementation
// =====================================================================

MavlinkTestPage::MavlinkTestPage(SerialCommunicator *comm, QWidget *parent)
    : QWidget(parent), ui(new Ui::PageSerialMavlinkTest), m_comm(comm)
{
    m_pageWidget = new QWidget();
    ui->setupUi(m_pageWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pageWidget);

    // MAVLink baud rates (115200 is the default)
    QStringList baudRates = {
        "115200", "57600", "38400", "9600", "19200",
        "230400", "460800", "921600", "100000"
    };
    ui->comboBaud->addItems(baudRates);
    ui->comboBaud->setCurrentText("115200");

    // Signal connections
    connect(ui->btnRefresh, &QPushButton::clicked, this, &MavlinkTestPage::onRefreshPorts);
    connect(ui->btnConnect, &QPushButton::clicked, this, &MavlinkTestPage::onToggleConnection);
    connect(ui->btnSend, &QPushButton::clicked, this, &MavlinkTestPage::onSendData);
    connect(ui->btnClearRecv, &QPushButton::clicked, this, &MavlinkTestPage::onClearRecv);
    connect(ui->btnClearRecvData, &QPushButton::clicked, this, &MavlinkTestPage::onClearRecv);
    connect(ui->btnSaveRecv, &QPushButton::clicked, this, &MavlinkTestPage::onSaveRecv);
    connect(ui->btnResetStats, &QPushButton::clicked, this, &MavlinkTestPage::onResetStats);
    connect(ui->btnTplHeartbeat, &QPushButton::clicked, this, &MavlinkTestPage::onTplHeartbeat);
    connect(ui->btnTplCustom, &QPushButton::clicked, this, &MavlinkTestPage::onTplCustom);

    connect(ui->spinStopAt, SIGNAL(valueChanged(int)), this, SLOT(onStopAtChanged(int)));
    connect(ui->btnStopAtEnable, &QPushButton::toggled, this, &MavlinkTestPage::onStopAtToggle);

    connect(m_comm, &SerialCommunicator::statusChanged, this, &MavlinkTestPage::onSerialStatus);
    connect(m_comm, &SerialCommunicator::dataReceived, this, &MavlinkTestPage::onDataReceived);

    m_stopAtFrameCount = 0;
    m_stoppedByLimit = false;
    ui->btnStopAtEnable->setChecked(false);
    m_rxTotalBytes = 0;
    m_validFrames = 0;
    m_crcOkFrames = 0;
    m_crcBadFrames = 0;

    onRefreshPorts();
    onResetStats();

    // default send: heartbeat
    onTplHeartbeat();

    // raw log init
    initLogFile();
    m_logFlushTimer = new QTimer(this);
    m_logFlushTimer->setSingleShot(false);
    m_logFlushTimer->setInterval(200);
    connect(m_logFlushTimer, &QTimer::timeout, this, &MavlinkTestPage::flushLogBuffer);
    m_logFlushTimer->start();

    // analyzer UI
    initAnalyzerUI();
}

MavlinkTestPage::~MavlinkTestPage()
{
    if (m_logFlushTimer) m_logFlushTimer->stop();
    closeLogFile();
    delete ui;
}

// =====================================================================
// Async log
// =====================================================================
void MavlinkTestPage::initLogFile()
{
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDir);
    m_logFilePath = logDir + "/mavlink_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".bin";
    m_logFile = new QFile(m_logFilePath, this);
    if (!m_logFile->open(QIODevice::WriteOnly)) {
        qWarning("MavLog: failed to open %s", qPrintable(m_logFile->errorString()));
        m_logEnabled = false;
        return;
    }
    m_logEnabled = true;
    m_logBytesWritten = 0;
    m_logQueue.clear();
    qInfo("MavLog: started -> %s", qPrintable(m_logFilePath));
}

void MavlinkTestPage::closeLogFile()
{
    if (m_logFile && m_logFile->isOpen()) {
        flushLogBuffer();
        m_logFile->close();
        qInfo("MavLog: closed, %lld bytes -> %s", m_logBytesWritten, qPrintable(m_logFilePath));
    }
    m_logEnabled = false;
}

void MavlinkTestPage::enqueueRawFrame(const QByteArray &frame)
{
    if (!m_logEnabled || !m_logFile || !m_logFile->isOpen()) return;
    m_logQueue.enqueue(frame);
    if (m_logQueue.size() > 10000) {
        int drop = m_logQueue.size() / 10;
        for (int i = 0; i < drop; ++i) m_logQueue.dequeue();
    }
}

void MavlinkTestPage::flushLogBuffer()
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
void MavlinkTestPage::onRefreshPorts()
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

void MavlinkTestPage::onToggleConnection()
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

void MavlinkTestPage::updateConnectionState()
{
    bool connected = m_comm->isConnected();
    ui->btnConnect->setText(connected ? tr("断开") : tr("连接"));
    ui->labelStatus->setText(connected ? tr("已连接") : tr("未连接"));
    ui->labelStatus->setStyleSheet(connected
        ? "color:green;font-weight:bold;font-size:12pt;"
        : "color:red;font-weight:bold;font-size:12pt;");
}

void MavlinkTestPage::onSerialStatus(const QString &msg)
{
    updateConnectionState();
    emit serialStatusChanged(msg);
}

// =====================================================================
// Send
// =====================================================================
void MavlinkTestPage::onSendData()
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

void MavlinkTestPage::onTplHeartbeat()
{
    // MAVLink v1 HEARTBEAT (msgid 0), minimal valid frame
    // FE 09 00 01 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00  [CRC computed]
    QByteArray payload(9, 0x00);
    QByteArray frame;
    frame.append(static_cast<char>(MAVLINK_STX_V1)); // 0xFE
    frame.append(static_cast<char>(payload.size())); // len = 9
    frame.append(static_cast<char>(0));              // seq
    frame.append(static_cast<char>(1));              // sysid
    frame.append(static_cast<char>(1));              // compid
    frame.append(static_cast<char>(0));              // msgid (HEARTBEAT)
    frame.append(payload);
    // CRC: header(5 bytes, excl stx) + payload
    quint16 crc = MavlinkProtocolParser::crc16(
        reinterpret_cast<const quint8*>(frame.constData()) + 1, 5 + payload.size());
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void MavlinkTestPage::onTplCustom()
{
    // MAVLink v2 example (stx 0xFD): len=0, empty payload heartbeat-like
    QByteArray frame;
    frame.append(static_cast<char>(MAVLINK_STX_V2)); // 0xFD
    frame.append(static_cast<char>(0));              // len
    frame.append(static_cast<char>(0));              // incompat
    frame.append(static_cast<char>(0));              // compat
    frame.append(static_cast<char>(0));              // seq
    frame.append(static_cast<char>(1));              // sysid
    frame.append(static_cast<char>(1));              // compid
    frame.append(static_cast<char>(0));              // msgid lo
    frame.append(static_cast<char>(0));              // msgid hi
    quint16 crc = MavlinkProtocolParser::crc16(
        reinterpret_cast<const quint8*>(frame.constData()) + 1, 9);
    frame.append(static_cast<char>(crc & 0xFF));
    frame.append(static_cast<char>((crc >> 8) & 0xFF));
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void MavlinkTestPage::onClearRecv() { ui->textRecv->clear(); }

void MavlinkTestPage::onSaveRecv()
{
    QString path = QFileDialog::getSaveFileName(this, tr("保存接收数据"), "mavlink_recv.txt", tr("文本文件 (*.txt)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(ui->textRecv->toPlainText().toUtf8());
}

void MavlinkTestPage::onResetStats()
{
    m_packetCount = 0;
    m_rxTotalBytes = 0;
    m_validFrames = 0;
    m_crcOkFrames = 0;
    m_crcBadFrames = 0;
    m_intervalSum = 0;
    m_intervalMin = 999999;
    m_intervalMax = 0;
    m_lastRxTime = 0;
    m_msgCounts.clear();
    if (m_analyzerText) m_analyzerText->clear();
    if (m_lblAnalyzerPktCount) m_lblAnalyzerPktCount->setText("包数: 0");
    if (m_lblCrcStats) m_lblCrcStats->setText("CRC: --");
    if (m_msgTable) {
        m_msgTable->setRowCount(0);
    }
}

void MavlinkTestPage::onStopAtChanged(int value) { m_stopAtFrameCount = value; }

void MavlinkTestPage::onStopAtToggle(bool checked)
{
    if (checked) m_stoppedByLimit = false;
}

void MavlinkTestPage::onClearAnalyzer()
{
    if (m_analyzerText) m_analyzerText->clear();
    m_msgCounts.clear();
    if (m_msgTable) m_msgTable->setRowCount(0);
}

// =====================================================================
// Receive data handling
// =====================================================================
void MavlinkTestPage::onDataReceived(const QByteArray &data)
{
    if (data.isEmpty()) return;
    m_rxBuffer.append(data);
    m_rxTotalBytes += data.size();
    processRxData();
}

void MavlinkTestPage::processRxData()
{
    const quint8 *raw = reinterpret_cast<const quint8*>(m_rxBuffer.constData());
    int bufSize = m_rxBuffer.size();

    int i = 0;
    while (i < bufSize) {
        if (!MavlinkProtocolParser::isStartByte(raw[i])) {
            ++i;
            continue;
        }
        quint8 stx = raw[i];
        int hdrLen = MavlinkProtocolParser::headerLength(stx);
        // Need len byte + at least 2 crc after payload
        if (i + hdrLen >= bufSize) break;
        quint8 len = raw[i + 1];
        int total = hdrLen + len + 2;
        bool sig = (stx == MAVLINK_STX_V2) && (raw[i + 2] & 0x01);
        if (sig) total += MAVLINK_SIGNATURE_LEN;

        if (i + total > bufSize) break;   // incomplete, wait for more

        QByteArray frame = m_rxBuffer.mid(i, total);
        i += total;

        ParsedMavlinkFrame parsed = MavlinkProtocolParser::parseFrame(frame);
        if (!parsed.valid) continue;

        m_validFrames++;
        if (parsed.crcValid) m_crcOkFrames++; else m_crcBadFrames++;
        m_packetCount++;

        qint64 now = QDateTime::currentMSecsSinceEpoch();
        qint64 intervalMs = (m_lastRxTime > 0) ? (now - m_lastRxTime) : 0;
        m_lastRxTime = now;
        if (intervalMs > 0) {
            m_intervalSum += intervalMs;
            if (intervalMs < m_intervalMin) m_intervalMin = intervalMs;
            if (intervalMs > m_intervalMax) m_intervalMax = intervalMs;
        }

        updateMessageCounts(parsed.msgid);
        appendAnalyzerPacket(m_packetCount, parsed, intervalMs);
        enqueueRawFrame(frame);

        // stop-at
        if (ui->btnStopAtEnable->isChecked() && !m_stoppedByLimit) {
            if (m_validFrames >= m_stopAtFrameCount) {
                m_stoppedByLimit = true;
                if (m_comm->isConnected()) m_comm->disconnect();
                ui->textRecv->append(QString("[!] 已到达帧数上限 %1，自动断开").arg(m_stopAtFrameCount));
            }
        }
    }

    // Drop consumed bytes; keep tail for incomplete frame
    if (i > 0) m_rxBuffer.remove(0, i);

    // update stat labels
    if (m_lblAnalyzerPktCount)
        m_lblAnalyzerPktCount->setText(QString("包数: %1").arg(m_packetCount));
    if (m_lblCrcStats)
        m_lblCrcStats->setText(QString("CRC: 通过%1 失败%2").arg(m_crcOkFrames).arg(m_crcBadFrames));
    if (m_lblIntervalAvg && m_packetCount > 1)
        m_lblIntervalAvg->setText(QString("平均间隔: %1ms").arg(m_intervalSum / m_packetCount));
}

void MavlinkTestPage::updateMessageCounts(uint32_t msgid)
{
    m_msgCounts[msgid]++;
    if (!m_msgTable) return;
    // Rebuild/update the message-frequency table
    QMap<uint32_t, int>::const_iterator it;
    for (it = m_msgCounts.constBegin(); it != m_msgCounts.constEnd(); ++it) {
        bool found = false;
        for (int r = 0; r < m_msgTable->rowCount(); ++r) {
            if (m_msgTable->item(r, 0) && m_msgTable->item(r, 0)->data(Qt::UserRole).toUInt() == it.key()) {
                m_msgTable->item(r, 1)->setText(QString::number(it.value()));
                found = true;
                break;
            }
        }
        if (!found) {
            int r = m_msgTable->rowCount();
            m_msgTable->insertRow(r);
            auto *nameItem = new QTableWidgetItem(
                QString("%1 (0x%2)").arg(MavlinkProtocolParser::messageName(it.key()))
                                    .arg(it.key(), 0, 16));
            nameItem->setData(Qt::UserRole, it.key());
            auto *countItem = new QTableWidgetItem(QString::number(it.value()));
            nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            countItem->setTextAlignment(Qt::AlignCenter);
            m_msgTable->setItem(r, 0, nameItem);
            m_msgTable->setItem(r, 1, countItem);
        }
    }
    m_msgTable->sortItems(1, Qt::DescendingOrder);
}

void MavlinkTestPage::appendAnalyzerPacket(int seq, const ParsedMavlinkFrame &parsed, qint64 intervalMs)
{
    if (!m_analyzerText) return;
    QString line = QString("[%1] #%2 v%3 %4 seq:%5 sys:%6/%7 间隔:%8ms %9%10")
                       .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                       .arg(seq, 5)
                       .arg(parsed.version)
                       .arg(parsed.msgName, -18)
                       .arg(parsed.seq)
                       .arg(parsed.sysid).arg(parsed.compid)
                       .arg(intervalMs)
                       .arg(parsed.crcValid ? "CRC✓" : "CRC✗")
                       .arg(parsed.summary.isEmpty() ? "" : " | " + parsed.summary);
    m_analyzerText->append(line);
    if (m_analyzerText->document()->blockCount() > 500)
        m_analyzerText->clear();
}

void MavlinkTestPage::initAnalyzerUI()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(m_pageWidget->layout());
    if (!mainLayout) return;

    // ---- 1. Analyzer panel ----
    m_groupAnalyzer = new QGroupBox("MAVLink 协议分析仪", this);
    QVBoxLayout *analyzerLayout = new QVBoxLayout(m_groupAnalyzer);

    QHBoxLayout *topLayout = new QHBoxLayout();
    m_lblAnalyzerPktCount = new QLabel("包数: 0", this);
    m_lblAnalyzerPktCount->setStyleSheet("font-weight:bold;color:#00BFFF;");
    m_lblCrcStats = new QLabel("CRC: --", this);
    m_lblIntervalAvg = new QLabel("平均间隔: --", this);
    topLayout->addWidget(m_lblAnalyzerPktCount);
    topLayout->addWidget(m_lblCrcStats);
    topLayout->addWidget(m_lblIntervalAvg);
    topLayout->addStretch();

    QPushButton *btnClearAnalyzer = new QPushButton("清空分析", this);
    connect(btnClearAnalyzer, &QPushButton::clicked, this, &MavlinkTestPage::onClearAnalyzer);
    topLayout->addWidget(btnClearAnalyzer);
    analyzerLayout->addLayout(topLayout);

    m_analyzerText = new QTextEdit(this);
    m_analyzerText->setReadOnly(true);
    m_analyzerText->setFont(QFont("Courier New", 9));
    m_analyzerText->setMinimumHeight(150);
    m_analyzerText->setMaximumHeight(260);
    m_analyzerText->setStyleSheet("QTextEdit { background-color:#1E1E1E; color:#D4D4D4; }");
    analyzerLayout->addWidget(m_analyzerText);

    mainLayout->insertWidget(mainLayout->count(), m_groupAnalyzer);

    // ---- 2. Message frequency table ----
    m_groupMsgs = new QGroupBox("消息类型统计", this);
    QVBoxLayout *msgsLayout = new QVBoxLayout(m_groupMsgs);
    m_msgTable = new QTableWidget(0, 2, this);
    m_msgTable->setHorizontalHeaderLabels({"消息类型", "次数"});
    m_msgTable->horizontalHeader()->setStretchLastSection(false);
    m_msgTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_msgTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_msgTable->verticalHeader()->setVisible(false);
    m_msgTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_msgTable->setSelectionMode(QAbstractItemView::NoSelection);
    m_msgTable->setMaximumHeight(180);
    msgsLayout->addWidget(m_msgTable);
    mainLayout->insertWidget(mainLayout->count(), m_groupMsgs);
}
