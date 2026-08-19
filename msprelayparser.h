#ifndef MSPRELAYPARSER_H
#define MSPRELAYPARSER_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QString>
#include <QDateTime>

// ============================================================
// MSP Relay Parser — 独立的中继器 MSP 数据解析器
//
// 背景:
//   中继器(relay)发来的 MSP 数据不是完整的 MSP 帧。
//   它可能是:
//     1) 无 FE EF A2 ... 57 ... 0D 0A 外层封装的裸 MSP V2 payload
//     2) MSP V2 帧被分片传输 (24 58 3E .../24 58 4D ... 头部与 payload 分离)
//     3) 直接是 MSP DISPLAYPORT 子命令数据 (35 seq rsv len row col attr text...)
//     4) 混合了其他协议数据的字节流
//
// 本类独立于 GUI，负责:
//   - 维护接收缓冲区，从字节流中识别/提取 MSP 相关数据
//   - 对分片数据进行重组(等待完整 payload)
//   - 解析 MSP DISPLAYPORT 子命令，输出 OSD 字符矩阵
//   - 提供解析回调/查询接口
//
// 使用方法:
//   MspRelayParser parser;
//   connect(&parser, &MspRelayParser::osdFrameReady, ...);
//   parser.feedData(rxBytes);   // 每次串口收到数据就调用
//
// ============================================================

// OSD 网格尺寸 (固定 20行 x 53列)
static constexpr int MSP_OSD_ROWS = 20;
static constexpr int MSP_OSD_COLS = 53;

// MSP V2 协议常量
static constexpr quint8 MSP_V2_HEADER_MARK    = 0x24;  // '$'
static constexpr quint8 MSP_V2_HEADER_X       = 0x58;  // 'X'
static constexpr quint8 MSP_V2_DIR_INBOUND    = 0x3E;  // '>'
static constexpr quint8 MSP_V2_DIR_OUTBOUND   = 0x4D;  // 'M'
static constexpr quint8 MSP_V2_HEADER_SIZE    = 8;     // $X> + flags + cmd(2) + size(2)
static constexpr quint16 MSP_CMD_DISPLAYPORT  = 0x00B6; // 182

// MSP DISPLAYPORT 子命令
// 注意: 本项目 SDK 使用 0x05 作为文本命令 (非标准 MSP 的 0x03)
// 参考 mainwindow.cpp 的 visualizeRawOsdData:
//   0x35 [seq] [rsv] -- 整帧更新 (自定义)
//   0x05 [row] [col] [attr] [chars...] -- 文本 (SDK 自定义)
//   0x02 -- 清屏 (SDK 自定义值, 非标准 MSP 的 0x0C)
//   0x04 -- 刷新屏幕
enum MspDisplaySubCmd : quint8 {
    MSP_DISPLAY_HEARTBEAT           = 0x00,
    MSP_DISPLAY_RELEASE             = 0x01,
    MSP_DISPLAY_CLEAR_SCREEN_SDK    = 0x02, // SDK 清屏
    MSP_DISPLAY_TEXT_SDK            = 0x05, // SDK 文本 (非标准 0x03)
    MSP_DISPLAY_DRAW_SCREEN_SDK     = 0x04, // SDK 刷新
    MSP_DISPLAY_DRAW_NORMAL_FULL_PKT = 0x35, // 自定义整帧
};

// OSD 字符属性掩码
enum MspOsdAttrBits : quint16 {
    MSP_OSD_ATTR_BLINK   = 0x01,
    MSP_OSD_ATTR_INVERT  = 0x02,
    MSP_OSD_ATTR_SOLID   = 0x04,
    MSP_OSD_ATTR_PAGE    = 0x0300,  // 高低位页选择 (低位2bit, 映射到 font page)
};

// 单条 OSD 解析结果
struct MspOsdSegment {
    int row = 0;
    int col = 0;
    quint16 attr = 0;
    QByteArray text;      // UTF-8 / ASCII 字符
};

