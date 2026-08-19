#ifndef F0READPAGE_H
#define F0READPAGE_H

#include <QWidget>
#include <QByteArray>
#include <QElapsedTimer>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVBoxLayout>
#include <QSplitter>
#include <cstring>

class QFile;

class SerialCommunicator;
class OsdGridWidget;

namespace Ui {
class PageSerialF0Read;
}

/**
 * F0 读取测试页面
 *
 * 功能：
 *   1. 发送 F0 指令（FE EF A2 00 08 F0 00 00 00 00 00 00 00 00 CS CS 0D 0A）
 *   2. 一个字节一个字节地接收并解析（模拟下位机 asvrx_async.c 的 parse_rx_packet，
 *      但改为逐字节喂入状态机）
 *   3. 每解析出一条完整协议帧，打印该帧的命令号与接收完成时间戳（ms 级）
 *
 * 时间戳使用 QElapsedTimer 单调时钟（不受系统时间跳变影响），
 * 并同时给出相对"发送 F0"的毫秒偏移，方便直接观察各条命令到达间隔。
 */
class F0ReadPage : public QWidget
{
    Q_OBJECT
public:
    explicit F0ReadPage(SerialCommunicator *comm, QWidget *parent = nullptr);
    ~F0ReadPage();

signals:
    // 每次发送 F0 快捷指令时发出，供 MainWindow 清理无线参数页的历史缓冲
    void f0CommandSent();
    // 快捷指令收到频点响应(0x51)：band/通道/跳频模式
    void freqUpdated(int band, int channel, int hop);
    // 快捷指令收到功率响应(0x53)：当前功率索引 + 可设置位图
    void powerUpdated(int pwrIdx, quint32 bitmap);

private slots:
    void onSendF0();
    void onAutoPollToggled(bool checked);
    void onClear();
    void onDataReceived(const QByteArray &data);
    void onPollTimer();
    void onBrowseFile();
    void onLoadFile();
    void onPlayToggled(bool checked);
    void onPrevFrame();
    void onNextFrame();
    void onPlayTimer();
    void onFontChanged(int index);
    void onSaveRawToggled(bool checked);

private:
    // 构建并发送 F0 命令
    void sendF0Command();
    quint16 calcChecksum(const QByteArray &data) const;

    // ---- 原始数据记录到文件（带时间戳）----
    void startRawLog();          // 弹窗选择保存路径并开始记录
    void stopRawLog();           // 停止记录并关闭文件
    void beginRawBatch(const QString &ts, const QByteArray &txBytes); // 开新批次(发送F0时)，记录发送字节
    void accumulateFrame(const QByteArray &fullFrame); // 追加一条完整响应帧(含帧头尾)到当前批次
    void flushRawBatch();        // 把当前批次(发送F0+全部响应)写成一行

    // 逐字节解析：把每个字节喂入状态机
    void feedByte(quint8 b);

    // 一条完整帧解析完成时调用
    void onFrameParsed(const QByteArray &frame);

    // ---- 快捷指令批次解析汇总 ----
    // 每次发送 F0 视为一批；本批所有响应的字段级解析累积，统一成一块输出
    void flushBatchParse();          // 把 m_batchParseLines 汇总成一块打印到日志
    void resetBatchFlushTimer();     // 重置批次收尾定时器(收到帧时调用)
    void batchLogLine(const QString &line); // 把一行解析追加到本批汇总(等待统一输出)
    // 参考无线参数页：对单条响应做字段级解析，返回多行描述(不含命令头)
    QString parseCommandDetail(quint8 cmd, const QByteArray &payload) const;

    // ---- OSD 解析（参考 mainwindow.cpp visualizeRawOsdData）----
    // 解析 0x57 OSD 数据：跳 MSP V2 头 → 解析 0x35/0x05 子命令 → 构建20x53字符矩阵
    // verbose=true 时打印 visualizeRawOsdData 风格的详细调试日志（播放器/离线模式用）
    void parseOsdData(const QByteArray &osdPayload, bool verbose = false);
    // 打印 20x53 矩阵（含非打印字符转义），并单独打印 roll/pitch 姿态相关行
    void printOsdMatrix();
    // 返回 20x53 矩阵的文本行列表（含 "--- OSD 20x53 矩阵 ---" 头尾，只含有内容行）
    QStringList osdMatrixLines() const;
    // 提取矩阵中指定行的可读文本
    QString osdRowText(int row) const;
    // 从 R10 姿态区(C21-C24 左, C28-C31 右)提取 8 格偏移, 传给 OsdGridWidget 图形化
    void updateAttitudeBar();
    // 单个字符 -> 姿态偏移 (-4..+4, -5=空)
    int attitudeOffset(quint16 cell) const;

