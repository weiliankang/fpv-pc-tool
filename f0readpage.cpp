#include "f0readpage.h"
#include "ui_page_serial_f0read.h"
#include "serialcommunicator.h"
#include "osdgridwidget.h"

#include <QDebug>
#include <QTextCursor>
#include <QDateTime>
#include <QRegularExpression>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QSettings>

// =====================================================================
// 协议常量（与 android 上位机 asvrx_async.c / 下位机 fpv_relay.c 一致）
// =====================================================================
static const quint8 FRAME_HEADER0 = 0xFE;
static const quint8 FRAME_HEADER1 = 0xEF;
static const quint8 FRAME_CMD    = 0xA2;

F0ReadPage::F0ReadPage(SerialCommunicator *comm, QWidget *parent)
    : QWidget(parent), ui(new Ui::PageSerialF0Read), m_comm(comm)
{
    m_pageWidget = new QWidget();
    ui->setupUi(m_pageWidget);

    // ---- 把"控制区、播放区、统计区、日志显示区"用垂直分割条隔开，可上下拖动 ----
    setupSplitter();

    // 信号连接
    connect(ui->btnSendF0, &QPushButton::clicked, this, &F0ReadPage::onSendF0);
    connect(ui->btnAutoPoll, &QPushButton::toggled, this, &F0ReadPage::onAutoPollToggled);
    connect(ui->btnClear, &QPushButton::clicked, this, &F0ReadPage::onClear);

    // 原始数据记录到文件（带时间戳）
    connect(ui->btnSaveRaw, &QPushButton::toggled, this, &F0ReadPage::onSaveRawToggled);

    // 离线读取 test.txt 解析
    connect(ui->btnBrowseFile, &QPushButton::clicked, this, &F0ReadPage::onBrowseFile);
    connect(ui->btnLoadFile, &QPushButton::clicked, this, &F0ReadPage::onLoadFile);

    // 串口数据：一个字节一个字节地喂入解析状态机
    connect(m_comm, &SerialCommunicator::dataReceived, this, &F0ReadPage::onDataReceived);

    // 自动轮询定时器
    m_pollTimer = new QTimer(this);
    m_pollTimer->setSingleShot(false);
    connect(m_pollTimer, &QTimer::timeout, this, &F0ReadPage::onPollTimer);

    // 快捷指令批次收尾定时器：收到帧后无新帧一段时间即把本批解析汇总输出
    // 650ms：快捷指令响应通常集中在发送后几十~一两百毫秒内到达，
    //        定时器要留足覆盖间隔(避免解析不全)，又不能太长让用户久等。
    //        sendF0Command 发送前还会兜底 flush 上一批，自动轮询场景不依赖此定时器。
    m_batchFlushTimer = new QTimer(this);
    m_batchFlushTimer->setSingleShot(true);
    m_batchFlushTimer->setInterval(650);
    connect(m_batchFlushTimer, &QTimer::timeout, this, &F0ReadPage::flushBatchParse);

    // ---- OSD 逐帧播放器 ----
    // 在占位的 widgetOsdPlayer 上创建 OsdGridWidget（与主窗口 OSD 页一致）
    m_osdGrid = new OsdGridWidget(ui->widgetOsdPlayer);
    QVBoxLayout *playerLayout = new QVBoxLayout(ui->widgetOsdPlayer);
    playerLayout->setContentsMargins(0, 0, 0, 0);
    playerLayout->addWidget(m_osdGrid);

    // 加载 OSD 字体：默认使用内置字符集 osdChars720_new（用户指定的 24x36 字库，
    // 其中 0xBA→186.png="9"）。SDK 图片字体(Betaflight等)里 0xBA 是 "I" 等，
    // 与预期数字字形不符，故默认选中第 6 项(index=5)内置字符集。
    ui->cboFont->setCurrentIndex(5);
    QString fontMsg = applyOsdFont(ui->cboFont->currentIndex());
    logLine(fontMsg);

    // 播放控制
    m_playTimer = new QTimer(this);
    m_playTimer->setSingleShot(false);
    connect(m_playTimer, &QTimer::timeout, this, &F0ReadPage::onPlayTimer);
    connect(ui->btnPlay, &QPushButton::toggled, this, &F0ReadPage::onPlayToggled);
    connect(ui->btnPrevFrame, &QPushButton::clicked, this, &F0ReadPage::onPrevFrame);
    connect(ui->btnNextFrame, &QPushButton::clicked, this, &F0ReadPage::onNextFrame);
    connect(ui->cboFont, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &F0ReadPage::onFontChanged);
    // 姿态条图形化开关
    connect(ui->chkAttitudeBar, &QCheckBox::toggled, this, [this](bool checked){
        if (m_osdGrid) m_osdGrid->setAttitudeBarEnabled(checked);
    });
    if (m_osdGrid) m_osdGrid->setAttitudeBarEnabled(ui->chkAttitudeBar->isChecked());
    ui->btnPlay->setCheckable(true);

    // 单调时钟（epoch 从页面创建开始，不随系统时间跳变）
    m_elapsed.start();

    // 恢复上次保存的分割条位置
    restoreSplitterState();

    logLine(QStringLiteral("F0 读取页面就绪。请先在\"连接\"页建立串口连接，或读取 test.txt 逐帧播放。"));
}

F0ReadPage::~F0ReadPage()
{
    if (m_pollTimer) m_pollTimer->stop();
    if (m_playTimer) m_playTimer->stop();
    stopRawLog();   // 关闭记录文件
    saveSplitterState();
    delete ui;
}

// ---------------------------------------------------------------------
// 布局：把"控制区、播放区、统计区、日志显示区"用垂直分割条隔开
// ---------------------------------------------------------------------
void F0ReadPage::setupSplitter()
{
    // 从 f0MainLayout 取出顶部的 4 个控制控件（groupControl/groupFile/groupPlayer/groupStats），
    // 全部移入上半容器；groupLog(日志) 保留为下半区域。
    QWidget *topContainer = new QWidget(this);
    QVBoxLayout *topLayout = new QVBoxLayout(topContainer);
    topLayout->setContentsMargins(0, 0, 0, 0);
    topLayout->setSpacing(0);

    QWidget *bottomContainer = new QWidget(this);
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomContainer);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomLayout->setSpacing(0);

    // 遍历 f0MainLayout，把非 groupLog 的项挪到上半区，groupLog 挪到下半区
    while (ui->f0MainLayout->count() > 0) {
        QLayoutItem *item = ui->f0MainLayout->takeAt(0);
        QWidget *w = item->widget();
        if (!w) { delete item; continue; }
        if (w == ui->groupLog) {
            bottomLayout->addWidget(w);
        } else {
            topLayout->addWidget(w);
        }
        delete item;
    }

    // 分割条
    m_splitter = new QSplitter(Qt::Vertical, this);
    m_splitter->setObjectName(QStringLiteral("splitterF0"));
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setOpaqueResize(true);      // 拖动时实时平滑重绘两侧
    m_splitter->addWidget(topContainer);
    m_splitter->addWidget(bottomContainer);

    // ---- 等比例：上下两区用相同拉伸因子 + Expanding，拖动/缩放时按比例同步伸缩 ----
    topContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    bottomContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 1);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_splitter);

    // 逐字节解析日志框(textLog)随 groupLog 自由拉伸填满
    if (ui->textLog) {
        ui->textLog->setMinimumSize(0, 0);
        ui->textLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        if (ui->logLayout)
            ui->logLayout->setStretch(0, 1); // 让 textLog 占据 logLayout 全部空间
    }
}

void F0ReadPage::saveSplitterState()
{
    if (!m_splitter) return;
    QSettings settings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    settings.setValue(QStringLiteral("f0read/splitterState"), m_splitter->saveState());
}

void F0ReadPage::restoreSplitterState()
{
    if (!m_splitter) return;
    QSettings settings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QByteArray state = settings.value(QStringLiteral("f0read/splitterState")).toByteArray();
    if (!state.isEmpty())
        m_splitter->restoreState(state);
}

