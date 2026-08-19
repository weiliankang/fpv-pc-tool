#include "msprelayparser.h"
#include <QDebug>
#include <cstring>
#include <cstdio>

// ============================================================
// MspRelayParser 实现
// ============================================================

MspRelayParser::MspRelayParser(QObject *parent)
    : QObject(parent)
{
    memset(m_charMap, 0, sizeof(m_charMap));
}

// ------------------------------------------------------------------
// 数据输入
// ------------------------------------------------------------------
void MspRelayParser::feedData(const QByteArray &data)
{
    if (data.isEmpty()) return;
    feedData(data.constData(), data.size());
}

void MspRelayParser::feedData(const char *data, int len)
{
    if (!data || len <= 0) return;

    m_buffer.append(data, len);
    m_stats.totalBytesFed += len;
    processBuffer();
}

// ------------------------------------------------------------------
// 缓冲区扫描与提取
//
// 支持以下格式:
//   A) 完整的 MSP V2 帧:  $X> flags cmd(2) size(2) payload... crc
//   B) 裸 MSP payload:    直接是 0x35/0x05 等子命令开头的数据
//   C) 分包数据:          首包只有头部,需等后续数据补齐 payload
// ------------------------------------------------------------------
void MspRelayParser::processBuffer()
{
    if (m_buffer.isEmpty()) return;

    const quint8 *buf = reinterpret_cast<const quint8*>(m_buffer.constData());
    int avail = m_buffer.size();

    // 1) 扫描是否存在 MSP V2 头部 ($X> 或 $XM)
    int mspV2Pos = -1;
    for (int i = 0; i <= avail - 2; ++i) {
        if (buf[i] == MSP_V2_HEADER_MARK && buf[i+1] == MSP_V2_HEADER_X) {
            mspV2Pos = i;
            break;
        }
    }

    if (mspV2Pos < 0) {
        // 没有 MSP V2 头。尝试直接按裸 DISPLAYPORT payload 解析。
        // 特征是首字节为 0x35 / 0x05 / 0x02 / 0x04 等子命令。
        quint8 first = buf[0];
        if (first == MSP_DISPLAY_DRAW_NORMAL_FULL_PKT ||
            first == MSP_DISPLAY_TEXT_SDK ||
            first == MSP_DISPLAY_CLEAR_SCREEN_SDK ||
            first == MSP_DISPLAY_DRAW_SCREEN_SDK ||
            first == MSP_DISPLAY_HEARTBEAT) {
            // 裸 payload 直接解析
            parseDisplayportPayload(buf, avail);
            m_buffer.clear();
            return;
        }
        // 无法识别的数据，丢弃全部（避免无限堆积）
        if (avail > 4096) {
            m_stats.malformedDropped++;
            m_stats.lastError = "无法识别的 MSP 数据流";
            emit parseError(m_stats.lastError);
            m_buffer.clear();
        }
        return;
    }

    // 2) 有 MSP V2 头，丢弃头之前的垃圾数据
    if (mspV2Pos > 0) {
        m_buffer.remove(0, mspV2Pos);
        buf = reinterpret_cast<const quint8*>(m_buffer.constData());
        avail = m_buffer.size();
    }

    // 3) 解析 MSP V2 头部，获取 payload 长度
    int headerLen = 0, payloadLen = 0;
    bool hasCrc = false;
    int cmd = 0;
    if (!tryParseMspV2Header(buf, avail, headerLen, payloadLen, hasCrc, cmd)) {
        // 头部不完整，等更多数据
        m_stats.partialPending++;
        debugLog(QString("头部不完整, avail=%1").arg(avail));
        return;
    }

    m_stats.mspFramesFound++;
    debugLog(QString("MSP V2 头部: len=%1 cmd=0x%2 crc=%3")
                 .arg(payloadLen).arg(cmd, 4, 16, QChar('0'))
                 .arg(hasCrc ? "yes" : "no"));

    int totalLen = headerLen + payloadLen + (hasCrc ? 1 : 0);
    if (avail < totalLen) {
        // payload 还没到齐，继续等待
        m_stats.partialPending++;
        debugLog(QString("等待 MSP V2 payload: %1/%2 bytes")
                     .arg(avail).arg(totalLen));
        return;
    }

    // 4) 完整帧到齐，取出
    QByteArray fullFrame = m_buffer.left(totalLen);
    m_lastRawFrame = fullFrame;
    m_buffer.remove(0, totalLen);
    m_stats.mspFramesComplete++;

    const quint8 *payload = reinterpret_cast<const quint8*>(fullFrame.constData()) + headerLen;
    int payloadSize = payloadLen;

    // 5) 可选: 校验 MSP V2 CRC (仅 inbound '>' 有)
    if (hasCrc && payloadSize > 0) {
        quint8 pktCrc = fullFrame.at(totalLen - 1);
        quint8 calcCrc = mspV2Crc8(reinterpret_cast<const quint8*>(fullFrame.constData()) + 3,
                                   headerLen - 3 + payloadSize);
        if (pktCrc != calcCrc) {
            m_stats.malformedDropped++;
            m_stats.lastError = QString("MSP V2 CRC 不匹配: calc=0x%1 pkt=0x%2")
                                    .arg(calcCrc, 2, 16, QChar('0'))
                                    .arg(pktCrc, 2, 16, QChar('0'));
            emit parseError(m_stats.lastError);
            return; // 丢弃该帧，继续处理缓冲区剩余数据
        }
    }

    // 6) 只处理 DISPLAYPORT 命令，其他命令忽略
    if (cmd == MSP_CMD_DISPLAYPORT) {
        parseDisplayportPayload(payload, payloadSize);
        emit mspV2FrameReady(fullFrame);
    } else {
        debugLog(QString("非 DISPLAYPORT 命令 (0x%1)，忽略 %2 bytes")
                     .arg(cmd, 4, 16, QChar('0')).arg(payloadSize));
    }

    // 7) 递归处理剩余缓冲区（可能有多个帧粘连）
    if (!m_buffer.isEmpty())
        processBuffer();
}

