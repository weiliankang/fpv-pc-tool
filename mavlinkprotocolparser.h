#ifndef MAVLINKPROTOCOLPARSER_H
#define MAVLINKPROTOCOLPARSER_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>

// ============================================================
// MAVLink Protocol Parser
// Supports MAVLink v1 and v2 framing, CRC16/MCRF4XX (X.25) checksum,
// and decoding of common messages.
//
// v1 frame:
//   [0] 0xFE [1]len [2]seq [3]sysid [4]compid [5]msgid [6..6+len-1]payload [..]crc_lo crc_hi
// v2 frame:
//   [0] 0xFD [1]len [2]incompat [3]compat [4]seq [5]sysid [6]compid [7..8]msgid(LE)
//   [9..9+len-1]payload [..]crc_lo crc_hi [..]signature(13 if incompat&1)
// ============================================================

// Frame markers
static constexpr quint8 MAVLINK_STX_V1 = 0xFE;
static constexpr quint8 MAVLINK_STX_V2 = 0xFD;
static constexpr int MAVLINK_MAX_PAYLOAD = 255;
static constexpr int MAVLINK_SIGNATURE_LEN = 13;

// Parsed MAVLink frame (common header fields)
struct ParsedMavlinkFrame {
    bool valid = false;
    bool crcValid = false;
    int version = 0;                 // 1 or 2
    quint8 stx = 0;
    quint8 payloadLen = 0;
    quint8 incompatFlags = 0;
    quint8 compatFlags = 0;
    quint8 seq = 0;
    quint8 sysid = 0;
    quint8 compid = 0;
    uint32_t msgid = 0;
    QByteArray payload;
    quint16 crcReceived = 0;
    quint16 crcCalculated = 0;
    bool hasSignature = false;
    QByteArray signature;

    QString msgName;                 // human readable message name
    QString summary;                 // one-line human-readable content
    QString hexDump;                 // full raw frame hex
    int totalLength = 0;
};

class MavlinkProtocolParser
{
public:
    MavlinkProtocolParser() = default;

    // Parse a single complete MAVLink frame (starting at stx).
    // Returns a ParsedMavlinkFrame; valid=false if framing is bad.
    static ParsedMavlinkFrame parseFrame(const QByteArray &frame);

    // CRC16 (X.25 / MCRF4XX) used by MAVLink.
    static quint16 crc16(const quint8 *data, int len, quint16 crc = 0xFFFF);

    // Decode payload into a summary string for a known msgid.
    static void decodeMessage(ParsedMavlinkFrame &frame);

    // Name of a message id (e.g. 0 -> "HEARTBEAT").
    static QString messageName(uint32_t msgid);

    // Whether this byte is a MAVLink start byte.
    static bool isStartByte(quint8 byte) {
        return byte == MAVLINK_STX_V1 || byte == MAVLINK_STX_V2;
    }

    // Minimum header length for a version (after stx known).
    static int headerLength(quint8 stx) {
        return (stx == MAVLINK_STX_V2) ? 10 : 6;
    }

private:
    // Read little-endian scalar helpers on payload
    static double   leDouble(const quint8 *p, int off);
    static float    leFloat(const quint8 *p, int off);
    static quint64  leU64(const quint8 *p, int off);
    static qint32   leI32(const quint8 *p, int off);
    static quint32  leU32(const quint8 *p, int off);
    static qint16   leI16(const quint8 *p, int off);
    static quint16  leU16(const quint8 *p, int off);
    static qint8    leI8(const quint8 *p, int off);

    static void decodeHeartbeat(ParsedMavlinkFrame &f);
    static void decodeSysStatus(ParsedMavlinkFrame &f);
    static void decodeGpsRawInt(ParsedMavlinkFrame &f);
    static void decodeAttitude(ParsedMavlinkFrame &f);
    static void decodeBatteryStatus(ParsedMavlinkFrame &f);
    static void decodeVfrHud(ParsedMavlinkFrame &f);
    static void decodeGlobalPositionInt(ParsedMavlinkFrame &f);
    static void decodeRcChannels(ParsedMavlinkFrame &f);
    static void decodeServoOutputRaw(ParsedMavlinkFrame &f);
};

#endif // MAVLINKPROTOCOLPARSER_H