// ---------------------------------------------------------------------
// 校验和：所有数据字节求和（与 vrx_checksum 一致）
// ---------------------------------------------------------------------
quint16 F0ReadPage::calcChecksum(const QByteArray &data) const
{
    quint32 sum = 0;
    for (int i = 0; i < data.size(); ++i)
        sum += (quint8)data.at(i);
    return (quint16)(sum & 0xFFFF);
}

// ---------------------------------------------------------------------
// 构建并发送 F0 命令
//   FE EF A2 00 08 F0 00 00 00 00 00 00 00 00 CS CS 0D 0A
// ---------------------------------------------------------------------
void F0ReadPage::sendF0Command()
{
    // 上一批(F0)若还有未输出的解析汇总，先输出，再开始新一批
    if (m_batchFlushTimer && m_batchFlushTimer->isActive())
        m_batchFlushTimer->stop();
    flushBatchParse();
    m_batchSeq++;
    m_batchStartTs = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    QByteArray tx;
    QByteArray payload;
    payload.resize(8);
    payload[0] = 0xF0;                     // 数据体首字节 = F0
    for (int i = 1; i < 8; ++i) payload[i] = 0x00;

    tx.append((char)FRAME_HEADER0);
    tx.append((char)FRAME_HEADER1);
    tx.append((char)FRAME_CMD);
    tx.append((char)((payload.size() >> 8) & 0xFF));
    tx.append((char)(payload.size() & 0xFF));
    tx.append(payload);
    quint16 chk = calcChecksum(payload);
    tx.append((char)((chk >> 8) & 0xFF));
    tx.append((char)(chk & 0xFF));
    tx.append((char)0x0D);
    tx.append((char)0x0A);

    // 记录发送时刻（单调时钟）
    m_sendF0Ms = m_elapsed.elapsed();
    m_hasLastFrame = false;

    // 记录到文件（若正在记录）：以"发送 F0"为新批次起点，时间戳=发送时刻
    if (m_rawLogFile) {
        QString sendTs = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        beginRawBatch(sendTs, tx);
    }

    QString absTime = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    logLine(QStringLiteral("[发送F0] 发送时刻: %1 | 数据: %2  (%3 bytes)")
                .arg(absTime)
                .arg(toHex(tx))
                .arg(tx.size()));
    logLine(QStringLiteral("          ↓ 以下各条回复的\"距F0发送\"=该帧接收完成时刻 - 本次F0发送时刻(见帧日志)"));

    if (!m_comm->sendData(tx)) {
        logLine(QStringLiteral("[错误] 串口未连接或发送失败！"));
    }

    // 通知 MainWindow 清空无线参数页历史缓冲，避免快捷指令的历史响应堆积污染无线打印栏
    emit f0CommandSent();
}

// ---------------------------------------------------------------------
// 手动发送
// ---------------------------------------------------------------------
void F0ReadPage::onSendF0()
{
    sendF0Command();
}

// ---------------------------------------------------------------------
// 自动轮询开关
// ---------------------------------------------------------------------
void F0ReadPage::onAutoPollToggled(bool checked)
{
    if (checked) {
        m_pollTimer->setInterval(ui->spinInterval->value());
        m_pollTimer->start();
        logLine(QStringLiteral("[轮询] 开启，间隔 %1 ms").arg(ui->spinInterval->value()));
        sendF0Command();  // 立即发一次
    } else {
        m_pollTimer->stop();
        logLine(QStringLiteral("[轮询] 关闭"));
    }
}

void F0ReadPage::onPollTimer()
{
    // 每次超时都更新间隔（用户可能在运行中改 spin 的值）
    m_pollTimer->setInterval(ui->spinInterval->value());
    sendF0Command();
}

// ---------------------------------------------------------------------
// 接收串口数据：逐字节喂入解析状态机
// ---------------------------------------------------------------------
void F0ReadPage::onDataReceived(const QByteArray &data)
{
    for (int i = 0; i < data.size(); ++i)
        feedByte((quint8)data.at(i));
}

// ---------------------------------------------------------------------
// 记录原始数据到文件（带时间戳）
// ---------------------------------------------------------------------
void F0ReadPage::onSaveRawToggled(bool checked)
{
    if (checked)
        startRawLog();
    else
        stopRawLog();
}

void F0ReadPage::startRawLog()
{
    // 已经打开则先关掉旧的
    if (m_rawLogFile) stopRawLog();

    // 让用户选择保存路径；默认文件名是 f0_raw_yyyyMMdd_HHmmss.txt
    QString defaultName = QStringLiteral("f0_raw_%1.txt")
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
    QString startDir = QDir::currentPath();
    QString path = QFileDialog::getSaveFileName(
            this,
            QStringLiteral("选择原始数据记录文件"),
            startDir + "/" + defaultName,
            QStringLiteral("文本日志 (*.txt);;所有文件 (*)"));
    if (path.isEmpty()) {
        // 用户取消 → 关闭按钮状态
        ui->btnSaveRaw->setChecked(false);
        return;
    }

    m_rawLogFile = new QFile(path, this);
    if (!m_rawLogFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logLine(QStringLiteral("[记录] 打开文件失败: %1 (%2)")
                    .arg(path, m_rawLogFile->errorString()));
        delete m_rawLogFile;
        m_rawLogFile = nullptr;
        ui->btnSaveRaw->setChecked(false);
        return;
    }

    m_rawLogPath = path;
    m_rawLogBytes = 0;
    m_rawLogBatch.clear();
    m_rawLogBatchActive = false;

    // 写文件头（纯 ASCII，避免编码问题）
    QTextStream out(m_rawLogFile);
    out << QStringLiteral("# f0 raw log  start: %1\n")
               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
    out << QStringLiteral("# line = << A HH:mm:ss.zzz TABLE F2: <F0 cmd + all resp frames, full raw bytes>\n");
    out.flush();

    ui->btnSaveRaw->setText(QStringLiteral("⏹ 停止记录"));
    ui->lbSavePath->setText(QStringLiteral("记录中: %1").arg(QFileInfo(path).fileName()));
    logLine(QStringLiteral("[记录] 开始记录原始数据 → %1").arg(path));
}

void F0ReadPage::stopRawLog()
{
    if (!m_rawLogFile) {
        ui->btnSaveRaw->setChecked(false);
        return;
    }
    // 把还没写出的残留批次写出去（可能最后一批还未触发切批）
    flushRawBatch();

    // 写文件尾，注明结束时间与总字节数
    QTextStream out(m_rawLogFile);
    out << QStringLiteral("# end %1 bytes=%2\n")
               .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"))
               .arg(m_rawLogBytes);
    out.flush();
    m_rawLogFile->close();
    delete m_rawLogFile;
    m_rawLogFile = nullptr;

    logLine(QStringLiteral("[记录] 已停止，共写入 %1 字节 → %2")
                .arg(m_rawLogBytes).arg(m_rawLogPath));
    ui->btnSaveRaw->setText(QStringLiteral("📁 记录原始数据"));
    ui->lbSavePath->setText(QStringLiteral("已保存: %1 (%2 字节)")
                                .arg(QFileInfo(m_rawLogPath).fileName())
                                .arg(m_rawLogBytes));
}

// ---------------------------------------------------------------------
// 开一个新的批次（发送 F0 时调用）。
//   先把上一批写出一行，再记录本次发送的 F0 原始字节，时间戳 = 发送时刻。
// ---------------------------------------------------------------------
void F0ReadPage::beginRawBatch(const QString &ts, const QByteArray &txBytes)
{
    if (!m_rawLogFile) return;
    // 上一批若还没写完（异常情况）先写出
    if (m_rawLogBatchActive)
        flushRawBatch();

    m_rawLogBatch.clear();
    m_rawLogBatchTs = ts;
    m_rawLogBatch.append(txBytes);   // 发送的 F0 命令原始字节
    m_rawLogBatchActive = true;
}

// ---------------------------------------------------------------------
// 追加一条完整响应帧（含 FE EF A2 头 + 校验 + 0D 0A 尾）到当前批次。
//   若距批次第一帧超过阈值（自动轮询异常时兜底切批）。
// ---------------------------------------------------------------------
void F0ReadPage::accumulateFrame(const QByteArray &fullFrame)
{
    if (!m_rawLogFile || fullFrame.isEmpty()) return;

    // 兜底：若没有活动批次，开一个（时间戳用当前时刻）
    if (!m_rawLogBatchActive) {
        m_rawLogBatch.clear();
        m_rawLogBatchTs = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
        m_rawLogBatchActive = true;
    }
    m_rawLogBatch.append(fullFrame);  // 完整原始帧（含帧头、校验/帧尾）
}