// ------------------------------------------------------------------
// 解析 MSP V2 头部
// 格式: $X [dir:1] [flags:1] [cmd:2 LE] [size:2 LE] [payload...] [crc:1]
//   dir = 0x3E('>') inbound 或 0x4D('M') outbound
//   inbound('>') 末尾带 1 byte CRC; outbound('M') 不带
// headerLen = 8 (不含 payload 和 CRC)
// 返回值: 头部解析成功则 true; 头部不完整则 false
// ------------------------------------------------------------------
bool MspRelayParser::tryParseMspV2Header(const quint8 *buf, int avail,
                                          int &headerLen, int &payloadLen,
                                          bool &hasCrc, int &cmd)
{
    // 最低需要前 4 字节确认格式
    if (avail < 4) return false;

    if (buf[0] != MSP_V2_HEADER_MARK || buf[1] != MSP_V2_HEADER_X)
        return false;

    if (avail < MSP_V2_HEADER_SIZE) return false; // 头部不完整

    quint8 dir = buf[2];
    if (dir != MSP_V2_DIR_INBOUND && dir != MSP_V2_DIR_OUTBOUND)
        return false; // 非 MSP V2 方向字节

    headerLen = MSP_V2_HEADER_SIZE;
    // 注意: buf[3] = flags, cmd 从 buf[4] 开始 (LE), size 从 buf[6] 开始 (LE)
    cmd = buf[4] | (buf[5] << 8);       // 小端
    payloadLen = buf[6] | (buf[7] << 8); // 小端
    hasCrc = (dir == MSP_V2_DIR_INBOUND);

    if (payloadLen > 2048) {
        m_stats.malformedDropped++;
        m_stats.lastError = "MSP V2 payload 长度异常";
        emit parseError(m_stats.lastError);
        return false;
    }

    return true;
}