    // OSD 播放器：显示指定索引帧 + 步进
    void showOsdFrame(int index);
    // 按下拉框索引加载字体（0-4 = SDK 图片字体, 5 = 内置 osdChars），返回结果消息
    QString applyOsdFont(int index);
    // 更新状态栏的 roll/pitch 摘要
    void updateRollPitchSummary();

    // 格式化成十六进制字符串
    static QString toHex(const QByteArray &data);
    static QString commandName(quint8 cmd);

    void logLine(const QString &line);

    Ui::PageSerialF0Read *ui;
    QWidget *m_pageWidget;
    SerialCommunicator *m_comm;

    // 控制区 / 日志显示区 的分割条（可上下拖动，位置持久化）
    QSplitter *m_splitter = nullptr;
    void setupSplitter();
    void saveSplitterState();
    void restoreSplitterState();

    // ---- 逐字节解析状态（对应 asvrx_async.c 的 rx_raw 状态机）----
    QByteArray m_rxBuf;        // 累积的原始接收字节
    qint64 m_lastByteMs;       // 上一个字节到达的毫秒时间戳（单调时钟）

    // ---- 帧间隔统计 ----
    QElapsedTimer m_elapsed;   // 单调时钟，epoch 从页面创建时开始
    qint64 m_sendF0Ms = 0;     // 最近一次发送 F0 的时刻（单调时钟）
    qint64 m_lastFrameMs = 0;  // 最近一条完整帧解析完成的时刻
    bool m_hasLastFrame = false;

    // ---- 统计 ----
    quint64 m_rxByteCount = 0;
    quint64 m_frameCount = 0;
    quint8  m_lastCmd = 0;
    qint64  m_lastIntervalMs = 0;

    // ---- 自动轮询 ----
    QTimer *m_pollTimer = nullptr;

    // ---- 原始数据记录（带时间戳写入文件）----
    QFile    *m_rawLogFile = nullptr; // 当前打开的记录文件（nullptr = 未记录）
    QString   m_rawLogPath;           // 记录文件完整路径
    quint64   m_rawLogBytes = 0;      // 本次记录已写入的字节数
    QByteArray m_rawLogBatch;         // 当前批次累积的字节（发送F0 + 所有完整响应帧）
    QString    m_rawLogBatchTs;       // 当前批次(发送F0)的绝对时间
    bool       m_rawLogBatchActive = false; // 是否有未写出的批次

    // ---- OSD 字符矩阵 ----
    static constexpr int OSD_ROWS = 20;
    static constexpr int OSD_COLS = 53;
    quint16 m_osdCharMap[OSD_ROWS][OSD_COLS] = {{0}};

    // ---- 离线文件解析时累积的每帧姿态行历史 ----
    QStringList m_attitudeRowHistory;   // 每帧 R10(姿态条) 行的内容
    QStringList m_attitudeRowTimestamps;// 对应的帧时间戳

    // ---- OSD 逐帧播放器 ----
    OsdGridWidget *m_osdGrid = nullptr;   // OSD 预览控件
    QList<QByteArray> m_osdFramePayloads; // 每帧 0x57 的完整数据体（含 MSPV2 头）
    QStringList m_osdFrameTimestamps;     // 每帧时间戳
    int m_currentFrame = -1;              // 当前播放到第几帧
    QTimer *m_playTimer = nullptr;        // 播放定时器
    bool m_playerPrepared = false;        // 是否已加载好帧序列

    // ---- 快捷指令批次解析汇总 ----
    QStringList m_batchParseLines;        // 本批(F0一次)所有响应的字段级解析行
    QString     m_batchStartTs;           // 本批开始(发送F0)时间
    int         m_batchSeq = 0;           // 批次序号
    QTimer     *m_batchFlushTimer = nullptr; // 批次收尾定时器(无新帧后输出汇总块)
};

#endif // F0READPAGE_H