// 单次 OSD 帧解析结果 (一个 0x35 或 0x05 子命令)
struct MspOsdFrame {
    quint8 subCmd = 0;
    quint8 seq = 0;               // 帧序号 (0x35 时有效)
    QVector<MspOsdSegment> segments;
    QString error;                 // 非空表示解析出错
    QDateTime rxTime;              // 接收完成时间
};

// 解析统计信息
struct MspRelayStats {
    quint64 totalBytesFed = 0;     // 总输入字节
    quint64 mspFramesFound = 0;    // 识别到的 MSP V2 帧
    quint64 mspFramesComplete = 0; // 组装完成的 MSP V2 帧
    quint64 displayportParsed = 0; // 成功解析的 DISPLAYPORT 子命令数
    quint64 malformedDropped = 0;  // 因格式错误丢弃的块数
    quint64 partialPending = 0;    // 等待更多数据的分片数
    QString lastError;
};

class MspRelayParser : public QObject
{
    Q_OBJECT
public:
    explicit MspRelayParser(QObject *parent = nullptr);

    // ---- 数据输入 ----
    // 串口/TCP/UDP 等收到的任意字节块，直接喂进来。
    // 内部自动扫描、提取、重组 MSP 数据。
    void feedData(const QByteArray &data);
    void feedData(const char *data, int len);

    // ---- 状态查询 ----
    MspRelayStats stats() const { return m_stats; }
    void resetStats();
    void clearBuffer();   // 清空内部重组缓冲区

    // OSD 当前字符矩阵 (解析完成后更新)
    const quint16 (*osdCharMap() const)[MSP_OSD_COLS] { return m_charMap; }
    quint16 charAt(int row, int col) const;

    // 最近一帧的原始数据 (调试用)
    QByteArray lastRawFrame() const { return m_lastRawFrame; }

    // 启停调试日志
    void setDebugEnabled(bool en) { m_debug = en; }
    bool debugEnabled() const { return m_debug; }

signals:
    // OSD 帧解析完成 (0x35 整帧 / 0x05 文本 / 0x02 清屏等)
    void osdFrameReady(const MspOsdFrame &frame);

    // 完整 20x53 字符矩阵更新 (有效显示内容变化时触发)
    void osdGridUpdated();

    // 原始 MSP V2 帧组装完成 (包含头部，调试用)
    void mspV2FrameReady(const QByteArray &fullFrame);

    // 解析错误
    void parseError(const QString &msg);

private:
    // 扫描并提取数据
    void processBuffer();

    // 从 MSP V2 头部解析长度
    bool tryParseMspV2Header(const quint8 *buf, int avail,
                             int &headerLen, int &payloadLen,
                             bool &hasCrc, int &cmd);

    // 处理一段 MSP payload (可能含 0x35/0x05 等子命令)
    void parseDisplayportPayload(const quint8 *data, int len);

    // 解析一条 0x35 整帧子命令
    void parseFullPacket(const quint8 *data, int len, int &consumed);

    // 解析 0x05 文本子命令 (标准 MSP_DISPLAY_TEXT)
    void parseTextCommand(const quint8 *data, int len, int &consumed);

    // 将字符写入矩阵
    void writeChar(int row, int col, quint16 ch, quint16 attr);

    // 校验 MSP V2 CRC (X.25)
    static quint8 mspV2Crc8(const quint8 *data, int len);

    // 清屏
    void clearScreen();

    void debugLog(const QString &msg) const;

private:
    QByteArray m_buffer;        // 重组缓冲区
    MspRelayStats m_stats;
    quint16 m_charMap[MSP_OSD_ROWS][MSP_OSD_COLS]; // 字符矩阵 [row][col]
    QByteArray m_lastRawFrame;
    QVector<MspOsdSegment> m_lastSegments;  // 最近解析的 OSD 条目
    bool m_debug = false;
};

#endif // MSPRELAYPARSER_H