// ---------------------------------------------------------------------
// 把当前批次（发送 F0 + 全部响应）写成一行。
//   格式: << A HH:mm:ss.zzz TABLE F2: <完整原始帧hex 全部合并>
// ---------------------------------------------------------------------
void F0ReadPage::flushRawBatch()
{
    if (!m_rawLogFile || !m_rawLogBatchActive || m_rawLogBatch.isEmpty()) {
        m_rawLogBatch.clear();
        m_rawLogBatchActive = false;
        return;
    }
    QTextStream out(m_rawLogFile);
    out << QStringLiteral("<< A %1 TABLE F2: %2\n")
               .arg(m_rawLogBatchTs)
               .arg(toHex(m_rawLogBatch));
    out.flush();

    m_rawLogBytes += (quint64)m_rawLogBatch.size();
    m_rawLogBatch.clear();
    m_rawLogBatchActive = false;
}

// ---------------------------------------------------------------------
// 逐字节解析状态机（对应下位机 asvrx_async.c 的 parse_rx_packet）
// 但保证一个字节一个字节处理，并在整帧完成瞬间打时间戳。
// ---------------------------------------------------------------------
void F0ReadPage::feedByte(quint8 b)
{
    // 单调时钟毫秒时间戳（epoch = 页面创建）
    qint64 nowMs = m_elapsed.elapsed();

    // 追加字节
    m_rxBuf.append((char)b);
    m_rxByteCount++;

    // 超过 512 字节仍未成帧，丢弃最前面的（保持缓冲有界）
    if (m_rxBuf.size() > 512) {
        m_rxBuf.remove(0, m_rxBuf.size() - 512);
    }

    // ---- 状态机扫描，尽量凑出完整帧 ----
    // 帧格式: FE EF A2 lenH lenL [data...] csH csL 0D 0A
    while (m_rxBuf.size() >= 5) {
        // 找帧头
        const quint8 b0 = (quint8)m_rxBuf.at(0);
        const quint8 b1 = (quint8)m_rxBuf.at(1);
        const quint8 b2 = (quint8)m_rxBuf.at(2);

        if (!(b0 == FRAME_HEADER0 && b1 == FRAME_HEADER1 && b2 == FRAME_CMD)) {
            // 帧头不对 → 丢弃最前面 1 字节，继续找
            m_rxBuf.remove(0, 1);
            continue;
        }

        // 有帧头，取长度
        quint16 dataLen = ((quint8)m_rxBuf.at(3) << 8) | (quint8)m_rxBuf.at(4);
        int totalLen = 5 + dataLen + 2 + 2;  // 头 + 数据 + 校验 + 帧尾

        if (m_rxBuf.size() < totalLen) {
            // 还没凑够整帧，等更多字节
            break;
        }

        // 校验帧尾 0D 0A
        if ((quint8)m_rxBuf.at(totalLen - 2) == 0x0D &&
            (quint8)m_rxBuf.at(totalLen - 1) == 0x0A) {

            // 校验和
            quint16 recvSum = ((quint8)m_rxBuf.at(5 + dataLen) << 8) |
                              (quint8)m_rxBuf.at(6 + dataLen);
            quint32 calc = 0;
            for (int i = 0; i < dataLen; ++i)
                calc += (quint8)m_rxBuf.at(5 + i);
            quint16 calcSum = (quint16)(calc & 0xFFFF);

            QByteArray frame = m_rxBuf.left(totalLen);

            if (calcSum == recvSum) {
                // 完整且校验通过 → 整帧完成，打时间戳
                m_rxBuf.remove(0, totalLen);
                onFrameParsed(frame);
                continue;  // 可能还有下一帧
            } else {
                // 校验失败：丢弃这一整帧（保留预期内的健壮性行为）
                logLine(QStringLiteral("[丢弃] 校验失败 CS=0x%1 calc=0x%2")
                            .arg(recvSum, 4, 16, QChar('0'))
                            .arg(calcSum, 4, 16, QChar('0')));
                m_rxBuf.remove(0, totalLen);
                continue;
            }
        } else {
            // 帧尾不对：丢弃这一整帧
            m_rxBuf.remove(0, totalLen);
            continue;
        }
    }

    m_lastByteMs = nowMs;
}

// ---------------------------------------------------------------------
// 快捷指令批次解析汇总：把一次 F0 收到的所有字段级解析集中成一块输出
// ---------------------------------------------------------------------
void F0ReadPage::batchLogLine(const QString &line)
{
    m_batchParseLines << line;
    resetBatchFlushTimer();
}

void F0ReadPage::resetBatchFlushTimer()
{
    if (m_batchParseLines.isEmpty()) return;
    if (m_batchFlushTimer) m_batchFlushTimer->start();
}

void F0ReadPage::flushBatchParse()
{
    if (m_batchParseLines.isEmpty()) return;
    logLine(QStringLiteral("====== 快捷指令 #%1 解析汇总 (发送 %2) ======")
                .arg(m_batchSeq).arg(m_batchStartTs));
    for (const QString &ln : m_batchParseLines)
        logLine(ln);
    logLine(QStringLiteral("====== 快捷指令 #%1 汇总结束 ======").arg(m_batchSeq));
    m_batchParseLines.clear();
}

