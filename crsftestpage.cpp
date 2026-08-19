#include "crsftestpage.h"
#include "ui_page_serial_crsftest.h"
#include "serialcommunicator.h"
#include "crsfprotocolparser.h"
#include "crsfchannelwidget.h"
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>

#include <QSerialPortInfo>
#include <QMessageBox>
#include <QTextCursor>
#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <QMap>
#include <QPair>
#include <QLabel>
#include <QDir>
#include <QTextCodec>
#include <QCoreApplication>

// Legacy CRSF aliases for backward compatibility.
// The CrsfFrameType enum from crsfprotocolparser.h defines the canonical names.
// These #defines provide names NOT in the enum or with different naming:
#define LEGACY_ADDR_BCAST       0x00
#define LEGACY_ADDR_FLIGHT_CTRL 0x1C
#define LEGACY_ADDR_RADIO       0xEA
#define CRSF_FRAMETYPE_PARAM_READ            0x2A
#define CRSF_FRAMETYPE_PARAM_WRITE           0x2B
#define CRSF_FRAMETYPE_RADIO_ID              0x3A

// CRSF protocol constants
static constexpr int CRSF_MIN_FRAME_SIZE = 4;
static constexpr int DIAG_RING_SIZE = 20;

// =====================================================================
// CRSF CRC8 calculation (polynomial 0xD5, init 0x00)
// =====================================================================
static quint8 crsfCrc8(const quint8 *data, int len)
{
    quint8 crc = 0;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0xD5;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// =====================================================================
// Build a complete CRSF frame: [sync=0xC8] [len] [addr] [type] [payload...] [crc]
// len = addr(1) + type(1) + payload_size + crc(1)
// =====================================================================
static QByteArray buildCrsfFrame(quint8 type, const QByteArray &payload)
{
    QByteArray frame;
    frame.append(static_cast<char>(LEGACY_ADDR_RADIO));
    frame.append(static_cast<char>(type));
    frame.append(payload);
    quint8 crc = crsfCrc8(reinterpret_cast<const quint8*>(frame.constData()), frame.size());
    frame.append(static_cast<char>(crc));

    QByteArray fullFrame;
    fullFrame.append(static_cast<char>(CRSF_SYNC_BYTE));
    fullFrame.append(static_cast<char>(frame.size() + 1));
    fullFrame.append(frame);
    return fullFrame;
}

// =====================================================================
// CrsfTestPage implementation
// =====================================================================

CrsfTestPage::CrsfTestPage(SerialCommunicator *comm, QWidget *parent)
    : QWidget(parent), ui(new Ui::PageSerialCrsfTest), m_comm(comm)
{
    m_pageWidget = new QWidget();
    ui->setupUi(m_pageWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_pageWidget);

    // Baud rates - add 420000 for CRSF
    QStringList baudRates = {
        "9600", "19200", "38400", "57600", "115200",
        "230400", "420000", "460800", "921600"
    };
    ui->comboBaud->addItems(baudRates);
    ui->comboBaud->setCurrentText("420000");

    // CRSF frame type presets
    ui->comboCrsfType->addItem(QStringLiteral("Custom Hex"), 0);
    ui->comboCrsfType->addItem(QStringLiteral("RC Channels (0x16)"), CRSF_FRAMETYPE_RC_CHANNELS_PACKED);
    ui->comboCrsfType->addItem(QStringLiteral("Link Stats (0x14)"), CRSF_FRAMETYPE_LINK_STATISTICS);
    ui->comboCrsfType->addItem(QStringLiteral("Heartbeat (0x0B)"), CRSF_FRAMETYPE_HEARTBEAT);
    ui->comboCrsfType->addItem(QStringLiteral("Device Info (0x29)"), CRSF_FRAMETYPE_DEVICE_INFO);

    // Signal connections
    connect(ui->btnRefresh, &QPushButton::clicked, this, &CrsfTestPage::onRefreshPorts);
    connect(ui->btnConnect, &QPushButton::clicked, this, &CrsfTestPage::onToggleConnection);
    connect(ui->btnSend, &QPushButton::clicked, this, &CrsfTestPage::onSendData);
    connect(ui->btnClearRecv, &QPushButton::clicked, this, &CrsfTestPage::onClearRecv);
    connect(ui->btnClearRecvData, &QPushButton::clicked, this, &CrsfTestPage::onClearRecv);
    connect(ui->btnSaveRecv, &QPushButton::clicked, this, &CrsfTestPage::onSaveRecv);
    connect(ui->btnDiagWindow, &QPushButton::clicked, this, &CrsfTestPage::openDiagDialog);
    connect(ui->btnResetStats, &QPushButton::clicked, this, &CrsfTestPage::onResetStats);

    connect(ui->spinStopAt, SIGNAL(valueChanged(int)), this, SLOT(onStopAtChanged(int)));
    connect(ui->btnStopAtEnable, &QPushButton::toggled, this, &CrsfTestPage::onStopAtToggle);
    ui->spinStopAt->setToolTip("到达帧数上限后自动断开\n点击右侧按钮启用/禁用");

    connect(ui->btnTplRcChannels, &QPushButton::clicked, this, &CrsfTestPage::onTplRcChannels);
    connect(ui->btnTplLinkStats, &QPushButton::clicked, this, &CrsfTestPage::onTplLinkStats);
    connect(ui->btnTplHeartbeat, &QPushButton::clicked, this, &CrsfTestPage::onTplHeartbeat);
    connect(ui->btnTplCustom, &QPushButton::clicked, this, &CrsfTestPage::onTplCustom);

    connect(m_comm, &SerialCommunicator::statusChanged, this, &CrsfTestPage::onSerialStatus);
    connect(m_comm, &SerialCommunicator::dataReceived, this, &CrsfTestPage::onDataReceived);

    // 接收缓冲区刷新定时器 - 10ms 持续轮询，减少驱动缓冲延迟
    m_rxFlushTimer = new QTimer(this);
    m_rxFlushTimer->setSingleShot(false);
    m_rxFlushTimer->setInterval(10);
    connect(m_rxFlushTimer, &QTimer::timeout, this, &CrsfTestPage::onRxFlushTimeout);
    m_rxFlushTimer->start();

    m_stopAtFrameCount = 0;
    m_stoppedByLimit = false;
    ui->btnStopAtEnable->setChecked(false);
    m_rxTotalBytes = 0;
    m_crcPassBytes = 0;
    m_crcPassFrames = 0;
    m_crcFailFrames = 0;
    m_diagRing.reserve(DIAG_RING_SIZE);
    m_diagDialog = nullptr;
    m_diagText = nullptr;
    m_btnClearDiag = nullptr;
    m_diagBufAccumCount = 0;
    m_diagIntervalAnomaly = 0;

    onRefreshPorts();
    onResetStats();

    // 默认发送数据: CRSF RC 通道帧
    onTplRcChannels();

    // 异步CRSF日志初始化
    initLogFile();
    m_logFlushTimer = new QTimer(this);
    m_logFlushTimer->setSingleShot(false);
    m_logFlushTimer->setInterval(200);  // 每200ms批量flush一次
    connect(m_logFlushTimer, &QTimer::timeout, this, &CrsfTestPage::onLogFlushTimeout);
    m_logFlushTimer->start();

    // 延迟分布初始化: 0-10, 10-20, 20-30, ..., 190-200 (ms)
    m_distributionBins.clear();
    for (int i = 10; i <= 200; i += 10)
        m_distributionBins.append(i);

    // 接收间隔分布初始化:
    //   0-40ms: 每1ms一个区间 (0~1, 1~2, ..., 39~40)
    //   40ms以上: 每10ms一个区间 (40~50, 50~60, ..., 190~200)
    //   然后 200~300, >300
    m_rxIntervalBins.clear();
    for (int i = 1; i <= 40; ++i)      // 0~1, 1~2, ..., 39~40
        m_rxIntervalBins.append(i);
    for (int i = 50; i <= 200; i += 10) // 40~50, 50~60, ..., 190~200
        m_rxIntervalBins.append(i);
    m_rxIntervalBins.append(300);       // 200~300ms
    m_rxIntervalBins.append(-1);        // >300ms
    m_rxIntervalCount.resize(m_rxIntervalBins.size());
    m_rxIntervalCount.fill(0);

    // CRSF协议解析器
    m_parser = new CrsfProtocolParser();

    // 初始化CRSF分析器UI面板
    initAnalyzerUI();
}

CrsfTestPage::~CrsfTestPage()
{
    m_logFlushTimer->stop();
    closeLogFile();
    delete m_parser;
    delete ui;
}

// =====================================================================
// 异步CRSF日志写入
// 将所有CRSF原始数据（手动发送、自动发送、接收）写入文件
// 使用队列缓冲 + 定时flush，不阻塞接收路径
// =====================================================================
void CrsfTestPage::initLogFile()
{
    // 在可执行文件同级目录下创建 logs 子目录
    QString logDir = QCoreApplication::applicationDirPath() + "/logs";
    QDir().mkpath(logDir);

    m_logFilePath = logDir + "/crsf_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".bin";
    m_logFile = new QFile(m_logFilePath, this);

    // 打开日志文件（二进制写入模式）
    if (!m_logFile->open(QIODevice::WriteOnly)) {
        qWarning("CrsfLog: Failed to open log file: %s", qPrintable(m_logFile->errorString()));
        m_logEnabled = false;
        return;
    }

    m_logEnabled = true;
    m_logBytesWritten = 0;
    m_logQueue.clear();

    qInfo("CrsfLog: Raw binary logging started -> %s", qPrintable(m_logFilePath));
}

void CrsfTestPage::closeLogFile()
{
    if (m_logFile && m_logFile->isOpen()) {
        flushLogBuffer(); // 确保剩余数据写入
        m_logFile->close();
        qInfo("CrsfLog: Log file closed, total %lld bytes written -> %s",
              m_logBytesWritten, qPrintable(m_logFilePath));
    }
    m_logEnabled = false;
}

void CrsfTestPage::enqueueLogLine(const QString &line)
{
    Q_UNUSED(line);
    // 已废弃，改用 enqueueRawFrame
}

void CrsfTestPage::enqueueRawFrame(const QByteArray &frame)
{
    if (!m_logEnabled || !m_logFile || !m_logFile->isOpen())
        return;

    // 只入队，不做IO操作
    m_logQueue.enqueue(frame);

    // 防止队列无限增长（最多保留1万帧）
    if (m_logQueue.size() > 10000) {
        int drop = m_logQueue.size() / 10;
        for (int i = 0; i < drop; ++i)
            m_logQueue.dequeue();
    }
}

void CrsfTestPage::flushLogBuffer()
{
    if (!m_logEnabled || !m_logFile || !m_logFile->isOpen())
        return;

    if (m_logQueue.isEmpty())
        return;

    // 批量出队，拼接写入（减少系统调用）
    QByteArray batch;
    batch.reserve(m_logQueue.size() * 64); // 预分配

    while (!m_logQueue.isEmpty()) {
        batch.append(m_logQueue.dequeue());
    }

    qint64 written = m_logFile->write(batch);
    if (written > 0) {
        m_logBytesWritten += written;
    }
    m_logFile->flush();
}

void CrsfTestPage::onLogFlushTimeout()
{
    flushLogBuffer();
}

// =====================================================================
// Serial port operations
// =====================================================================
void CrsfTestPage::onRefreshPorts()
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

void CrsfTestPage::onToggleConnection()
{
    if (m_comm->isConnected()) {
        m_comm->disconnect();
        updateConnectionState();
    } else {
        QString port = ui->comboPort->currentText();
        if (port.isEmpty()) {
            QMessageBox::warning(this, tr("Prompt"), tr("Select a port first"));
            return;
        }
        int baud = ui->comboBaud->currentText().toInt();
        if (m_comm->connectToPort(port, baud)) {
            updateConnectionState();
        }
    }
}

void CrsfTestPage::updateConnectionState()
{
    bool connected = m_comm->isConnected();
    ui->btnConnect->setText(connected ? tr("Disconnect") : tr("Connect"));
    ui->labelStatus->setText(connected ? tr("Connected") : tr("Disconnected"));
    ui->labelStatus->setStyleSheet(connected
        ? "color:green;font-weight:bold;font-size:12pt;"
        : "color:red;font-weight:bold;font-size:12pt;");
}

void CrsfTestPage::onSerialStatus(const QString &msg)
{
    bool wasConnected = m_comm->isConnected();
    updateConnectionState();
    emit serialStatusChanged(msg);

    // 如果断开了连接，写一条日志标记
    if (!wasConnected) {
    }
}

// =====================================================================
// Send data
// =====================================================================
void CrsfTestPage::onSendData()
{
    if (!m_comm->isConnected()) {
        QMessageBox::warning(this, tr("Prompt"), tr("Please connect serial port first"));
        return;
    }

    QString input = ui->textSend->toPlainText().trimmed();
    if (input.isEmpty()) {
        QMessageBox::warning(this, tr("Prompt"), tr("Please enter data to send"));
        return;
    }

    // Parse hex
    QString hex = input.simplified().remove(' ');
    QByteArray data = QByteArray::fromHex(hex.toLatin1());
    if (data.isEmpty()) {
        QMessageBox::warning(this, tr("Prompt"), tr("Invalid hex data"));
        return;
    }

    int count = ui->spinRepeatCount->value();
    int interval = ui->spinRepeatInterval->value();

    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    for (int i = 0; i < count; ++i) {
        int seq = m_sendSeq++;

        // 记录发送
        SendRecord rec;
        rec.timestamp = QDateTime::currentMSecsSinceEpoch();
        rec.data = data;
        m_sendRecords.insert(seq, rec);
        while (m_sendRecords.size() > MAX_SEND_RECORDS)
            m_sendRecords.remove(m_sendRecords.firstKey());

        m_comm->sendData(data);

        // 记录到日志文件（异步，原始二进制）
        enqueueRawFrame(data);

        // 每包显示一行
        QString line = QString("[TX#%1] [%2] %3")
            .arg(seq).arg(timestamp)
            .arg(QString::fromLatin1(data.toHex(' ').toUpper()));

        ui->textRecv->append(line);

        if (i < count - 1 && interval > 0) {
            QThread::msleep(interval);
            // 每包更新时间戳
            timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        }
    }

    m_lastSendLineSeq = m_sendSeq - 1;

    if (ui->checkAutoScroll->isChecked()) {
        QTextCursor c = ui->textRecv->textCursor();
        c.movePosition(QTextCursor::End);
        ui->textRecv->setTextCursor(c);
    }
}

// =====================================================================
// Receive data handling
// =====================================================================
void CrsfTestPage::onDataReceived(const QByteArray &data)
{
    if (data.isEmpty()) return;

    // 追加到缓冲区
    m_rxBuffer.append(data);

    // 统计收到的字节数
    m_rxTotalBytes += data.size();

    // 直接处理缓冲区内的完整帧（10ms 定时器也会轮询）
    processRxData();
}

void CrsfTestPage::processRxData()
{
    if (m_rxBuffer.isEmpty()) return;

    while (true) {
        const quint8 *raw = reinterpret_cast<const quint8*>(m_rxBuffer.constData());
        int bufSize = m_rxBuffer.size();

        // 找 sync 字节
        int frameStart = -1;
        for (int i = 0; i < bufSize - CRSF_MIN_FRAME_SIZE + 1; ++i) {
            if (raw[i] == CRSF_SYNC_BYTE && raw[i+1] >= 2 && raw[i+1] <= 64
                && i + raw[i+1] + 2 <= bufSize) {
                frameStart = i;
                break;
            }
        }
        if (frameStart < 0)
            break;

        int frameLen = raw[frameStart + 1] + 2;

        // 组包完成的时间作为帧到达时间
        qint64 frameRxTime = QDateTime::currentMSecsSinceEpoch();

        QByteArray frame = m_rxBuffer.mid(frameStart, frameLen);

        // CRC校验：从 type(第2字节) 到 payload 末尾
        // ELRS风格: [sync][len][type][addr][payload...][crc]
        // CRC覆盖: type + addr + payload = frameLen - 2 字节（从 frame[2] 到 frame[frameLen-2]）
        // 或者严格按 Betaflight 标准只覆盖 type+payload... 
        // 实际上 ELRS 的 CRC 覆盖 type + addr + payload (所有帧头部之后的数据)
        // CRC覆盖范围 = 所有非头部非CRC字节 = frameLen - 3 字节
        // 从 frame[2] (type) 开始
        quint8 calcCrc = crsfCrc8(
            reinterpret_cast<const quint8*>(frame.constData()) + 2,
            frameLen - 3);
        quint8 pktCrc = static_cast<quint8>(frame[frameLen - 1]);
        bool crcOk = (calcCrc == pktCrc);

        if (!crcOk) {
            // CRC 错误，记录日志后丢弃
            m_crcFailFrames++;
            // 即使是CRC失败帧也记录日志
            enqueueRawFrame(frame);
            m_rxBuffer.remove(0, frameStart + 1);
            continue;
        }
        m_crcPassFrames++;
        m_crcPassBytes += frameLen;

        m_rxBuffer.remove(0, frameStart + frameLen);

        // ============================================================
        // CRSF协议分析仪: 每一包都送入分析器
        // ============================================================
        {
            // 计算与上一包的间隔
            qint64 intervalMs = 0;
            if (m_analyzerLastRxTime > 0) {
                intervalMs = frameRxTime - m_analyzerLastRxTime;
            }
            m_analyzerLastRxTime = frameRxTime;

            m_analyzerPacketCount++;
            appendAnalyzerPacket(m_analyzerPacketCount, frame, intervalMs);
        }

        // 延迟匹配（手动发送）
        int matchedSeq = -1;
        qint64 matchedLatency = -1;
        for (auto it = m_sendRecords.end(); it != m_sendRecords.begin();) {
            --it;
            if (frame.contains(it.value().data)) {
                matchedSeq = it.key();
                matchedLatency = frameRxTime - it.value().timestamp;
                m_sendRecords.erase(it);
                break;
            }
        }

        // 接收间隔 —— 用组包完成时间
        qint64 rxInterval = -1;
        if (m_lastRxTime > 0) {
            rxInterval = frameRxTime - m_lastRxTime;
            // 只过滤异常值（>60s 不算正常间隔）
            if (rxInterval < 60000) {
                m_rxIntervalSamples.append(rxInterval);
                int bin = 0;
                for (; bin < m_rxIntervalBins.size(); ++bin)
                    if (rxInterval < m_rxIntervalBins[bin]) break;
                if (bin >= m_rxIntervalBins.size())
                    bin = m_rxIntervalBins.size() - 1;
                m_rxIntervalCount[bin]++;
            }
        }
        m_lastRxTime = frameRxTime;

        // ============================================================
        // 每帧原始二进制写入CRSF日志（异步）
        // ============================================================
        enqueueRawFrame(frame);

        if (matchedLatency > 0 && matchedLatency < 10000) {
            m_latencySamples.append(matchedLatency);
            m_totalLatency += matchedLatency;
            m_packetCount++;
            if (m_packetCount == 1)
                m_minLatency = m_maxLatency = matchedLatency;
            else {
                if (matchedLatency < m_minLatency) m_minLatency = matchedLatency;
                if (matchedLatency > m_maxLatency) m_maxLatency = matchedLatency;
            }
            int binIdx = binForLatency(matchedLatency);
            while (m_distCount.size() <= binIdx)
                m_distCount.append(0);
            m_distCount[binIdx]++;

            // 记录事件到环形缓冲
            QString event = QString("[seq=%1 %2ms]").arg(matchedSeq).arg(matchedLatency);
            addDiagEvent(event);

            // 大延迟诊断
            if (matchedLatency > 50)
                diagnoseLargeLatency(matchedSeq, matchedLatency, frame);

            qint64 avg = m_totalLatency / m_packetCount;
            ui->lbLastLatency->setText(QString("Last: %1 ms").arg(matchedLatency));
            ui->lbAvgLatency->setText(QString("Avg: %1 ms").arg(avg));
            ui->lbMinLatency->setText(QString("Min: %1 ms").arg(m_minLatency));
            ui->lbMaxLatency->setText(QString("Max: %1 ms").arg(m_maxLatency));
            ui->lbPktCount->setText(QString("Packets: %1").arg(m_packetCount));
            updateDistDisplay();
        }

        // 记录每帧事件（无论是否匹配成功）— 同时推送到诊断窗口
        {
            QString ts2 = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
            quint8 fType = (frame.size() >= 4) ? static_cast<quint8>(frame[2]) : 0;
            const char *typeName = "";
            switch (fType) {
                case 0x14: typeName = "LinkStats"; break;
                case 0x16: typeName = "RCCh"; break;
                case 0x1C: typeName = "Batt"; break;
                case 0x0B: typeName = "HB"; break;
                case 0x08: typeName = "GPS"; break;
                case 0x29: typeName = "DevInfo"; break;
            }
            QString ev = QString("[%1] %2 lat=%3 rxIval=%4 buf=%5")
                .arg(ts2.right(12))
                .arg(typeName, -9)
                .arg(matchedLatency > 0 ? QString::number(matchedLatency) + "ms" : "?")
                .arg(rxInterval > 0 ? QString::number(rxInterval) + "ms" : "?")
                .arg(m_rxBuffer.size());
            addDiagEvent(ev);

            // 每帧都推到诊断窗口（实时流）
            addDiagLine(ev);

            // 实时分析
            QString analysis;
            if (m_rxBuffer.size() > 30) {
                m_diagBufAccumCount++;
                if (m_diagBufAccumCount == 1) {
                    analysis = QString("  ⚠ buf=%1 → 开始积压，驱动批量提交延迟")
                        .arg(m_rxBuffer.size());
                    addDiagLine(analysis);
                }
            } else {
                if (m_diagBufAccumCount > 0 && m_diagBufAccumCount <= 3) {
                    addDiagLine(QString("  ✓ 缓冲恢复 (buf=%1)").arg(m_rxBuffer.size()));
                }
                m_diagBufAccumCount = 0;
            }
            if (rxInterval > 40 && rxInterval < 200 && m_rxBuffer.size() < 30) {
                m_diagIntervalAnomaly++;
                if (m_diagIntervalAnomaly == 1) {
                    addDiagLine("  ⚠ 间隔异常但 buf 小 → 接收机输出不连续，可能丢帧");
                }
            } else if (rxInterval > 40 && m_rxBuffer.size() >= 30) {
                m_diagIntervalAnomaly = 0;
                addDiagLine("  ⚠ 间隔大且 buf 大 → 串口驱动批量缓冲，数据积压后一次性释放");
            }
            if (rxInterval > 200) {
                addDiagLine("  ⚠ 超大间隔 → 系统调度被抢占或 USB 帧周期超时");
            }

            // 每 50 帧输出一次累积摘要到诊断窗口
            if (m_crcPassFrames > 0 && m_crcPassFrames % 50 == 0) {
                qint64 iMin = 99999, iMax = 0, iSum = 0;
                int iCount = m_rxIntervalSamples.size();
                int n = qMin(iCount, 50);
                for (int i = iCount - n; i < iCount; ++i) {
                    qint64 s = m_rxIntervalSamples[i];
                    if (s < iMin) iMin = s;
                    if (s > iMax) iMax = s;
                    iSum += s;
                }
                QString summary = QString("\n=== Summary (%1 frames) ===").arg(m_crcPassFrames);
                summary += QString("\n  Interval: %1/%2/%3 ms (min/max/avg)")
                    .arg(iMin).arg(iMax).arg(n > 0 ? iSum / n : 0);
                summary += QString("\n  Buffer: %1 bytes | CRC OK: %2 | CRC FAIL: %3")
                    .arg(m_rxBuffer.size()).arg(m_crcPassFrames).arg(m_crcFailFrames);
                if (m_diagBufAccumCount > 0)
                    summary += "\n  ⚠ Still accumulating buffer (driving batch delay)";
                addDiagLine(summary);
            }
        }

        // 大间隔诊断（>50ms）
        if (rxInterval > 50) {
            diagnoseLargeInterval(rxInterval, frame, m_rxBuffer.size());
        }

        appendRecvData(frame, "RX", matchedSeq, matchedLatency);
        updateIntervalDisplay();

        // 检查帧数上限
        if (m_stopAtFrameCount > 0 && !m_stoppedByLimit) {
            if (m_rxIntervalSamples.size() >= m_stopAtFrameCount) {
                m_stoppedByLimit = true;
                if (m_comm && m_comm->isConnected()) {
                    m_comm->disconnect();
                    updateConnectionState();
                }
                ui->labelStatus->setText(QString("已停止（达到 %1 帧上限）").arg(m_stopAtFrameCount));
                ui->labelStatus->setStyleSheet("color:orange;font-weight:bold;");
            }
        }
    }
}

void CrsfTestPage::onStopAtChanged(int value)
{
    // 只存值，不生效；由 btnStopAtEnable 控制启用
}

void CrsfTestPage::onStopAtToggle(bool checked)
{
    if (checked) {
        m_stopAtFrameCount = ui->spinStopAt->value();
        m_stoppedByLimit = false;
        ui->labelStatus->setStyleSheet("color:blue;font-weight:bold;");
        ui->labelStatus->setText(QString("启用限帧：到达 %1 帧后自动断开").arg(m_stopAtFrameCount));
        ui->spinStopAt->setEnabled(false);
    } else {
        m_stopAtFrameCount = 0;
        m_stoppedByLimit = false;
        ui->labelStatus->setStyleSheet("color:red;font-weight:bold;font-size:12pt;");
        ui->labelStatus->setText("未连接");
        ui->spinStopAt->setEnabled(true);
    }
}

void CrsfTestPage::onRxFlushTimeout()
{
    // 定时器持续轮询：处理缓冲区中的残余数据
    processRxData();
}

int CrsfTestPage::binForLatency(qint64 latency) const
{
    if (latency <= 0) return 0;
    int idx = static_cast<int>(latency / 10);
    if (idx >= m_distributionBins.size())
        idx = m_distributionBins.size() - 1;
    return idx;
}

void CrsfTestPage::appendRecvData(const QByteArray &data, const QString &direction,
                                    int matchSeq, qint64 matchLatency)
{
    QString hexStr = QString::fromLatin1(data.toHex(' ').toUpper());

    // Try to parse CRSF frames
    int crsfCount = 0;
    QString crsfInfo;
    int offset = 0;
    while (offset <= data.size() - CRSF_MIN_FRAME_SIZE) {
        const quint8 *d = reinterpret_cast<const quint8*>(data.constData()) + offset;
        if (d[0] == CRSF_SYNC_BYTE && d[1] >= 2 && d[1] <= 64 && offset + d[1] + 2 <= data.size()) {
            int frameLen = d[1] + 2;
            quint8 calcCrc = crsfCrc8(d + 2, frameLen - 3);
            quint8 pktCrc = d[frameLen - 1];
            QString validStr = (calcCrc == pktCrc) ? QStringLiteral("OK") : QStringLiteral("BAD");
            quint8 crsfType = d[2];
            const char *typeName = "";
            switch (crsfType) {
                case 0x08: typeName = "GPS"; break;
                case 0x14: typeName = "LinkStats"; break;
                case 0x16: typeName = "RCChannels"; break;
                case 0x1C: typeName = "Battery"; break;
                case 0x21: typeName = "FlightMode"; break;
                case 0x29: typeName = "DeviceInfo"; break;
                case 0x0B: typeName = "Heartbeat"; break;
                default:   typeName = ""; break;
            }
            crsfInfo += QString(" [CRSF#%1: type=0x%2%3 len=%4 CRC%5]")
                .arg(crsfCount).arg(crsfType, 2, 16, QChar('0'))
                .arg(typeName[0] ? QString("(%1)").arg(typeName) : QString())
                .arg(frameLen).arg(validStr);
            crsfCount++;
            offset += frameLen;
        } else {
            offset++;
        }
    }

    // Build display line
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    QString latencyStr;
    if (matchSeq >= 0 && matchLatency >= 0)
        latencyStr = QString(" [RX#%1 %2ms]").arg(matchSeq).arg(matchLatency);

    QString line;
    if (ui->checkShowTimestamp->isChecked()) {
        if (ui->checkHexOnly->isChecked()) {
            line = QString("[%1] %2%3").arg(timestamp).arg(hexStr).arg(latencyStr);
        } else {
            line = QString("[%1] [%2] %3%4").arg(timestamp).arg(direction).arg(hexStr).arg(latencyStr);
            if (!crsfInfo.isEmpty())
                line += crsfInfo;
        }
    } else {
        if (ui->checkHexOnly->isChecked()) {
            line = QString("%1%2").arg(hexStr).arg(latencyStr);
        } else {
            line = QString("[%1] %2%3").arg(direction).arg(hexStr).arg(latencyStr);
            if (!crsfInfo.isEmpty())
                line += crsfInfo;
        }
    }

    ui->textRecv->append(line);
    if (ui->checkAutoScroll->isChecked()) {
        QTextCursor c = ui->textRecv->textCursor();
        c.movePosition(QTextCursor::End);
        ui->textRecv->setTextCursor(c);
    }
}

// =====================================================================
// Latency calculation (removed - now integrated into onDataReceived)
// =====================================================================
void CrsfTestPage::computeLatency(const QByteArray &responseData)
{
    Q_UNUSED(responseData);
    // legacy stub
}

void CrsfTestPage::updateDistDisplay()
{
    ui->textDist->setPlainText(buildLatencyDistStr());
}

void CrsfTestPage::updateIntervalDisplay()
{
    if (!ui->textDist->isVisible()) return;
    ui->textDist->setPlainText(buildLatencyDistStr());
}

QString CrsfTestPage::buildLatencyDistStr() const
{
    if (m_packetCount == 0) {
        // 纯接收模式：Latency 分布不适用，直接显示 RX 间隔分布
        return buildIntervalDistStr();
    }

    int maxCount = 1;
    for (int c : m_distCount)
        if (c > maxCount) maxCount = c;

    QString result = QString("Latency Distribution (%1 samples):\n").arg(m_packetCount);

    // 标题行显示最小/最大
    result += QString("  Min: %1 ms  |  Max: %2 ms  |  Avg: %3 ms\n")
                  .arg(m_minLatency).arg(m_maxLatency)
                  .arg(m_packetCount > 0 ? m_totalLatency / m_packetCount : 0);
    for (int i = 0; i < m_distCount.size(); ++i) {
        int count = m_distCount[i];
        int low = i * 10;
        int high = (i + 1) * 10;
        int barLen = (maxCount > 0) ? (count * 30 / maxCount) : 0;
        QString label = (i == m_distCount.size() - 1)
            ? QString(">%1ms").arg(low)
            : QString("%1-%2ms").arg(low, 3).arg(high, 3);
        result += QString("  %1: %2 ").arg(label, -12).arg(count, 4);
        result += QString(barLen, '#');
        result += "\n";
    }

    // 追加诊断报告（如果有）
    if (!m_lastDiagReport.isEmpty())
        result += "\n" + m_lastDiagReport;

    return result;
}

QString CrsfTestPage::buildIntervalDistStr() const
{
    int totalIntervals = 0;
    for (int c : m_rxIntervalCount)
        totalIntervals += c;

    // 数据统计
    QString statsLine = QString("  RX Bytes: %1  |  CRC OK: %2 bytes (%3 frames)  |  CRC FAIL: %4\n")
        .arg(m_rxTotalBytes).arg(m_crcPassBytes).arg(m_crcPassFrames).arg(m_crcFailFrames);

    if (totalIntervals == 0)
        return QString("RX Interval Distribution: (no samples)\n") + statsLine;

    int maxCount = 1;
    for (int c : m_rxIntervalCount)
        if (c > maxCount) maxCount = c;

    QString result = statsLine;
    result += QString("RX Interval Distribution (%1 samples):\n").arg(totalIntervals);

    // 接收间隔的 Min/Max/Avg
    qint64 rMin = 99999, rMax = 0, rSum = 0;
    for (qint64 s : m_rxIntervalSamples) {
        if (s < rMin) rMin = s;
        if (s > rMax) rMax = s;
        rSum += s;
    }
    result += QString("  Min: %1 ms  |  Max: %2 ms  |  Avg: %3 ms\n")
                  .arg(rMin).arg(rMax)
                  .arg(totalIntervals > 0 ? rSum / totalIntervals : 0);

    // 判断当前bin是否属于"精细模式"(1ms区间)
    auto isFineBin = [&](int idx) -> bool {
        if (idx >= m_rxIntervalBins.size()) return false;
        int bound = m_rxIntervalBins[idx];
        return (bound >= 1 && bound <= 40);
    };

    // 找出最后一个非零精细区间索引
    int lastNonZeroFine = -1;
    for (int i = m_rxIntervalBins.size() - 1; i >= 0; --i) {
        if (isFineBin(i) && i < m_rxIntervalCount.size() && m_rxIntervalCount[i] > 0) {
            lastNonZeroFine = i;
            break;
        }
    }

    int prevBound = 0;
    int lastFineRendered = -100; // 上次渲染的精细区间索引
    bool skipped = false;        // 是否跳过了区间

    for (int i = 0; i < m_rxIntervalBins.size(); ++i) {
        int count = (i < m_rxIntervalCount.size()) ? m_rxIntervalCount[i] : 0;
        int bound = m_rxIntervalBins[i];

        bool render = true;

        if (isFineBin(i)) {
            if (count == 0 && i < lastNonZeroFine) {
                // 精细区间且无数据，跳过
                render = false;
                skipped = true;
            } else if (count > 0 && skipped && lastFineRendered > 0 && i - lastFineRendered > 1) {
                // 非零区间，且之前跳过了一些，在它前面补省略号
                result += "    ...\n";
                skipped = false;
            } else {
                skipped = false;
            }
        }

        if (!render) continue;
        lastFineRendered = i;

        // 构造标签
        QString label;
        if (bound < 0) {
            label = QString("  >%1ms").arg(prevBound);
        } else if (prevBound == 0) {
            label = QString("0~%1ms").arg(bound);
        } else {
            label = QString("%1~%2ms").arg(prevBound).arg(bound);
        }

        int barLen = (maxCount > 0) ? (count * 30 / maxCount) : 0;
        result += QString("  %1: %2 ").arg(label, -12).arg(count, 4);
        result += QString(barLen, '#');
        result += "\n";

        prevBound = bound;
    }

    // 追加诊断报告（如果有）
    if (!m_lastDiagReport.isEmpty())
        result += "\n" + m_lastDiagReport;

    return result;
}

// =====================================================================
// Clear and reset
// =====================================================================
void CrsfTestPage::onClearRecv()
{
    ui->textRecv->clear();
}

void CrsfTestPage::onSaveRecv()
{
    QString content = ui->textRecv->toPlainText();
    if (content.isEmpty()) {
        ui->labelStatus->setStyleSheet("color:orange;font-weight:bold;");
        ui->labelStatus->setText("没有数据可保存");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "保存接收数据",
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + "_crsf_log.txt",
        "文本文件 (*.txt);;所有文件 (*)");
    if (fileName.isEmpty()) return;

    // 如果文件存在则追加，不存在则新建
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    if (QFile::exists(fileName))
        mode |= QIODevice::Append;

    QFile file(fileName);
    if (!file.open(mode)) {
        ui->labelStatus->setStyleSheet("color:red;font-weight:bold;");
        ui->labelStatus->setText("保存失败: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    // 追加模式下加个分割线
    if (mode.testFlag(QIODevice::Append))
        out << "\n--- Append: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << " ---\n";
    out << content;
    file.close();

    ui->labelStatus->setStyleSheet("color:green;font-weight:bold;");
    ui->labelStatus->setText(QString("已保存(%1): %2")
        .arg(mode.testFlag(QIODevice::Append) ? "追加" : "新建")
        .arg(fileName));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("保存完成");
    QString modeStr = mode.testFlag(QIODevice::Append) ? "(已追加到已有文件)" : "";
    msgBox.setText(QString("已" + modeStr + "保存到:\n%1").arg(fileName));
    QPushButton *btnOpen = msgBox.addButton("用记事本打开", QMessageBox::ActionRole);
    QPushButton *btnOpenEditor = msgBox.addButton("用编辑器打开", QMessageBox::ActionRole);
    msgBox.addButton("关闭", QMessageBox::RejectRole);
    msgBox.exec();

    if (msgBox.clickedButton() == btnOpen) {
        QProcess::startDetached("notepad.exe", QStringList() << fileName);
    } else if (msgBox.clickedButton() == btnOpenEditor) {
        QStringList editors = {"code.exe", "notepad++.exe", "sublime_text.exe", "vim"};
        bool opened = false;
        for (const QString &ed : editors) {
            if (QProcess::startDetached(ed, QStringList() << fileName)) {
                opened = true;
                break;
            }
        }
        if (!opened)
            QProcess::startDetached("notepad.exe", QStringList() << fileName);
    }
}

// =====================================================================
// Diagnose large latency
// =====================================================================
void CrsfTestPage::addDiagEvent(const QString &event)
{
    m_diagRing.append(event);
    if (m_diagRing.size() > DIAG_RING_SIZE)
        m_diagRing.removeFirst();
}

void CrsfTestPage::diagnoseLargeLatency(int matchedSeq, qint64 latency, const QByteArray &frame)
{
    // 帧类型
    QString frameType = "?";
    if (frame.size() >= 4) {
        quint8 type = static_cast<quint8>(frame[2]);
        switch (type) {
            case 0x14: frameType = "LinkStats"; break;
            case 0x16: frameType = "RCChannels"; break;
            case 0x1C: frameType = "Battery"; break;
            case 0x0B: frameType = "Heartbeat"; break;
            case 0x08: frameType = "GPS"; break;
            case 0x29: frameType = "DeviceInfo"; break;
            default:   frameType = QString("0x%1").arg(type, 2, 16, QChar('0')); break;
        }
    }

    // 发送这条记录时的 m_sendRecords 状态
    qint64 sendTime = 0;
    qint64 oldestSendTime = 0;
    qint64 newestSendTime = 0;
    int pendingCount = m_sendRecords.size();  // 匹配前没移除，用当前状态

    // 找这条匹配的发送记录的时间
    auto it = m_sendRecords.find(matchedSeq);
    if (it != m_sendRecords.end())
        sendTime = it.value().timestamp;

    // 诊断输出
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    m_lastDiagReport = QString("=== ⚠ LARGE LATENCY: %1ms (seq=%2, type=%3) @ %4 ===")
        .arg(latency).arg(matchedSeq).arg(frameType).arg(ts);
    m_lastDiagReport += QString("\n  SendRecords pending: %1").arg(pendingCount);

    // 最近 10 帧事件
    m_lastDiagReport += "\n  Recent events (newest first):";
    int count = 0;
    for (int i = m_diagRing.size() - 1; i >= 0 && count < 10; --i, ++count)
        m_lastDiagReport += "\n    " + m_diagRing[i];

    m_lastDiagReport += "\n";

    // 同时打印到接收区，方便即时查看
    // 同时打印到接收区，方便即时查看
    ui->textRecv->append("\n" + m_lastDiagReport);

    // 也输出到诊断窗口
    addDiagLine(m_lastDiagReport);
    showDiagStats();
}

void CrsfTestPage::diagnoseLargeInterval(qint64 interval, const QByteArray &frame, int bufSize)
{
    // 帧类型
    QString frameType = "?";
    if (frame.size() >= 4) {
        quint8 type = static_cast<quint8>(frame[2]);
        switch (type) {
            case 0x14: frameType = "LinkStats"; break;
            case 0x16: frameType = "RCChannels"; break;
            case 0x1C: frameType = "Battery"; break;
            case 0x0B: frameType = "Heartbeat"; break;
            default:   frameType = QString("0x%1").arg(type, 2, 16, QChar('0')); break;
        }
    }

    QString hex = QString::fromLatin1(frame.toHex(' ').toUpper()).left(48);
    QString ts = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    // 拼接诊断报告
    QString report = QString("=== ⚠ LARGE INTERVAL: %1ms @ %2 (type=%3, buf=%4) ===")
        .arg(interval).arg(ts).arg(frameType).arg(bufSize);
    report += "\n  Frame (first 16 bytes): " + hex;

    // 最近 8 帧事件
    report += "\n  Recent events (newest first):";
    int count = 0;
    for (int i = m_diagRing.size() - 1; i >= 0 && count < 8; --i, ++count)
        report += "\n    " + m_diagRing[i];

    report += "\n";

    // 保存供分布区显示
    m_lastDiagReport = report;

    // 推送到接收区
    ui->textRecv->append(report);

    // 推送到诊断窗口
    addDiagLine(report);
    showDiagStats();
}

// =====================================================================
// Reset all statistics
// =====================================================================
void CrsfTestPage::onResetStats()
{
    m_latencySamples.clear();
    m_diagRing.clear();
    m_lastDiagReport.clear();
    if (m_diagText) m_diagText->clear();
    m_diagBufAccumCount = 0;
    m_diagIntervalAnomaly = 0;
    m_minLatency = 0;
    m_maxLatency = 0;
    m_totalLatency = 0;
    m_packetCount = 0;
    m_sendSeq = 0;
    m_sendRecords.clear();
    m_distCount.clear();
    m_distCount.resize(m_distributionBins.size());
    std::fill(m_distCount.begin(), m_distCount.end(), 0);

    m_lastRxTime = 0;
    m_rxIntervalSamples.clear();
    m_rxIntervalCount.clear();
    m_rxIntervalCount.resize(m_rxIntervalBins.size());
    std::fill(m_rxIntervalCount.begin(), m_rxIntervalCount.end(), 0);
    m_rxBuffer.clear();
    m_rxTotalBytes = 0;
    m_crcPassBytes = 0;
    m_crcPassFrames = 0;
    m_crcFailFrames = 0;

    ui->lbLastLatency->setText("Last: --");
    ui->lbAvgLatency->setText("Avg: --");
    ui->lbMinLatency->setText("Min: --");
    ui->lbMaxLatency->setText("Max: --");
    ui->lbPktCount->setText("Packets: 0");

    updateDistDisplay();
}

// =====================================================================
// Template buttons
// =====================================================================
void CrsfTestPage::onTplRcChannels()
{
    // CRSF RC Channels Packed (0x16)
    // 22 bytes payload: 16 channels x 11bit
    QByteArray payload;
    for (int i = 0; i < 22; ++i)
        payload.append(static_cast<char>(0x5A));
    QByteArray frame = buildCrsfFrame(CRSF_FRAMETYPE_RC_CHANNELS_PACKED, payload);
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void CrsfTestPage::onTplLinkStats()
{
    // CRSF Link Statistics (0x14)
    QByteArray payload;
    payload.append(static_cast<char>(-50));  // Uplink RSSI (-50 dBm)
    payload.append(static_cast<char>(100));   // Uplink LQ (100%)
    payload.append(static_cast<char>(30));    // Uplink SNR (30 dB)
    payload.append(static_cast<char>(-60));   // Diversity RSSI (-60 dBm)
    payload.append(static_cast<char>(0));     // Antenna
    payload.append(static_cast<char>(4));     // Downlink RF Mode
    payload.append(static_cast<char>(100));   // Downlink LQ
    payload.append(static_cast<char>(30));    // Downlink SNR

    QByteArray frame = buildCrsfFrame(CRSF_FRAMETYPE_LINK_STATISTICS, payload);
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void CrsfTestPage::onTplHeartbeat()
{
    // CRSF Heartbeat (0x0B) - empty payload
    QByteArray payload;
    QByteArray frame = buildCrsfFrame(CRSF_FRAMETYPE_HEARTBEAT, payload);
    ui->textSend->setPlainText(QString::fromLatin1(frame.toHex(' ').toUpper()));
}

void CrsfTestPage::onTplCustom()
{
    ui->textSend->clear();
    ui->textSend->setFocus();
}

// =====================================================================
// CRSF Protocol Analyzer UI
// =====================================================================
void CrsfTestPage::initAnalyzerUI()
{
    // m_pageWidget 包含所有UI控件（从.ui文件加载）
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(m_pageWidget->layout());
    if (!mainLayout) return;

    // ---------------------------------------------------------------
    // 1. CRSF Packet Analyzer 面板（协议分析仪风格）
    // ---------------------------------------------------------------
    m_groupAnalyzer = new QGroupBox("CRSF 协议分析仪", this);
    QVBoxLayout *analyzerLayout = new QVBoxLayout(m_groupAnalyzer);

    // 间隔统计分析行
    QHBoxLayout *intervalLayout = new QHBoxLayout();
    m_lblIntervalPattern = new QLabel("模式: 等待数据...", this);
    m_lblIntervalPattern->setStyleSheet("font-weight:bold;color:#00BFFF;");
    m_lblAnalyzerPktCount = new QLabel("包数: 0", this);
    m_lblIntervalMin = new QLabel("最小间隔: --", this);
    m_lblIntervalMax = new QLabel("最大间隔: --", this);
    m_lblIntervalAvg = new QLabel("平均间隔: --", this);

    intervalLayout->addWidget(m_lblIntervalPattern);
    intervalLayout->addWidget(m_lblAnalyzerPktCount);
    intervalLayout->addWidget(m_lblIntervalMin);
    intervalLayout->addWidget(m_lblIntervalMax);
    intervalLayout->addWidget(m_lblIntervalAvg);
    intervalLayout->addStretch();

    QPushButton *btnClearAnalyzer = new QPushButton("清空分析", this);
    QPushButton *btnResetInterval = new QPushButton("重置间隔统计", this);
    connect(btnClearAnalyzer, &QPushButton::clicked, this, &CrsfTestPage::onClearAnalyzer);
    connect(btnResetInterval, &QPushButton::clicked, this, &CrsfTestPage::onResetIntervalStats);
    intervalLayout->addWidget(btnClearAnalyzer);
    intervalLayout->addWidget(btnResetInterval);

    analyzerLayout->addLayout(intervalLayout);

    // 分析仪文本区（类似Saleae逻辑分析仪风格）
    m_analyzerText = new QTextEdit(this);
    m_analyzerText->setReadOnly(true);
    m_analyzerText->setFont(QFont("Courier New", 9));
    m_analyzerText->setMinimumHeight(120);
    m_analyzerText->setMaximumHeight(250);
    m_analyzerText->setStyleSheet(
        "QTextEdit { background-color: #1E1E1E; color: #D4D4D4; }"
    );
    analyzerLayout->addWidget(m_analyzerText);

    mainLayout->insertWidget(mainLayout->count(), m_groupAnalyzer);

    // ---------------------------------------------------------------
    // 2. RC Channels 波形面板（Betaflight接收机风格）
    // ---------------------------------------------------------------
    m_groupChannels = new QGroupBox("RC 通道 (优化中...)", this);
    QVBoxLayout *chLayout = new QVBoxLayout(m_groupChannels);

    // 通道波形 widget
    m_channelWidget = new CrsfChannelWidget(this);
    m_channelWidget->setMinimumHeight(200);
    chLayout->addWidget(m_channelWidget);

    // 通道数值表格（2行 x 8列）
    m_channelTable = new QTableWidget(2, 8, this);
    m_channelTable->setFont(QFont("Courier New", 9));
    m_channelTable->setMinimumHeight(60);
    m_channelTable->setMaximumHeight(80);
    m_channelTable->horizontalHeader()->setStretchLastSection(true);
    m_channelTable->verticalHeader()->setVisible(false);
    QStringList hLabels;
    for (int i = 1; i <= 8; ++i) hLabels << QString("CH%1").arg(i);
    m_channelTable->setHorizontalHeaderLabels(hLabels);
    // Set row labels
    m_channelTable->setVerticalHeaderLabels({"1-8", "9-16"});
    m_channelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_channelTable->setSelectionMode(QAbstractItemView::NoSelection);
    // Fill with default values
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 8; ++col) {
            int chIdx = row * 8 + col;
            if (chIdx < 16) {
                QTableWidgetItem *item = new QTableWidgetItem("1500");
                item->setTextAlignment(Qt::AlignCenter);
                m_channelTable->setItem(row, col, item);
            }
        }
    }
    chLayout->addWidget(m_channelTable);

    mainLayout->insertWidget(mainLayout->count(), m_groupChannels);

    // ---------------------------------------------------------------
    // 3. 链路统计面板
    // ---------------------------------------------------------------
    m_groupLinkStats = new QGroupBox("链路统计 (Link Statistics)", this);
    QVBoxLayout *lsLayout = new QVBoxLayout(m_groupLinkStats);

    // 第一行: 上行链路
    QHBoxLayout *uplinkLayout = new QHBoxLayout();
    QVBoxLayout *rssiLayout = new QVBoxLayout();

    // RSSI1
    QHBoxLayout *rssi1Row = new QHBoxLayout();
    m_lblLinkRSSI1 = new QLabel("RSSI1: -- dBm", this);
    m_barRSSI1 = new QProgressBar(this);
    m_barRSSI1->setRange(0, 130);
    m_barRSSI1->setTextVisible(true);
    m_barRSSI1->setMinimumWidth(150);
    rssi1Row->addWidget(m_lblLinkRSSI1);
    rssi1Row->addWidget(m_barRSSI1);
    rssiLayout->addLayout(rssi1Row);

    // RSSI2
    QHBoxLayout *rssi2Row = new QHBoxLayout();
    m_lblLinkRSSI2 = new QLabel("RSSI2: -- dBm", this);
    m_barRSSI2 = new QProgressBar(this);
    m_barRSSI2->setRange(0, 130);
    m_barRSSI2->setTextVisible(true);
    m_barRSSI2->setMinimumWidth(150);
    rssi2Row->addWidget(m_lblLinkRSSI2);
    rssi2Row->addWidget(m_barRSSI2);
    rssiLayout->addLayout(rssi2Row);

    // LQ
    QHBoxLayout *lqRow = new QHBoxLayout();
    m_lblLinkLQ = new QLabel("LQ: -- %", this);
    m_barLQ = new QProgressBar(this);
    m_barLQ->setRange(0, 100);
    m_barLQ->setTextVisible(true);
    m_barLQ->setMinimumWidth(150);
    lqRow->addWidget(m_lblLinkLQ);
    lqRow->addWidget(m_barLQ);
    rssiLayout->addLayout(lqRow);

    uplinkLayout->addLayout(rssiLayout);
    uplinkLayout->addStretch();

    // 第二列: 其他链路参数
    QVBoxLayout *otherLinkLayout = new QVBoxLayout();
    m_lblLinkSNR = new QLabel("SNR: -- dB", this);
    m_lblLinkRFMode = new QLabel("RF模式: --", this);
    m_lblLinkTxPower = new QLabel("发射功率: --", this);
    m_lblLinkDLRSSI = new QLabel("下行RSSI: -- dBm", this);
    m_lblLinkDLLQ = new QLabel("下行LQ: -- %", this);
    m_lblLinkDLSNR = new QLabel("下行SNR: -- dB", this);
    otherLinkLayout->addWidget(m_lblLinkSNR);
    otherLinkLayout->addWidget(m_lblLinkRFMode);
    otherLinkLayout->addWidget(m_lblLinkTxPower);
    otherLinkLayout->addWidget(m_lblLinkDLRSSI);
    otherLinkLayout->addWidget(m_lblLinkDLLQ);
    otherLinkLayout->addWidget(m_lblLinkDLSNR);

    uplinkLayout->addLayout(otherLinkLayout);
    uplinkLayout->addStretch();

    lsLayout->addLayout(uplinkLayout);
    m_groupLinkStats->setMaximumHeight(150);

    mainLayout->insertWidget(mainLayout->count(), m_groupLinkStats);

    // 默认隐藏链路统计（等收到数据再显示）
    m_groupLinkStats->setVisible(false);
}

void CrsfTestPage::appendAnalyzerPacket(int seq, const QByteArray &rawFrame, qint64 intervalMs)
{
    if (!m_parser || !m_analyzerText) return;

    ParsedCrsfFrame parsed = m_parser->parseFrame(rawFrame);

    // Build formatted line
    QString color;
    switch (parsed.category) {
    case CrsfPacketCategory::RcChannels:   color = "#4EC9B0"; break; // teal
    case CrsfPacketCategory::LinkStats:    color = "#569CD6"; break; // blue
    case CrsfPacketCategory::Sensor:       color = "#D7BA7D"; break; // yellow-orange
    case CrsfPacketCategory::Heartbeat:    color = "#808080"; break; // gray
    case CrsfPacketCategory::Command:      color = "#C586C0"; break; // purple
    case CrsfPacketCategory::Msp:          color = "#4FC1FF"; break; // light blue
    case CrsfPacketCategory::DisplayPort:  color = "#F55385"; break; // pink
    default:                               color = "#D4D4D4"; break; // white
    }

    QString crcStr;
    if (parsed.crcValid) {
        crcStr = "<span style='color:#4EC9B0;'>✓</span>";
    } else {
        crcStr = "<span style='color:#FF5555;'>✗</span>";
    }

    QString intervalStr;
    if (intervalMs < 0) {
        intervalStr = "  +0.0ms ";
    } else {
        // Color code intervals that seem abnormal
        QString intColor = "#D4D4D4";
        if (intervalMs > 50) intColor = "#FF6B6B";   // red - too long
        else if (intervalMs > 20) intColor = "#FFA500"; // orange - long
        else if (intervalMs < 2) intColor = "#FFA500";   // orange - too short
        intervalStr = QString(" <span style='color:%1;'>+%2ms</span> ")
            .arg(intColor).arg(intervalMs, 0, 'f', 1);
    }

    // Source/Dest addresses with names
    QString addrStr = QString("<span style='color:#888;'>%1</span>")
        .arg(parsed.sourceAddressName);
    // Try to detect destination from frame type context
    QString destName = "?";
    if (parsed.deviceAddress == 0xEA) destName = "RX";
    else if (parsed.deviceAddress == 0xEC) destName = "FC";
    else if (parsed.deviceAddress == 0xC8) destName = "Radio";
    else if (parsed.deviceAddress == 0xEE) destName = "RX";
    addrStr += QString("→<span style='color:#888;'>%1</span>").arg(destName);

    // Truncate payload summary for display
    QString summary = parsed.payloadSummary;
    if (summary.length() > 70)
        summary = summary.left(67) + "...";

    QString line = QString(
        "<span style='color:#888;'>[#%1]</span>"
        "%2|"
        "<span style='color:%3;font-weight:bold;'>%4</span>"
        "| %5 | len=%6 | %7 | %8"
    ).arg(seq, 5, 10, QChar('0'))
     .arg(intervalStr)
     .arg(color)
     .arg(parsed.frameTypeName, -12)
     .arg(addrStr)
     .arg(rawFrame.size())
     .arg(summary)
     .arg(crcStr);

    // Append with HTML
    m_analyzerText->append(line);

    // Auto-scroll to bottom
    QScrollBar *sb = m_analyzerText->verticalScrollBar();
    if (sb) sb->setValue(sb->maximum());

    // Limit entries to 500 to prevent memory issues
    if (m_analyzerPacketCount > 500) {
        QTextDocument *doc = m_analyzerText->document();
        if (doc) {
            // Remove first block (oldest entry)
            QTextBlock first = doc->begin();
            if (first.isValid()) {
                QTextCursor cursor(first);
                cursor.select(QTextCursor::BlockUnderCursor);
                cursor.removeSelectedText();
                cursor.deleteChar(); // remove the block separator
            }
        }
    }

    // Update interval stats
    if (parsed.crcValid) {
        CrsfIntervalAnalysis analysis = m_parser->analyzeInterval(intervalMs);
        m_lblIntervalPattern->setText(
            QString("模式: <span style='color:#00FF00;'>%1</span>")
                .arg(analysis.detectedPattern.isEmpty() ? "分析中..." : analysis.detectedPattern));
        m_lblIntervalMin->setText(QString("最小间隔: %1ms").arg(analysis.minIntervalMs));
        m_lblIntervalMax->setText(QString("最大间隔: %1ms").arg(analysis.maxIntervalMs));
        m_lblIntervalAvg->setText(QString("平均间隔: %1ms").arg(analysis.avgIntervalMs));
        m_lblAnalyzerPktCount->setText(QString("包数: %1").arg(analysis.sampleCount));
    }

    // Check for RC channel data and update waveform
    if (parsed.frameType == 0x16 && parsed.payloadDecoded) {
        // Convert CRSF channel values to PWM for display
        for (int i = 0; i < 16; ++i) {
            m_currentChannels[i] = CrsfProtocolParser::channelValueToPwm(parsed.rcChannels.channels[i]);
        }
        m_hasRcData = true;
        updateChannelWidget();
    }

    // Check for link statistics
    if (parsed.frameType == 0x14 && parsed.payloadDecoded) {
        m_currentLinkStats = parsed.linkStats;
        m_hasLinkData = true;
        updateLinkStatsUI();
        m_groupLinkStats->setVisible(true);
    }
}

void CrsfTestPage::updateChannelWidget()
{
    if (!m_channelWidget || !m_hasRcData) return;
    m_channelWidget->updateChannels(m_currentChannels);

    // Update table
    if (m_channelTable) {
        for (int row = 0; row < 2; ++row) {
            for (int col = 0; col < 8; ++col) {
                int chIdx = row * 8 + col;
                if (chIdx < 16) {
                    QTableWidgetItem *item = m_channelTable->item(row, col);
                    if (item) {
                        uint16_t val = m_currentChannels[chIdx];
                        QString text = QString::number(val);
                        item->setText(text);

                        // Color code: green around center, red at extremes
                        if (val > 1450 && val < 1550) {
                            item->setForeground(QColor("#4EC9B0"));
                        } else if (val > 1300 && val < 1700) {
                            item->setForeground(QColor("#D7BA7D"));
                        } else {
                            item->setForeground(QColor("#FF5555"));
                        }
                    }
                }
            }
        }
    }
}

void CrsfTestPage::updateLinkStatsUI()
{
    if (!m_hasLinkData) return;

    // RSSI values (stored as negative, show absolute for bar display)
    int rssi1Val = qAbs(m_currentLinkStats.uplinkRSSI1);
    int rssi2Val = qAbs(m_currentLinkStats.uplinkRSSI2);

    if (m_lblLinkRSSI1) {
        m_lblLinkRSSI1->setText(QString("RSSI1: %1 dBm").arg(m_currentLinkStats.uplinkRSSI1));
        // Color: green if > -80, yellow if > -95, red otherwise
        if (m_currentLinkStats.uplinkRSSI1 >= -80)
            m_lblLinkRSSI1->setStyleSheet("color:#4EC9B0;");
        else if (m_currentLinkStats.uplinkRSSI1 >= -95)
            m_lblLinkRSSI1->setStyleSheet("color:#D7BA7D;");
        else
            m_lblLinkRSSI1->setStyleSheet("color:#FF5555;");
    }
    if (m_barRSSI1) {
        m_barRSSI1->setValue(rssi1Val);
        m_barRSSI1->setFormat(QString("%1 dBm").arg(m_currentLinkStats.uplinkRSSI1));
        // Color the bar via stylesheet
        if (rssi1Val <= 80)
            m_barRSSI1->setStyleSheet("QProgressBar::chunk { background-color: #4EC9B0; }");
        else if (rssi1Val <= 95)
            m_barRSSI1->setStyleSheet("QProgressBar::chunk { background-color: #D7BA7D; }");
        else
            m_barRSSI1->setStyleSheet("QProgressBar::chunk { background-color: #FF5555; }");
    }

    if (m_lblLinkRSSI2) {
        m_lblLinkRSSI2->setText(QString("RSSI2: %1 dBm").arg(m_currentLinkStats.uplinkRSSI2));
        if (m_currentLinkStats.uplinkRSSI2 >= -80)
            m_lblLinkRSSI2->setStyleSheet("color:#4EC9B0;");
        else if (m_currentLinkStats.uplinkRSSI2 >= -95)
            m_lblLinkRSSI2->setStyleSheet("color:#D7BA7D;");
        else
            m_lblLinkRSSI2->setStyleSheet("color:#FF5555;");
    }
    if (m_barRSSI2) {
        m_barRSSI2->setValue(rssi2Val);
        m_barRSSI2->setFormat(QString("%1 dBm").arg(m_currentLinkStats.uplinkRSSI2));
        if (rssi2Val <= 80)
            m_barRSSI2->setStyleSheet("QProgressBar::chunk { background-color: #4EC9B0; }");
        else if (rssi2Val <= 95)
            m_barRSSI2->setStyleSheet("QProgressBar::chunk { background-color: #D7BA7D; }");
        else
            m_barRSSI2->setStyleSheet("QProgressBar::chunk { background-color: #FF5555; }");
    }

    if (m_lblLinkLQ) {
        m_lblLinkLQ->setText(QString("LQ: %1%").arg(m_currentLinkStats.uplinkLinkQuality));
        if (m_currentLinkStats.uplinkLinkQuality >= 95)
            m_lblLinkLQ->setStyleSheet("color:#4EC9B0;");
        else if (m_currentLinkStats.uplinkLinkQuality >= 80)
            m_lblLinkLQ->setStyleSheet("color:#D7BA7D;");
        else
            m_lblLinkLQ->setStyleSheet("color:#FF5555;");
    }
    if (m_barLQ) {
        m_barLQ->setValue(m_currentLinkStats.uplinkLinkQuality);
        m_barLQ->setFormat(QString("%1%").arg(m_currentLinkStats.uplinkLinkQuality));
        if (m_currentLinkStats.uplinkLinkQuality >= 95)
            m_barLQ->setStyleSheet("QProgressBar::chunk { background-color: #4EC9B0; }");
        else if (m_currentLinkStats.uplinkLinkQuality >= 80)
            m_barLQ->setStyleSheet("QProgressBar::chunk { background-color: #D7BA7D; }");
        else
            m_barLQ->setStyleSheet("QProgressBar::chunk { background-color: #FF5555; }");
    }

    if (m_lblLinkSNR) {
        m_lblLinkSNR->setText(QString("SNR: %1 dB").arg(m_currentLinkStats.uplinkSNR));
        if (m_currentLinkStats.uplinkSNR >= 12)
            m_lblLinkSNR->setStyleSheet("color:#4EC9B0;");
        else if (m_currentLinkStats.uplinkSNR >= 6)
            m_lblLinkSNR->setStyleSheet("color:#D7BA7D;");
        else
            m_lblLinkSNR->setStyleSheet("color:#FF5555;");
    }
    if (m_lblLinkRFMode)
        m_lblLinkRFMode->setText(QString("RF模式: %1").arg(
            CrsfProtocolParser::rfModeName(m_currentLinkStats.rfMode)));
    if (m_lblLinkTxPower)
        m_lblLinkTxPower->setText(QString("发射功率: %1").arg(
            CrsfProtocolParser::txPowerName(m_currentLinkStats.uplinkTxPower)));
    if (m_lblLinkDLRSSI)
        m_lblLinkDLRSSI->setText(QString("下行RSSI: %1 dBm").arg(m_currentLinkStats.downlinkRSSI));
    if (m_lblLinkDLLQ)
        m_lblLinkDLLQ->setText(QString("下行LQ: %1%").arg(m_currentLinkStats.downlinkLinkQuality));
    if (m_lblLinkDLSNR)
        m_lblLinkDLSNR->setText(QString("下行SNR: %1 dB").arg(m_currentLinkStats.downlinkSNR));
}

void CrsfTestPage::onClearAnalyzer()
{
    if (m_analyzerText)
        m_analyzerText->clear();
}

void CrsfTestPage::onResetIntervalStats()
{
    // 重置解析器内部间隔统计（重新创建即可清零）
    delete m_parser;
    m_parser = new CrsfProtocolParser();
    if (m_lblIntervalPattern) m_lblIntervalPattern->setText("模式: 已重置");
    if (m_lblIntervalMin) m_lblIntervalMin->setText("最小间隔: --");
    if (m_lblIntervalMax) m_lblIntervalMax->setText("最大间隔: --");
    if (m_lblIntervalAvg) m_lblIntervalAvg->setText("平均间隔: --");
    if (m_lblAnalyzerPktCount) m_lblAnalyzerPktCount->setText("包数: 0");
}

// =====================================================================
// Diagnostics Window
// =====================================================================
void CrsfTestPage::openDiagDialog()
{
    if (m_diagDialog) {
        m_diagDialog->raise();
        m_diagDialog->activateWindow();
        return;
    }

    m_diagDialog = new QDialog(this);
    m_diagDialog->setWindowTitle("CRSF Diagnostic");
    m_diagDialog->resize(700, 500);

    QVBoxLayout *lay = new QVBoxLayout(m_diagDialog);

    // 实时统计标题
    QLabel *statsLabel = new QLabel("—", m_diagDialog);
    statsLabel->setObjectName("diagStatsLabel");
    statsLabel->setStyleSheet("font-weight:bold; padding:4px;");
    lay->addWidget(statsLabel);

    // 诊断文本
    m_diagText = new QTextEdit(m_diagDialog);
    m_diagText->setReadOnly(true);
    m_diagText->setFont(QFont("Consolas", 9));
    lay->addWidget(m_diagText);

    // 清空按钮
    QHBoxLayout *btnLay = new QHBoxLayout;
    m_btnClearDiag = new QPushButton("Clear", m_diagDialog);
    QPushButton *btnClose = new QPushButton("Close", m_diagDialog);
    btnLay->addStretch();
    btnLay->addWidget(m_btnClearDiag);
    btnLay->addWidget(btnClose);
    lay->addLayout(btnLay);

    connect(m_btnClearDiag, &QPushButton::clicked, [this]() {
        m_diagText->clear();
    });
    connect(btnClose, &QPushButton::clicked, m_diagDialog, &QDialog::close);
    connect(m_diagDialog, &QDialog::finished, [this]() {
        m_diagDialog = nullptr;
        m_diagText = nullptr;
        m_btnClearDiag = nullptr;
    });

    // 填入已有诊断数据
    for (int i = 0; i < m_diagRing.size(); ++i)
        m_diagText->append(m_diagRing[i]);
    if (!m_lastDiagReport.isEmpty())
        m_diagText->append(m_lastDiagReport);

    showDiagStats();
    m_diagDialog->show();
}

void CrsfTestPage::addDiagLine(const QString &line)
{
    if (m_diagText)
        m_diagText->append(line);
}

void CrsfTestPage::showDiagStats()
{
    if (!m_diagDialog) return;

    QLabel *label = m_diagDialog->findChild<QLabel*>("diagStatsLabel");
    if (!label) return;

    // 构建统计摘要
    QString stats = QString(
        "RX: %1 bytes | CRC OK: %2 bytes (%3 frames) | FAIL: %4 | "
        "Delay: min=%5 ms  max=%6 ms  avg=%7 ms (%8 pkts) | "
        "Interval: min=%9 ms  max=%10 ms"
    ).arg(m_rxTotalBytes)
     .arg(m_crcPassBytes).arg(m_crcPassFrames).arg(m_crcFailFrames)
     .arg(m_minLatency).arg(m_maxLatency)
     .arg(m_packetCount > 0 ? m_totalLatency / m_packetCount : 0)
     .arg(m_packetCount);

    // 间隔统计
    qint64 iMin = 99999, iMax = 0;
    for (qint64 s : m_rxIntervalSamples) {
        if (s < iMin) iMin = s;
        if (s > iMax) iMax = s;
    }
    if (iMin == 99999) { iMin = 0; iMax = 0; }
    stats += QString(" | Interval: min=%1 ms  max=%2 ms")
        .arg(iMin).arg(iMax);

    label->setText(stats);
}
