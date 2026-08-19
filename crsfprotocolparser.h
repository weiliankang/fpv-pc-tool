#ifndef CRSFPROTOCOLPARSER_H
#define CRSFPROTOCOLPARSER_H

#include <QByteArray>
#include <QString>
#include <QVector>
#include <QMap>
#include <QDateTime>
#include <QElapsedTimer>
#include <QPair>

// ============================================================
// CRSF Protocol Parser
// Based on Betaflight's CRSF implementation
// Fully decodes all packet types defined in crsf_protocol.h
// ============================================================

// Maximum number of RC channels
static constexpr int CRSF_MAX_CHANNELS = 16;

// CRSF sync byte
static constexpr quint8 CRSF_SYNC_BYTE = 0xC8;

// CRSF address constants
static constexpr quint8 CRSF_ADDRESS_BROADCAST      = 0x00;
static constexpr quint8 CRSF_ADDRESS_USB             = 0x10;
static constexpr quint8 CRSF_ADDRESS_TBS_CORE_PNP    = 0x1B;
static constexpr quint8 CRSF_ADDRESS_RESERVED1       = 0x1C;
static constexpr quint8 CRSF_ADDRESS_SERIAL_RX       = 0x1C;  // iNav
static constexpr quint8 CRSF_ADDRESS_SERIAL_TX       = 0x1D;  // iNav
static constexpr quint8 CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8;
static constexpr quint8 CRSF_ADDRESS_RADIO_TX        = 0xEA;
static constexpr quint8 CRSF_ADDRESS_CRSF_RECEIVER   = 0xEC;
static constexpr quint8 CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE;

// Frame type constants (from Betaflight crsf_protocol.h)
enum CrsfFrameType : quint8 {
    CRSF_FRAMETYPE_GPS                      = 0x02,
    CRSF_FRAMETYPE_GPS_TIME                 = 0x03,
    CRSF_FRAMETYPE_VARIO                    = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR           = 0x08,
    CRSF_FRAMETYPE_BARO_ALT                 = 0x09,
    CRSF_FRAMETYPE_HEARTBEAT                = 0x0B,
    CRSF_FRAMETYPE_ACC_GYRO                 = 0x13,
    CRSF_FRAMETYPE_LINK_STATISTICS          = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED       = 0x16,
    CRSF_FRAMETYPE_SUBSET_RC_CHANNELS       = 0x17,
    CRSF_FRAMETYPE_LINK_STATISTICS_RX       = 0x1C,
    CRSF_FRAMETYPE_LINK_STATISTICS_TX       = 0x1D,
    CRSF_FRAMETYPE_ATTITUDE                 = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE              = 0x21,
    // 0x22-0x26: various types
    CRSF_FRAMETYPE_DEVICE_PING              = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO              = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ           = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE          = 0x2D,
    CRSF_FRAMETYPE_COMMAND                  = 0x32,
    CRSF_FRAMETYPE_MSP_REQ                  = 0x7A,
    CRSF_FRAMETYPE_MSP_RESP                 = 0x7B,
    CRSF_FRAMETYPE_MSP_WRITE                = 0x7C,
    CRSF_FRAMETYPE_DISPLAYPORT_CMD          = 0x7D,
};

// Parsed payload for RC channels
struct CrsfRcChannels {
    uint16_t channels[CRSF_MAX_CHANNELS] = {0};
};

// Parsed payload for GPS
struct CrsfGps {
    double latitude = 0.0;
    double longitude = 0.0;
    uint16_t groundspeed = 0;   // cm/s
    uint16_t heading = 0;       // degrees * 10
    uint16_t altitude = 0;      // cm
    uint8_t satellites = 0;
};

// Parsed payload for GPS time
struct CrsfGpsTime {
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    uint8_t millis = 0;
};

// Parsed payload for Vario
struct CrsfVario {
    int16_t verticalSpeed = 0;  // cm/s
};

// Parsed payload for Battery sensor
struct CrsfBattery {
    uint16_t voltage = 0;     // mV * 100 (0.01V units)
    uint16_t current = 0;     // mA * 100 (0.01A units)
    uint32_t capacity = 0;    // mAh
    uint8_t remaining = 0;    // percent
};

// Parsed payload for Baro altitude
struct CrsfBaroAlt {
    uint16_t altitude = 0;    // cm
    uint16_t vspeed = 0;      // cm/s
};

// Parsed payload for ACC/GYRO
struct CrsfAccGyro {
    int16_t accX = 0, accY = 0, accZ = 0;
    int16_t gyroX = 0, gyroY = 0, gyroZ = 0;
};

// Parsed payload for Link Statistics (0x14)
struct CrsfLinkStatistics {
    int8_t uplinkRSSI1 = 0;      // dBm * -1 (stored as negative dBm)
    int8_t uplinkRSSI2 = 0;      // dBm * -1
    uint8_t uplinkLinkQuality = 0;  // percent
    int8_t uplinkSNR = 0;        // dB
    uint8_t activeAntenna = 0;   // 0=ant1, 1=ant2
    uint8_t rfMode = 0;          // 0=4fps, 1=50fps, 2=150fps
    uint8_t uplinkTxPower = 0;   // see CRSF_TX_POWER enums
    int8_t downlinkRSSI = 0;     // dBm * -1
    uint8_t downlinkLinkQuality = 0;  // percent
    int8_t downlinkSNR = 0;      // dB
};

// Parsed payload for Link Statistics RX (0x1C)
struct CrsfLinkStatisticsRx {
    int8_t uplinkRSSI1 = 0;
    int8_t uplinkRSSI2 = 0;
    uint8_t uplinkLinkQuality = 0;
    int8_t uplinkSNR = 0;
    uint8_t activeAntenna = 0;
    uint8_t rfMode = 0;
    uint8_t uplinkTxPower = 0;
};