// ---------------------------------------------------------------------
// 参考无线参数页：对单条响应做字段级解析，返回多行描述（不含命令头）
// 覆盖 0x50-0x57(地面端) 与 0xA0-0xA7(中继)
// ---------------------------------------------------------------------
QString F0ReadPage::parseCommandDetail(quint8 cmd, const QByteArray &payload) const
{
    static const char bandLetters[5] = { 'A', 'B', 'C', 'D', 'E' };
    QString t;
    switch (cmd) {
    case 0x50: // 设置频点响应
        {
            int errCode = (payload.size() >= 2) ? static_cast<qint8>(payload.at(1)) : -1;
            t = (errCode == 0) ? QStringLiteral("频点设置完成")
                               : QString("频点设置失败, 错误码=%1").arg(errCode);
        }
        break;
    case 0x51: // 获取频点响应
        if (payload.size() >= 4) {
            int band = static_cast<quint8>(payload.at(1));
            int channel = static_cast<quint8>(payload.at(2));
            int hop = static_cast<quint8>(payload.at(3));
            char bl = (band >= 0 && band < 5) ? bandLetters[band] : '?';
            t = QString("频段: %1 (%2)\n通道: %3\n跳频模式: %4")
                    .arg(band).arg(bl).arg(channel + 1)
                    .arg(hop ? QStringLiteral("跳频") : QStringLiteral("定频"));
        }
        break;
    case 0x52: // 获取地面端无线状态
        if (payload.size() >= 6) {
            int rssi1 = static_cast<signed char>(payload.at(1));
            int rssi2 = static_cast<signed char>(payload.at(2));
            int rate = static_cast<quint8>(payload.at(3));
            int delay = static_cast<quint8>(payload.at(4));
            int conn = static_cast<quint8>(payload.at(5));
            t = QString("RSSI1: %1\nRSSI2: %2\n码率: %3 Mbps\n延迟: %4 ms\n连接状态: %5")
                    .arg(rssi1).arg(rssi2).arg(rate).arg(delay)
                    .arg(conn ? QStringLiteral("已连接") : QStringLiteral("未连接"));
        }
        break;
    case 0x53: // 获取基带功率响应
        if (payload.size() >= 6) {
            int pwrIdx = static_cast<quint8>(payload.at(1));
            quint32 bitmap = static_cast<quint8>(payload.at(2)) |
                             (static_cast<quint8>(payload.at(3)) << 8) |
                             (static_cast<quint8>(payload.at(4)) << 16) |
                             (static_cast<quint8>(payload.at(5)) << 24);
            t = QString("当前功率索引: %1\n可设置位图: 0x%2")
                    .arg(pwrIdx).arg(bitmap, 8, 16, QChar('0'));
        }
        break;
    case 0x55: // 获取天空端无线状态
        if (payload.size() >= 3) {
            int rssi1 = static_cast<signed char>(payload.at(1));
            int rssi2 = static_cast<signed char>(payload.at(2));
            t = QString("RSSI1: %1\nRSSI2: %2").arg(rssi1).arg(rssi2);
        }
        break;
    case 0x56: // 获取距离
        if (payload.size() >= 5) {
            quint32 dist = static_cast<quint8>(payload.at(1)) |
                           (static_cast<quint8>(payload.at(2)) << 8) |
                           (static_cast<quint8>(payload.at(3)) << 16) |
                           (static_cast<quint8>(payload.at(4)) << 24);
            t = QString("天空端距离: %1 m").arg(dist);
        }
        break;
    case 0x57: // OSD数据响应(矩阵在别处单独打印)
        t = QString("OSD数据: %1 bytes").arg(qMax(0, payload.size() - 1));
        if (payload.size() > 1)
            t += QString(" 首字节: 0x%1").arg(static_cast<quint8>(payload.at(1)), 2, 16, QChar('0'));
        break;
    case 0xA0: // 设置中继频点响应
        {
            int errCode = (payload.size() >= 2) ? static_cast<qint8>(payload.at(1)) : -1;
            t = (errCode == 0) ? QStringLiteral("中继频点设置完成")
                               : QString("中继频点设置失败, 错误码=%1").arg(errCode);
        }
        break;
    case 0xA1: // 获取中继频点响应
        if (payload.size() >= 4) {
            int band = static_cast<quint8>(payload.at(1));
            int channel = static_cast<quint8>(payload.at(2));
            int hop = static_cast<quint8>(payload.at(3));
            char bl = (band >= 0 && band < 5) ? bandLetters[band] : '?';
            t = QString("中继频段: %1 (%2)\n中继通道: %3\n跳频模式: %4")
                    .arg(band).arg(bl).arg(channel + 1)
                    .arg(hop ? QStringLiteral("跳频") : QStringLiteral("定频"));
        }
        break;
    case 0xA2: // 获取中继状态响应
        if (payload.size() >= 7) {
            int gndRssi1 = static_cast<signed char>(payload.at(1));
            int gndRssi2 = static_cast<signed char>(payload.at(2));
            int skyRssi1 = static_cast<signed char>(payload.at(3));
            int skyRssi2 = static_cast<signed char>(payload.at(4));
            int apConn = static_cast<quint8>(payload.at(5));
            int devConn = static_cast<quint8>(payload.at(6));
            t = QString("中继地面链路 RSSI1: %1\n中继地面链路 RSSI2: %2\n中继天空链路 RSSI1: %3\n中继天空链路 RSSI2: %4\n中继天空连接: %5\n中继地面连接: %6")
                    .arg(gndRssi1).arg(gndRssi2).arg(skyRssi1).arg(skyRssi2)
                    .arg(apConn ? QStringLiteral("已连接") : QStringLiteral("未连接"))
                    .arg(devConn ? QStringLiteral("已连接") : QStringLiteral("未连接"));
        }
        break;
    case 0xA3: // 获取中继BB功率响应
        if (payload.size() >= 6) {
            int pwrIdx = static_cast<quint8>(payload.at(1));
            quint32 bitmap = static_cast<quint8>(payload.at(2)) |
                             (static_cast<quint8>(payload.at(3)) << 8) |
                             (static_cast<quint8>(payload.at(4)) << 16) |
                             (static_cast<quint8>(payload.at(5)) << 24);
            t = QString("中继当前功率索引: %1\n可设置位图: 0x%2")
                    .arg(pwrIdx).arg(bitmap, 8, 16, QChar('0'));
        }
        break;
    case 0xA4: // 设置中继BB功率响应
        {
            int errCode = (payload.size() >= 2) ? static_cast<qint8>(payload.at(1)) : -1;
            t = (errCode == 0) ? QStringLiteral("中继BB功率设置完成")
                               : QString("中继BB功率设置失败, 错误码=%1").arg(errCode);
        }
        break;
    case 0xA5: // 获取中继-地面距离
        if (payload.size() >= 5) {
            qint32 dist = static_cast<qint8>(payload.at(1)) |
                          (static_cast<quint8>(payload.at(2)) << 8) |
                          (static_cast<quint8>(payload.at(3)) << 16) |
                          (static_cast<quint8>(payload.at(4)) << 24);
            t = QString("中继-地面距离: %1 m").arg(dist);
        }
        break;
    case 0xA6: // 获取中继-天空距离
        if (payload.size() >= 5) {
            qint32 dist = static_cast<qint8>(payload.at(1)) |
                          (static_cast<quint8>(payload.at(2)) << 8) |
                          (static_cast<quint8>(payload.at(3)) << 16) |
                          (static_cast<quint8>(payload.at(4)) << 24);
            t = QString("中继-天空距离: %1 m").arg(dist);
        }
        break;
    case 0xA7: // 中继OSD数据响应
        t = QString("中继OSD数据: %1 bytes").arg(qMax(0, payload.size() - 1));
        if (payload.size() > 1)
            t += QString(" 首字节: 0x%1").arg(static_cast<quint8>(payload.at(1)), 2, 16, QChar('0'));
        break;
    default:
        t = QStringLiteral("未知命令");
        break;
    }
    return t;
}

// ---------------------------------------------------------------------
// 一条完整帧解析完成 → 打印命令号 + 接收完成时间戳（ms 级）
// ---------------------------------------------------------------------
void F0ReadPage::onFrameParsed(const QByteArray &frame)
{
    qint64 nowMs = m_elapsed.elapsed();

    // 帧格式: FE EF A2 lenH lenL [data...] csH csL 0D 0A
    // 命令号 = 数据体首字节（frame[5]）
    quint8 cmd = (quint8)frame.at(5);
    m_lastCmd = cmd;

    // 记录到文件（若正在记录）：把这条完整响应帧（含帧头尾）追加进当前批次
    if (m_rawLogFile)
        accumulateFrame(frame);

    // 本帧完成时刻（单调时钟 + 绝对时间，双时间戳）
    qint64 offsetFromSend = (m_sendF0Ms > 0) ? (nowMs - m_sendF0Ms) : -1;
    QString absTime = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");

    // 相对上一条帧的间隔
    qint64 interval = -1;
    if (m_hasLastFrame) {
        interval = nowMs - m_lastFrameMs;
        m_lastIntervalMs = interval;
    }
    m_lastFrameMs = nowMs;
    m_hasLastFrame = true;
    m_frameCount++;

    // 数据体十六进制
    QByteArray payload = frame.mid(5, frame.size() - 9);  // 去掉头、校验、尾
    QString cmdName = commandName(cmd);

    // 每条日志含义说明：
    //   完成时刻(abs)  = 本帧解析完成的墙钟时间(绝对时间)
    //   距F0发送       = 本帧接收完成时刻 - 最近一次发送F0的时刻(单调时钟,ms)
    //                    → 即 从发出F0到收到这条回复花了多少毫秒
    //   帧间隔         = 本帧与上一条帧的接收完成时刻之差(单调时钟,ms)
    //                    → 即 相邻两条回复之间隔了多少毫秒"
    logLine(QStringLiteral("[帧 %1] CMD=0x%2 %3 | 完成时刻(abs): %4 | 距F0发送: %5 ms | 帧间隔: %6 ms | %7")
                .arg(m_frameCount)
                .arg(cmd, 2, 16, QChar('0'))
                .arg(cmdName)
                .arg(absTime)
                .arg(offsetFromSend >= 0 ? QString::number(offsetFromSend) : QStringLiteral("-"))
                .arg(interval >= 0 ? QString::number(interval) : QStringLiteral("-"))
                .arg(toHex(payload)));

    // OSD 数据帧：解析字符矩阵 + roll/pitch，矩阵累积进本批汇总块
    if (cmd == 0x57) {
        parseOsdData(payload);
        const QStringList mtx = osdMatrixLines();   // OSD 显示中也打印 20x53 矩阵
        for (const QString &ln : mtx)
            batchLogLine(ln);
        updateRollPitchSummary();
    }

    // 无线参数响应帧：按无线参数页面的解析方式，解析全部命令类型并累积进本批汇总
    bool isWirelessCmd = (cmd >= 0x50 && cmd <= 0x57) || (cmd >= 0xA0 && cmd <= 0xA7);
    if (isWirelessCmd) {
        QString detail = parseCommandDetail(cmd, payload);
        if (!detail.isEmpty())
            batchLogLine(QStringLiteral("解析[%1]: %2").arg(commandName(cmd)).arg(detail));
    }

    // 控件回填：快捷指令数据更新到无线参数页(普通/中继完全分离)
    //  - 0x51 普通频点 → freqUpdated(普通频率设置)
    //  - 0x53 普通功率 → powerUpdated(普通基带功率)
    //  - 0xA1 中继频点 → relayFreqUpdated(中继频率设置)
    //  - 0xA3 中继功率 → relayPowerUpdated(中继基带功率)
    if (cmd == 0x51 && payload.size() >= 4) {
        emit freqUpdated(static_cast<quint8>(payload.at(1)),
                         static_cast<quint8>(payload.at(2)),
                         static_cast<quint8>(payload.at(3)));
    } else if (cmd == 0x53 && payload.size() >= 6) {
        quint32 bitmap = static_cast<quint8>(payload.at(2)) |
                         (static_cast<quint8>(payload.at(3)) << 8) |
                         (static_cast<quint8>(payload.at(4)) << 16) |
                         (static_cast<quint8>(payload.at(5)) << 24);
        emit powerUpdated(static_cast<quint8>(payload.at(1)), bitmap);
    } else if (cmd == 0xA1 && payload.size() >= 4) {
        emit relayFreqUpdated(static_cast<quint8>(payload.at(1)),
                              static_cast<quint8>(payload.at(2)),
                              static_cast<quint8>(payload.at(3)));
    } else if (cmd == 0xA3 && payload.size() >= 6) {
        quint32 bitmap = static_cast<quint8>(payload.at(2)) |
                         (static_cast<quint8>(payload.at(3)) << 8) |
                         (static_cast<quint8>(payload.at(4)) << 16) |
                         (static_cast<quint8>(payload.at(5)) << 24);
        emit relayPowerUpdated(static_cast<quint8>(payload.at(1)), bitmap);
    }

    // 更新顶部状态
    ui->lbRxBytes->setText(QStringLiteral("接收字节: %1").arg(m_rxByteCount));
    ui->lbFrames->setText(QStringLiteral("完整帧: %1").arg(m_frameCount));
    ui->lbLastCmd->setText(QStringLiteral("最后命令: 0x%1").arg(cmd, 2, 16, QChar('0')));
    ui->lbLastInterval->setText(QStringLiteral("帧间隔: %1 ms")
                                    .arg(interval >= 0 ? QString::number(interval) : QStringLiteral("-")));
}