// ------------------------------------------------------------------
// 解析 MSP DISPLAYPORT payload
// payload[0] = 子命令
// ------------------------------------------------------------------
void MspRelayParser::parseDisplayportPayload(const quint8 *data, int len)
{
    if (!data || len <= 0) return;

    MspOsdFrame frame;
    frame.rxTime = QDateTime::currentDateTime();

    quint8 subCmd = data[0];
    frame.subCmd = subCmd;

    int consumed = 0;
    switch (subCmd) {
    case MSP_DISPLAY_DRAW_NORMAL_FULL_PKT: // 0x35 整帧
        if (len >= 3) {
            frame.seq = data[1];
            parseFullPacket(data, len, consumed);
            frame.segments = m_lastSegments;
        } else {
            frame.error = "0x35 帧数据不完整";
        }
        break;

    case MSP_DISPLAY_TEXT_SDK: // 0x05 文本 (SDK 自定义)
        // 格式: [0x05][row][col][attr][chars...]
        if (len >= 5) {
            parseTextCommand(data, len, consumed);
            frame.segments = m_lastSegments;
        } else {
            frame.error = "0x05 文本数据不完整";
        }
        break;

    case MSP_DISPLAY_CLEAR_SCREEN_SDK: // 0x02 清屏 (SDK)
        clearScreen();
        emit osdGridUpdated();
        break;

    case MSP_DISPLAY_DRAW_SCREEN_SDK: // 0x04 刷新屏幕
        emit osdGridUpdated();
        break;

    case MSP_DISPLAY_HEARTBEAT: // 0x00 心跳
        break; // 无动作

    default:
        frame.error = QString("未知子命令 0x%1").arg(subCmd, 2, 16, QChar('0'));
        break;
    }

    if (frame.error.isEmpty() || !frame.segments.isEmpty()) {
        m_stats.displayportParsed++;
        emit osdFrameReady(frame);
    } else if (!frame.error.isEmpty()) {
        m_stats.malformedDropped++;
        emit parseError(frame.error);
    }
}

// ------------------------------------------------------------------
// 解析 0x35 整帧子命令
// 格式: 0x35 [seq] [rsv] [entry...]
//   entry: [len][row][col][attr][text...]  (len = 4 + textLen)
// ------------------------------------------------------------------
void MspRelayParser::parseFullPacket(const quint8 *data, int len, int &consumed)
{
    m_lastSegments.clear();
    consumed = 0;

    if (len < 3) return;
    int pos = 3; // 跳过 0x35 seq rsv

    while (pos + 4 <= len) {
        int entryLen = data[pos];
        if (entryLen < 4) {
            // len < 4 表示畸形条目，停止解析
            break;
        }
        int textLen = entryLen - 4;
        if (pos + entryLen > len) break; // 条目不完整

        int row = data[pos + 1];
        int col = data[pos + 2];
        int attrByte = data[pos + 3];
        quint16 attr = (attrByte & 0x03) << 8; // 低位2bit 映射到 font page

        if (row >= 0 && row < MSP_OSD_ROWS && col >= 0 && col < MSP_OSD_COLS) {
            MspOsdSegment seg;
            seg.row = row;
            seg.col = col;
            seg.attr = attr;
            seg.text = QByteArray(reinterpret_cast<const char*>(data + pos + 4), textLen);
            m_lastSegments.append(seg);

            // 写入字符矩阵
            for (int i = 0; i < textLen && (col + i) < MSP_OSD_COLS; ++i) {
                writeChar(row, col + i, data[pos + 4 + i], attr);
            }
        }

        pos += entryLen;
        consumed = pos;
    }

    if (consumed == 0) consumed = len; // 防止死循环

    // 有实际内容则触发网格更新
    bool hasContent = false;
    for (int r = 0; r < MSP_OSD_ROWS; ++r)
        for (int c = 0; c < MSP_OSD_COLS; ++c)
            if (m_charMap[r][c] != 0) { hasContent = true; break; }
    if (hasContent)
        emit osdGridUpdated();
}