// Parsed payload for Link Statistics TX (0x1D)
struct CrsfLinkStatisticsTx {
    int8_t downlinkRSSI = 0;
    uint8_t downlinkLinkQuality = 0;
    int8_t downlinkSNR = 0;
};

// Parsed payload for Attitude
struct CrsfAttitude {
    int16_t pitch = 0;   // degrees * 100
    int16_t roll = 0;    // degrees * 100
    int16_t yaw = 0;     // degrees * 100
};

// Parsed payload for Subset RC channels
struct CrsfSubsetRcChannels {
    uint8_t startChannel = 0;
    uint8_t bitsPerChannel = 11;
    QVector<uint16_t> channels; // variable length, up to 16
};

// Category for colored display
enum class CrsfPacketCategory {
    Unknown,
    RcChannels,     // Green
    LinkStats,      // Blue
    Sensor,         // Orange
    System,         // Purple
    Heartbeat,      // Gray
    Command,        // Yellow
    Msp,            // Cyan
    DisplayPort,    // Magenta
};

// ============================================================
// Main parsed frame structure
// ============================================================
struct ParsedCrsfFrame {
    // Frame header
    quint8 syncByte = 0;
    quint8 frameLength = 0;      // total bytes including CRC (len field value - includes addr+type+payload+crc)
    quint8 deviceAddress = 0;    // source/dest address (first byte after length)
    quint8 frameType = 0;        // the frame type byte
    quint8 crcReceived = 0;
    quint8 crcCalculated = 0;
    bool crcValid = false;

    // Parsed fields
    QString frameTypeName;
    QString sourceAddressName;
    QString destAddressName;
    int payloadLength = 0;
    CrsfPacketCategory category = CrsfPacketCategory::Unknown;

    // Union-like storage for decoded payloads
    CrsfRcChannels rcChannels;
    CrsfLinkStatistics linkStats;
    CrsfGps gps;
    CrsfGpsTime gpsTime;
    CrsfVario vario;
    CrsfBattery battery;
    CrsfBaroAlt baroAlt;
    CrsfAccGyro accGyro;
    CrsfAttitude attitude;
    QByteArray rawPayload;      // The raw payload bytes (for unknown types)
    QByteArray fullFrameBytes;  // Complete raw frame bytes (for hex display)

    // Payload summary string (human readable)
    QString payloadSummary;

    // Full hex dump of the frame
    QString hexDump;

    // Whether we could decode the payload meaningfully
    bool payloadDecoded = false;
};

// ============================================================
// Interval analysis struct
// ============================================================
struct CrsfIntervalAnalysis {
    qint64 currentIntervalMs = 0;   // time since previous packet (ms)
    qint64 minIntervalMs = 0;
    qint64 maxIntervalMs = 0;
    qint64 avgIntervalMs = 0;
    int sampleCount = 0;
    // Detected pattern, if any (e.g. "6.7ms ≈ 150Hz")
    QString detectedPattern;
};

// ============================================================
// Parser class
// ============================================================
class CrsfProtocolParser
{
public:
    CrsfProtocolParser();

    // Parse a complete CRSF frame (starting with sync byte)
    // Data must include [Sync=0xC8][Len][Addr][Type][Payload...][CRC]
    ParsedCrsfFrame parseFrame(const QByteArray &fullFrame, bool validateCrc = true) const;

    // Convenience: parse just the payload bytes given a type
    // This assumes the payload has already been stripped of the header/CRC
    void parsePayload(ParsedCrsfFrame &frame) const;

    // CRC calculation helper
    static quint8 crc8(const quint8 *data, int len, quint8 poly = 0xD5, quint8 init = 0x00);

    // Validate CRC for a complete frame (sync+len+addr+type+payload+crc)
    static bool validateFrameCrc(const quint8 *frameStart, int frameLen);

    // Convert CRSF channel value to PWM microseconds (us)
    // CRSF range: 172-1811 → PWM: 988-2012 us
    static uint16_t channelValueToPwm(uint16_t crsfValue);

    // Convert PWM microseconds back to CRSF value
    static uint16_t pwmToChannelValue(uint16_t pwmUs);

    // Parse packed RC channels from a 22-byte payload
    // Returns 16 channel values in CRSF range [172, 1811]
    static CrsfRcChannels parseRcChannels(const quint8 *payload, int payloadLen);

    // Get the expected payload length for a given frame type (excluding type byte, including CRC byte)
    // Returns -1 if variable/unknown
    static int expectedPayloadLength(quint8 frameType);

    // Get a human-readable name for a frame type
    static QString frameTypeName(quint8 frameType);

    // Get a human-readable name for an address
    static QString addressName(quint8 address);

    // Get category for a frame type (for colored display)
    static CrsfPacketCategory frameCategory(quint8 frameType);

    // Get RF mode name
    static QString rfModeName(uint8_t rfMode);

    // Get TX power name
    static QString txPowerName(uint8_t powerIdx);

    // Interval analysis helper
    CrsfIntervalAnalysis analyzeInterval(qint64 newIntervalMs);

private:
    // Internal: parse RC channels subset
    static CrsfSubsetRcChannels parseSubsetRcChannels(const quint8 *payload, int payloadLen);

    // Running interval statistics
    qint64 m_intervalSum = 0;
    qint64 m_intervalMin = 999999;
    qint64 m_intervalMax = 0;
    int m_intervalCount = 0;

    // Static maps
    static const QMap<quint8, QString> &typeNameMap();
    static const QMap<quint8, int> &typePayloadLengthMap();
};

#endif // CRSFPROTOCOLPARSER_H