void F0ReadPage::onClear()
{
    m_rxBuf.clear();
    m_rxByteCount = 0;
    m_frameCount = 0;
    m_lastCmd = 0;
    m_lastIntervalMs = 0;
    m_hasLastFrame = false;
    m_lastFrameMs = 0;
    m_attitudeRowHistory.clear();
    m_attitudeRowTimestamps.clear();
    m_osdFramePayloads.clear();
    m_osdFrameTimestamps.clear();
    m_currentFrame = -1;
    m_playerPrepared = false;
    if (m_playTimer) m_playTimer->stop();
    if (m_osdGrid) m_osdGrid->clear();
    ui->btnPlay->setChecked(false);
    ui->btnPlay->setText(QStringLiteral("▶ 播放"));
    ui->lbFrameInfo->setText(QStringLiteral("帧: -/-"));
    ui->textLog->clear();
    ui->textLog->setPlainText(QStringLiteral("[已清空]"));
    ui->lbRxBytes->setText(QStringLiteral("接收字节: 0"));
    ui->lbFrames->setText(QStringLiteral("完整帧: 0"));
    ui->lbLastCmd->setText(QStringLiteral("最后命令: -"));
    ui->lbLastInterval->setText(QStringLiteral("帧间隔: -"));
}

// ---------------------------------------------------------------------
// 浏览选择 test.txt 文件
// ---------------------------------------------------------------------
void F0ReadPage::onBrowseFile()
{
    // 记忆上次浏览路径：优先用已填写的文件路径，否则用记忆的上次目录
    QString startPath;
    QSettings fset(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    if (!ui->editFilePath->text().isEmpty())
        startPath = ui->editFilePath->text();
    else
        startPath = fset.value(QStringLiteral("filedialog/f0/lastDir")).toString();

    QString path = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("选择 0x57 日志文件"),
            startPath,
            QStringLiteral("文本日志 (*.txt);;所有文件 (*)"));
    if (!path.isEmpty()) {
        ui->editFilePath->setText(path);
        fset.setValue(QStringLiteral("filedialog/f0/lastDir"), QFileInfo(path).absolutePath());
    }
}