// ------------------------------------------------------------------
// 解析 0x05 文本子命令 (SDK 自定义)
// 格式: [0x05][row][col][attr][chars...]
// ------------------------------------------------------------------
void MspRelayParser::parseTextCommand(const quint8 *data, int len, int &consumed)
{
    m_lastSegments.clear();
    consumed = 0;

    if (len < 5) return;

    int row = data[1];
    int col = data[2];
    int attrByte = data[3];
    quint16 attr = (attrByte & 0x03) << 8;

    // 文本从 pos+4 开始，遇到下一个子命令字节或非 ASCII 即结束
    int textStart = 4;
    int textEnd;
    for (textEnd = textStart; textEnd < len; textEnd++) {
        quint8 b = data[textEnd];
        // 遇到下一个已知子命令或控制字符则停止
        if (b == 0x00 || b == 0x05 || b == 0x02 || b == 0x04 ||
            b == MSP_DISPLAY_DRAW_NORMAL_FULL_PKT || b >= 0x80)
            break;
    }
    int textLen = textEnd - textStart;

    debugLog(QString("0x05 文本: row=%1 col=%2 textLen=%3")
                 .arg(row).arg(col).arg(textLen));

    if (row >= 0 && row < MSP_OSD_ROWS && textLen > 0) {
        MspOsdSegment seg;
        seg.row = row;
        seg.col = col;
        seg.attr = attr;
        seg.text = QByteArray(reinterpret_cast<const char*>(data + textStart), textLen);
        m_lastSegments.append(seg);

        for (int i = 0; i < textLen && (col + i) < MSP_OSD_COLS; ++i) {
            writeChar(row, col + i, data[textStart + i], attr);
        }
    }

    consumed = qMax(textEnd, 5);
    emit osdGridUpdated();
}

// ------------------------------------------------------------------
// 清屏
// ------------------------------------------------------------------
void MspRelayParser::clearScreen()
{
    memset(m_charMap, 0, sizeof(m_charMap));
    m_lastSegments.clear();
}

// ------------------------------------------------------------------
// 清空内部缓冲区
// ------------------------------------------------------------------
void MspRelayParser::clearBuffer()
{
    m_buffer.clear();
    m_lastRawFrame.clear();
}

// ------------------------------------------------------------------
// 写字符到矩阵
// ------------------------------------------------------------------
void MspRelayParser::writeChar(int row, int col, quint16 ch, quint16 attr)
{
    if (row < 0 || row >= MSP_OSD_ROWS) return;
    if (col < 0 || col >= MSP_OSD_COLS) return;
    m_charMap[row][col] = (ch & 0xFF) | attr;
}

// ------------------------------------------------------------------
// 查询矩阵中的字符
// ------------------------------------------------------------------
quint16 MspRelayParser::charAt(int row, int col) const
{
    if (row < 0 || row >= MSP_OSD_ROWS) return 0;
    if (col < 0 || col >= MSP_OSD_COLS) return 0;
    return m_charMap[row][col];
}

// ------------------------------------------------------------------
// MSP V2 CRC (X.25, 多项式 0x1021)
// ------------------------------------------------------------------
quint8 MspRelayParser::mspV2Crc8(const quint8 *data, int len)
{
    // MSP V2 使用 8-bit CRC (X.25 variant)
    // 参考 Betaflight: crc8_dvb_s2 的变体
    quint8 crc = 0x00;
    for (int i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0xD5;  // CRC-8/DVB-S2
            else
                crc <<= 1;
        }
    }
    return crc;
}

// ------------------------------------------------------------------
// 统计重置
// ------------------------------------------------------------------
void MspRelayParser::resetStats()
{
    m_stats = MspRelayStats();
}

// ------------------------------------------------------------------
// 调试日志
// ------------------------------------------------------------------
void MspRelayParser::debugLog(const QString &msg) const
{
    if (m_debug) {
        QByteArray ba = msg.toUtf8();
        fprintf(stderr, "[MspRelayParser] %s\n", ba.constData());
        fflush(stderr);
    }
}