// ---------------------------------------------------------------------
// 读取 test.txt 并离线解析
//
// 文件格式（每行）:
//   [10:41:37.463] 57 24 58 3E 00 B6 00 0B 01 35 A2 00 ...
//   即 [时间戳] 命令号(hex) + MSPV2/OSD 数据(hex 空格分隔)
// 我们把命令号 57 拿出来，把后面的 MSPV2 数据体喂给 parseOsdData 解析，
// 并打印时间戳、矩阵与姿态候选。
// ---------------------------------------------------------------------
void F0ReadPage::onLoadFile()
{
    QString path = ui->editFilePath->text().trimmed();
    if (path.isEmpty()) {
        logLine(QStringLiteral("[文件] 请先选择 test.txt 文件路径。"));
        return;
    }

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        logLine(QStringLiteral("[文件] 打开失败: %1").arg(path));
        return;
    }

    QTextStream in(&f);
    int lineNo = 0;
    int osdFrameCount = 0;

    // 清空上一轮离线解析的矩阵/统计
    onClear();

    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNo++;

        // 提取时间戳 [HH:mm:ss.zzz]
        QString ts;
        int lb = line.indexOf('[');
        int rb = line.indexOf(']');
        if (lb >= 0 && rb > lb)
            ts = line.mid(lb + 1, rb - lb - 1).trimmed();

        // 提取该行所有 hex 字节（先去掉 [时间戳] 段，避免把时间数字误抓成 hex）
        QByteArray bytes;
        QString body = line;
        int lb0 = line.indexOf('[');
        int rb0 = line.indexOf(']');
        if (lb0 >= 0 && rb0 > lb0)
            body = line.left(lb0) + line.mid(rb0 + 1);
        QRegularExpression hexRe(QStringLiteral("\\b[0-9A-Fa-f]{2}\\b"));
        auto it = hexRe.globalMatch(body);
        while (it.hasNext()) {
            bool ok = false;
            int v = it.next().captured().toInt(&ok, 16);
            if (ok) bytes.append((char)(v & 0xFF));
        }

        if (bytes.isEmpty())
            continue;

        // ---- 定位 0x57 OSD 帧 ----
        // 兼容两种日志格式：
        //   A) [时间戳] 57 24 58 3E ... D8        （命令号直接开头）
        //   B) FE EF A2 lenH lenL 57 24 58 3E ... D8 0A xx 0D 0A （外层封装帧）
        // 统一做法：在字节流中搜索 "57 24 58 3E" 特征（命令号+$X 帧头），
        //           从该位置截取完整 OSD 帧，再用 MSPV2 len 精确切掉行尾杂字节。
        int osdPos = -1;
        for (int k = 0; k + 3 < bytes.size(); ++k) {
            if ((quint8)bytes.at(k)     == 0x57 &&
                (quint8)bytes.at(k + 1) == 0x24 &&
                (quint8)bytes.at(k + 2) == 0x58 &&
                (quint8)bytes.at(k + 3) == 0x3E) {
                osdPos = k;
                break;
            }
        }
        if (osdPos < 0)
            continue;   // 该行不含 0x57 OSD 帧，跳过

        // 只关心 OSD 数据命令 0x57：从特征位置开始取完整 0x57 帧 + MSPV2 头
        QByteArray osdPayload = bytes.mid(osdPos);  // 57 24 58 3E ... (命令号+MSPV2头+条目)
        // 用 MSP V2 len 字段精确截断（此时 osdPayload[0]=57）：
        //   [0]=57(命令号) [1..8]=$X头 [7]=len_lo [8]=len_hi [9..]=payload [9+len]=crc
        if (osdPayload.size() >= 10) {
            int mspLen = ((quint8)osdPayload.at(7)) | (((quint8)osdPayload.at(8)) << 8);
            int totalLen = 9 + mspLen + 1; // 命令号 + 8字节日头 + payload + crc
            if (totalLen > 9 && totalLen < osdPayload.size())
                osdPayload.truncate(totalLen);  // 去掉行尾 0A xx 0D 0A 等杂字节
        }
        osdPayload.remove(0, 1);  // 去掉命令号 57 → 24 58 3E ... (MSPV2 头 + 条目)，与 parseOsdData 期望一致

        osdFrameCount++;
        // 累积到播放器帧缓存（完整 payload + 时间戳），供逐帧播放
        m_osdFramePayloads.append(osdPayload);
        m_osdFrameTimestamps.append(ts);

        logLine(QStringLiteral("┌─ [行 %1] 时间戳: %2 | CMD=0x57 OSD 帧#%3  (offset=%4)")
                    .arg(lineNo).arg(ts).arg(osdFrameCount).arg(osdPos));
        logLine(QStringLiteral("│  数据(%1 bytes): %2").arg(osdPayload.size())
                    .arg(toHex(osdPayload)));

        // 解析 OSD → 填充矩阵
        parseOsdData(osdPayload);

        // 打印矩阵与姿态候选
        printOsdMatrix();
        updateRollPitchSummary();

        // 累积本帧姿态条行（R10）内容，供文件末尾汇总逐帧变化
        m_attitudeRowHistory.append(osdRowText(10));
        m_attitudeRowTimestamps.append(ts);
        logLine(QStringLiteral("└─ 帧#%1 解析完成\n").arg(osdFrameCount));
    }

    f.close();

    // ---- 汇总：R10 姿态条 逐帧变化轨迹 ----
    logLine(QStringLiteral("\n====== R10 姿态条 逐帧变化轨迹（共 %1 帧）======").arg(m_attitudeRowHistory.size()));
    QString prevRow;
    for (int i = 0; i < m_attitudeRowHistory.size(); ++i) {
        const QString &rowText = m_attitudeRowHistory.at(i);
        QString mark = (rowText == prevRow) ? QStringLiteral("") : QStringLiteral("  <<变化");
        QString ts = (i < m_attitudeRowTimestamps.size()) ? m_attitudeRowTimestamps.at(i) : QString();
        logLine(QStringLiteral("#%1 %2 |%3|%4")
                    .arg(i, 3)
                    .arg(ts)
                    .arg(rowText)
                    .arg(mark));
        prevRow = rowText;
    }
    logLine(QStringLiteral("====== R10 姿态条汇总结束 ======"));

    // ---- 准备逐帧播放器 ----
    m_playerPrepared = (m_osdFramePayloads.size() > 0);
    if (m_playerPrepared) {
        m_currentFrame = 0;
        showOsdFrame(0);
        ui->btnPlay->setText(QStringLiteral("▶ 播放"));
        ui->lbFrameInfo->setText(QStringLiteral("帧: 1/%1").arg(m_osdFramePayloads.size()));
        logLine(QStringLiteral("[播放器] 已载入 %1 帧，点击\"▶ 播放\"按设定间隔逐帧显示。")
                    .arg(m_osdFramePayloads.size()));
    } else {
        ui->lbFrameInfo->setText(QStringLiteral("帧: -/-"));
        logLine(QStringLiteral("[播放器] 未解析到有效 OSD 帧，无法播放。"));
    }

    logLine(QStringLiteral("[文件] 解析结束：共 %1 条 0x57 OSD 帧（文件 %2 行）。")
                .arg(osdFrameCount).arg(lineNo));
}

// ---------------------------------------------------------------------
// 播放器：显示指定索引的帧（解析其 payload 并刷新 OSD Grid）
// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// 姿态条图形化：从 R10 姿态区提取 8 格偏移，传给 OsdGridWidget
// ---------------------------------------------------------------------
void F0ReadPage::updateAttitudeBar()
{
    if (!m_osdGrid) return;

    // 姿态条字符序列 LMNOPQRST (偏移 -4..+4)
    int left[4], right[4];
    for (int i = 0; i < 4; i++) {
        left[i]  = attitudeOffset(m_osdCharMap[10][21 + i]);
        right[i] = attitudeOffset(m_osdCharMap[10][28 + i]);
    }
    m_osdGrid->setAttitudeBar(left, right);
}

int F0ReadPage::attitudeOffset(quint16 cell) const
{
    static const char *seq = "LMNOPQRST";
    char ch = (char)(cell & 0xFF);
    if (ch < 32 || ch > 126) return -5;
    const char *p = strchr(seq, ch);
    if (!p) return -5;
    return (int)(p - seq) - 4; // L=-4 .. T=+4
}

void F0ReadPage::showOsdFrame(int index)
{
    if (index < 0 || index >= m_osdFramePayloads.size())
        return;
    m_currentFrame = index;

    // 解析该帧 OSD → 填充矩阵（verbose=true 打印 visualizeRawOsdData 风格日志）
    parseOsdData(m_osdFramePayloads.at(index), true);

    // 刷新 OSD 预览控件（把矩阵交给 OsdGridWidget 渲染，与 visualizeRawOsdData 一致）
    if (m_osdGrid) {
        m_osdGrid->setCharacterMap(m_osdCharMap);
        updateAttitudeBar(); // 姿态条图形化
    }

    // 更新帧信息
    QString ts = (index < m_osdFrameTimestamps.size()) ? m_osdFrameTimestamps.at(index) : QString();
    ui->lbFrameInfo->setText(QStringLiteral("帧: %1/%2  %3")
                                 .arg(index + 1).arg(m_osdFramePayloads.size()).arg(ts));
}

// ---------------------------------------------------------------------
// 播放/暂停
// ---------------------------------------------------------------------
void F0ReadPage::onPlayToggled(bool checked)
{
    if (!m_playerPrepared) {
        logLine(QStringLiteral("[播放器] 请先\"读取并解析\" test.txt 文件。"));
        ui->btnPlay->setChecked(false);
        ui->btnPlay->setText(QStringLiteral("▶ 播放"));
        return;
    }
    if (checked) {
        // 播放到最后则从头再来
        if (m_currentFrame >= m_osdFramePayloads.size() - 1)
            showOsdFrame(0);
        m_playTimer->setInterval(ui->spinFrameInterval->value());
        m_playTimer->start();
        ui->btnPlay->setText(QStringLiteral("⏸ 暂停"));
        logLine(QStringLiteral("[播放器] 播放中，帧间隔 %1 ms。").arg(ui->spinFrameInterval->value()));
    } else {
        m_playTimer->stop();
        ui->btnPlay->setText(QStringLiteral("▶ 播放"));
        logLine(QStringLiteral("[播放器] 已暂停。"));
    }
}

// ---------------------------------------------------------------------
// 播放定时器：切到下一帧
// ---------------------------------------------------------------------
void F0ReadPage::onPlayTimer()
{
    if (!m_playerPrepared || m_osdFramePayloads.isEmpty())
        return;
    // 每帧都刷新间隔（用户可能在播放中改 spin 值）
    m_playTimer->setInterval(ui->spinFrameInterval->value());

    int next = m_currentFrame + 1;
    if (next >= m_osdFramePayloads.size()) {
        // 播完回到开头（循环播放）
        next = 0;
    }
    showOsdFrame(next);
}

// ---------------------------------------------------------------------
// 上一帧 / 下一帧（手动步进）
// ---------------------------------------------------------------------
void F0ReadPage::onPrevFrame()
{
    if (!m_playerPrepared || m_osdFramePayloads.isEmpty())
        return;
    int idx = m_currentFrame - 1;
    if (idx < 0) idx = m_osdFramePayloads.size() - 1;
    showOsdFrame(idx);
}

void F0ReadPage::onNextFrame()
{
    if (!m_playerPrepared || m_osdFramePayloads.isEmpty())
        return;
    int idx = m_currentFrame + 1;
    if (idx >= m_osdFramePayloads.size()) idx = 0;
    showOsdFrame(idx);
}

// ---------------------------------------------------------------------
// 字体：按下拉框索引加载对应 OSD 字体样式
//   0-4 : SDK 图片字体（Betaflight/INAV/ArduPilot/Fettec/KISS, 24px）
//   5   : 内置字符集 osdChars（Qt 资源 / 源码目录 / exe 目录回退）
// ---------------------------------------------------------------------
QString F0ReadPage::applyOsdFont(int index)
{
    if (!m_osdGrid) return QStringLiteral("[字体] OSD 预览控件未创建。");

    QStringList names = { QStringLiteral("Betaflight"), QStringLiteral("INAV"),
                          QStringLiteral("ArduPilot"), QStringLiteral("Fettec"),
                          QStringLiteral("KISS") };

    if (index >= 0 && index <= 4) {
        // SDK 图片字体（Betaflight 样式等）
        if (m_osdGrid->loadFontByIndex(index, false)) {
            return QStringLiteral("[字体] 已加载 %1 图片字体 (SDK 24px)。").arg(names.at(index));
        }
        // 尝试 1080 版本
        if (m_osdGrid->loadFontByIndex(index, true)) {
            return QStringLiteral("[字体] 已加载 %1 图片字体 (SDK 1080 36px)。").arg(names.at(index));
        }
        // 回退到内置字符集
        index = 5;
    }

    if (index == 5) {
        // 内置字符集 osdChars720_new（优先源码目录 24x36 字库，用户指定的数字字形）
        // qrc :/osdchars 映射的是 osdChars_new(36x54)，其中 0xBA→186.png 是 "4"，
        // 与 osdChars720_new 的 186.png="9" 不同，因此必须优先加载 osdChars720_new。
        QString fontDir = QFileInfo(QStringLiteral(__FILE__)).absolutePath() + "/osdChars720_new";
        if (QDir(fontDir).exists()) {
            if (m_osdGrid->loadCharImages(fontDir, 512))
                return QStringLiteral("[字体] 已加载内置字符集 (源码目录 osdChars720_new)。");
        }
        QString exeDir = QCoreApplication::applicationDirPath() + "/osdChars720_new";
        if (QDir(exeDir).exists()) {
            if (m_osdGrid->loadCharImages(exeDir, 512))
                return QStringLiteral("[字体] 已加载内置字符集 (exe 目录 osdChars720_new)。");
        }
        // 回退到 Qt 资源 osdChars（osdChars_new）
        if (QFile::exists(":/osdchars/0.png")) {
            if (m_osdGrid->loadCharImages(":/osdchars", 512))
                return QStringLiteral("[字体] 已加载内置字符集 (Qt 资源 :/osdchars)。");
        }
        m_osdGrid->unloadFont();
        return QStringLiteral("[字体] 警告: 未找到任何 OSD 字体，播放器使用回退文本显示。");
    }

    m_osdGrid->unloadFont();
    return QStringLiteral("[字体] 未知字体索引 %1，使用回退显示。").arg(index);
}

void F0ReadPage::onFontChanged(int index)
{
    QString msg = applyOsdFont(index);
    logLine(msg);
    // 刷新当前帧（字体变了要重画）
    if (m_currentFrame >= 0 && m_currentFrame < m_osdFramePayloads.size())
        showOsdFrame(m_currentFrame);
}

// ---------------------------------------------------------------------
// 工具
// ---------------------------------------------------------------------
QString F0ReadPage::toHex(const QByteArray &data)
{
    return QString::fromLatin1(data.toHex(' ').toUpper());
}

void F0ReadPage::logLine(const QString &line)
{
    ui->textLog->appendPlainText(line);
    // 自动滚到底
    QTextCursor c = ui->textLog->textCursor();
    c.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(c);
}

// 根据 cmd 给个可读名称，方便对照 asvrx_async.c
QString F0ReadPage::commandName(quint8 cmd)
{
    switch (cmd) {
    case 0x50: return QStringLiteral("设置频点");
    case 0x51: return QStringLiteral("频率(Freq)");
    case 0x52: return QStringLiteral("无线状态(Status)");
    case 0x53: return QStringLiteral("功率(BB Pwr)");
    case 0x55: return QStringLiteral("天空端状态(Sky)");
    case 0x56: return QStringLiteral("距离(Distance)");
    case 0x57: return QStringLiteral("OSD数据");
    case 0xA0: return QStringLiteral("设置中继频点");
    case 0xA1: return QStringLiteral("中继频率(Relay Freq)");
    case 0xA2: return QStringLiteral("中继状态(Relay Status)");
    case 0xA3: return QStringLiteral("中继功率(Relay BB Pwr)");
    case 0xA4: return QStringLiteral("设置中继功率");
    case 0xA5: return QStringLiteral("中继-地面距离");
    case 0xA6: return QStringLiteral("中继-天空距离");
    case 0xA7: return QStringLiteral("中继OSD数据");
    default:   return QStringLiteral("未知");
    }
}

// ---------------------------------------------------------------------
// 解析 0x57 的 OSD 数据，构建 20x53 字符矩阵
// 参考: mainwindow.cpp visualizeRawOsdData
//   payload 可能是：
//     1) 直接是 MSP DISPLAYPORT 子命令流（0x35 整帧 / 0x05 文本）
//     2) 带 MSP V2 头（$X> ... cmd=0x00B6 ... payload ... crc）
// ---------------------------------------------------------------------
void F0ReadPage::parseOsdData(const QByteArray &osdPayload, bool verbose)
{
    const quint8 *osd = reinterpret_cast<const quint8*>(osdPayload.constData());
    int osdSize = osdPayload.size();

    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));
    if (osdSize <= 0) return;

    int parsedCount = 0;

    // ---- 像 visualizeRawOsdData 一样打印调试信息 ----
    if (verbose) {
        logLine(QStringLiteral("visualizeRawOsdData: 总数据 %1 bytes").arg(osdSize));
        logLine(QStringLiteral("OSD包找到 offset=0, dataLen=%1, osdSize=%2")
                    .arg(osdSize).arg(osdSize));
        QString headerHex;
        for (int i = 0; i < osdSize; i++)
            headerHex += QStringLiteral("%1 ").arg(osd[i], 2, 16, QChar('0'));
        logLine(QStringLiteral("osd数据头: %1").arg(headerHex));
    }

    // ---- 跳过 MSP V2 头部 ----
    // 格式: $X [dir] [flags] [cmd_LE(2)] [len_LE(2)] [payload...] [crc]
    int dataStart = 0;
    int maxParseEnd = osdSize;
    int mspV2PayloadLen = 0;

    // 本工程 OSD 数据是 MSPV2 变体：3 字节头，无 flags 字节；
    //   24 58 3E [dir] [cmd_lo cmd_hi] [len_lo len_hi] [payload...] [crc]
    //   即 len_lo=osd[6], len_hi=osd[7]
    if (osdSize >= 9 && osd[0] == 0x24 && osd[1] == 0x58 && osd[2] == 0x3E) {
        dataStart = 8;  // 跳过 8 字节 MSPV2 头
        mspV2PayloadLen = osd[6] | (osd[7] << 8); // len LE
        if (mspV2PayloadLen > 0 && dataStart + mspV2PayloadLen < maxParseEnd)
            maxParseEnd = dataStart + mspV2PayloadLen; // 截掉末尾 CRC

        // 打印 MSP V2 头信息
        if (verbose) {
            int mspCmd = osd[4] | (osd[5] << 8);   // cmd LE: 00 B6 -> 0xB600
            bool crc = (dataStart + mspV2PayloadLen <= osdSize); // 有 CRC 空间
            logLine(QStringLiteral("跳过 MSP V2 头: cmd=0x%1 len=%2 crc=%3")
                        .arg(mspCmd, 4, 16, QChar('0'))
                        .arg(mspV2PayloadLen)
                        .arg(crc ? QStringLiteral("yes") : QStringLiteral("no")));
        }
    }

    // ---- 解析 DISPLAYPORT 子命令 ----
    // 本数据实际格式: 35 [seq] [rsv] 后紧接 [len][row][col][attr][chars] 条目流
    // 注意: 条目长度 len 常为 0x05/0x06/0x07 等, 与子命令号"冲突"
    //       因此这里以 0x35 整帧为主路径, 专一地按条目流解析
    int p = dataStart;
    while (p < maxParseEnd) {
        quint8 subcmd = osd[p];

        if (subcmd == 0x35) {
            // 整帧: 0x35 [seq] [rsv] [entry...]
            // entry: [len][row][col][attr][chars...]
            if (p + 3 > maxParseEnd) break;
            int entryPos = p + 3;
            while (entryPos + 4 <= maxParseEnd) {
                int entryLen = osd[entryPos];
                if (entryLen < 4) break;
                int textLen = entryLen - 4;
                if (entryPos + entryLen > maxParseEnd) break;
                int row = osd[entryPos + 1];
                int col = osd[entryPos + 2];
                int rawAttr = osd[entryPos + 3];
                quint16 attrs = (quint16)((rawAttr & 0x03) << 8);
                if (row >= 0 && row < OSD_ROWS && col >= 0 && col < OSD_COLS) {
                    for (int i = 0; i < textLen && (col + i) < OSD_COLS; ++i)
                        m_osdCharMap[row][col + i] = (quint16)osd[entryPos + 4 + i] | attrs;
                    parsedCount++;
                }
                entryPos += entryLen;
            }
            p = entryPos;
        }
        else if (subcmd == 0x05) {
            // 文本: 0x05 [row] [col] [attr] [chars...]
            if (p + 4 > maxParseEnd) break;
            int row = osd[p + 1];
            int col = osd[p + 2];
            int rawAttr = osd[p + 3];
            quint16 attrs = (quint16)((rawAttr & 0x03) << 8);
            int textStart = p + 4;
            int textEnd;
            for (textEnd = textStart; textEnd < maxParseEnd; textEnd++) {
                quint8 b = osd[textEnd];
                if (b == 0x00 || b == 0x05 || b == 0x09 || b == 0x0A ||
                    b == 0x35 || b >= 0x80)
                    break;
            }
            int textLen = textEnd - textStart;
            if (row >= 0 && row < OSD_ROWS) {
                for (int i = 0; i < textLen && (col + i) < OSD_COLS; ++i)
                    m_osdCharMap[row][col + i] = (quint16)osd[textStart + i] | attrs;
                parsedCount++;
            }
            p = qMax(textEnd, p + 4);
        }
        else if (subcmd == 0x09 || subcmd == 0x0A || subcmd == 0x00) {
            p++; // DRAW_SCREEN / CLEAR_SCREEN / 填充零
        }
        else if (subcmd >= 4) {
            // 容错: 按 entry 格式 [len][row][col][attr][chars...]
            int entryLen = subcmd;
            if (p + entryLen <= maxParseEnd) {
                int textLen = entryLen - 4;
                if (textLen > 0 && textLen < 60) {
                    int row = osd[p + 1];
                    int col = osd[p + 2];
                    int rawAttr = osd[p + 3];
                    quint16 attrs = (quint16)((rawAttr & 0x03) << 8);
                    if (row >= 0 && row < OSD_ROWS && col >= 0 && col < OSD_COLS) {
                        for (int i = 0; i < textLen && (col + i) < OSD_COLS; ++i)
                            m_osdCharMap[row][col + i] = (quint16)osd[p + 4 + i] | attrs;
                        parsedCount++;
                    }
                    p += entryLen;
                    continue;
                }
            }
            p++;
        }
        else {
            p++;
        }
    }

    // ---- 打印解析结果（verbose 模式，像 visualizeRawOsdData）---
    if (verbose) {
        logLine(QStringLiteral("解析了 %1 个条目").arg(parsedCount));
        logLine(QStringLiteral("--- OSD 内容 ---"));
        for (int row = 0; row < OSD_ROWS; ++row) {
            QString line;
            bool hasContent = false;
            for (int col = 0; col < OSD_COLS; ++col) {
                quint16 v = m_osdCharMap[row][col];
                quint8 ch = (quint8)(v & 0xFF);
                if (ch == 0) {
                    line += ' ';
                } else if (ch >= 0x20 && ch <= 0x7E) {
                    line += QChar(ch);
                    hasContent = true;
                } else {
                    line += '?';
                    hasContent = true;
                }
            }
            if (hasContent)
                logLine(QStringLiteral("[%1] %2").arg(row).arg(line));
        }
        logLine(QStringLiteral("✓ OSD: %1 条目 (%2 bytes)").arg(parsedCount).arg(osdSize));
    }
}

// ---------------------------------------------------------------------
// 打印 20x53 OSD 字符矩阵
// 可打印 ASCII 直接显示，非 ASCII 图形字符用 [xx] 表示；
// 空字符用空格。只打印非空行，避免刷屏。
// ---------------------------------------------------------------------
// 提取矩阵中指定行的可读文本（非 ASCII 图形字符用 [xx] 表示）
QString F0ReadPage::osdRowText(int row) const
{
    QString line;
    if (row < 0 || row >= OSD_ROWS) return line;
    for (int col = 0; col < OSD_COLS; ++col) {
        quint8 ch = (quint8)(m_osdCharMap[row][col] & 0xFF);
        if (ch == 0)
            line += ' ';
        else if (ch >= 0x20 && ch <= 0x7E)
            line += QChar(ch);
        else
            line += QStringLiteral("[%1]").arg(ch, 2, 16, QChar('0'));
    }
    return line;
}

QStringList F0ReadPage::osdMatrixLines() const
{
    QStringList out;
    out << QStringLiteral("--- OSD 20x53 矩阵 ---");
    for (int row = 0; row < OSD_ROWS; ++row) {
        QString line;
        bool hasContent = false;
        for (int col = 0; col < OSD_COLS; ++col) {
            quint16 v = m_osdCharMap[row][col];
            quint8 ch = static_cast<quint8>(v & 0xFF);
            if (ch == 0) {
                line += ' ';
            } else if (ch >= 0x20 && ch <= 0x7E) {
                line += QChar(ch);
                hasContent = true;
            } else {
                // 非 ASCII 图形字符（姿态条等）
                line += QStringLiteral("[%1]").arg(ch, 2, 16, QChar('0'));
                hasContent = true;
            }
        }
        if (hasContent)
            out << QStringLiteral("R %1: %2").arg(row).arg(line);
    }
    out << QStringLiteral("--- 矩阵结束 ---");
    return out;
}

void F0ReadPage::printOsdMatrix()
{
    const QStringList lines = osdMatrixLines();
    for (const QString &ln : lines)
        logLine(ln);
}

// ---------------------------------------------------------------------
// 提取并打印 roll(横滚)/pitch(俯仰)
//
// 说明：不同飞控 OSD 布局不同，roll/pitch 的确切位置不固定。
// 姿态通常以两种形式出现：
//   1) 数值：如 "010" / " 12" / "-45"（含 +/- 号 2~4 位数字）
//   2) 图形条：非 ASCII 图形字符铺成的刻度条（多行长条）
// 我们扫描矩阵，把"含数字/符号的姿态特征行"打印出来，
// 供你对照屏幕确认 roll/pitch 所在行。
// ---------------------------------------------------------------------
void F0ReadPage::updateRollPitchSummary()
{
    logLine(QStringLiteral("--- 姿态候选（含数字行，供定位 roll/pitch）--"));
    for (int row = 0; row < OSD_ROWS; ++row) {
        QString text;
        for (int col = 0; col < OSD_COLS; ++col) {
            quint8 ch = (quint8)(m_osdCharMap[row][col] & 0xFF);
            if (ch == 0) text += ' ';
            else if (ch >= 0x20 && ch <= 0x7E) text += QChar(ch);
        }
        // 姿态数值特征：包含 2~4 位连续数字，或 +/- 引导的数字
        if (text.contains(QRegularExpression(QStringLiteral("[-+]?\\d{2,4}"))) &&
            !text.contains(QRegularExpression(QStringLiteral("\\d{2}:\\d{2}")))) {
            logLine(QStringLiteral("  R%1: '%2'").arg(row, 2).arg(text.trimmed()));
        }
    }
    logLine(QStringLiteral("--- 姿态候选结束 ---"));
}
