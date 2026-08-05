#ifndef SBUSPROTOCOLPARSER_H
#define SBUSPROTOCOLPARSER_H

#include <QByteArray>
#include <QString>
#include <QVector>

// ============================================================
// SBUS Protocol Parser
// Futaba SBUS serial bus protocol.
//
// Frame format (25 bytes, 100000 baud, 8E2, INVERTED signal):
//   [0]    0x0F          Start byte (sync)
//   [1..22] 22 bytes     16 channels x 11 bits (little-endian bit packing)
//   [23]   Flags byte    bit0=ch17, bit1=ch18, bit2=frame lost, bit3=failsafe
//   [24]   0x00          End byte
//
// Note: SBUS is transmitted inverted. When captured on a non-inverting
// UART you will see 0xF0 as the start byte instead of 0x0F. The parser
// auto-detects both polarities.
// ============================================================

// SBUS constants
static constexpr int SBUS_FRAME_SIZE        = 25;
static constexpr int SBUS_NUM_CHANNELS      = 16;
static constexpr int SBUS_CHANNEL_BITS      = 11;
static constexpr unsigned short SBUS_START_BYTE       = 0x0F;
static constexpr unsigned short SBUS_END_BYTE         = 0x00;
// Inverted forms (when captured non-inverted)
static constexpr unsigned short SBUS_START_BYTE_INV   = 0xF0;
static constexpr unsigned short SBUS_END_BYTE_INV     = 0xFF;

// Channel value ranges
static constexpr uint16_t SBUS_CH_MIN    = 0;      // -100%
static constexpr uint16_t SBUS_CH_MID    = 992;    // center (neutral)
static constexpr uint16_t SBUS_CH_MAX    = 2047;   // +100%
// Convert to standard servo PWM range
static constexpr uint16_t SBUS_PWM_MIN   = 988;
static constexpr uint16_t SBUS_PWM_MID   = 1500;
static constexpr uint16_t SBUS_PWM_MAX   = 2012;

// Parsed SBUS frame
struct ParsedSbusFrame {
    bool valid = false;              // start/end bytes matched and structure OK
    bool inverted = false;           // true if frame was captured inverted (0xF0 start)
    quint8 startByte = 0;
    quint8 endByte = 0;
    quint8 flagsByte = 0;

    // 16 channel values in raw SBUS range [0..2047]
    uint16_t channels[SBUS_NUM_CHANNELS] = {0};
    // 16 channel values converted to PWM microseconds [988..2012]
    uint16_t channelsPwm[SBUS_NUM_CHANNELS] = {0};

    // Flag byte bits
    bool ch17 = false;               // channel 17 (digital)
    bool ch18 = false;               // channel 18 (digital)
    bool frameLost = false;          // signal lost
    bool failsafe = false;           // failsafe active

    // Analysis
    QString hexDump;                 // full 25-byte hex
    int payloadLength = 0;
};

class SbusProtocolParser
{
public:
    SbusProtocolParser() = default;

    // Parse a complete 25-byte SBUS frame.
    // Auto-detects inverted vs normal polarity.
    // Returns a ParsedSbusFrame with valid=false if structure is wrong.
    static ParsedSbusFrame parseFrame(const QByteArray &frame);

    // Raw channel value -> PWM microseconds (maps 0..2047 -> 988..2012)
    static uint16_t channelToPwm(uint16_t sbusValue);

    // PWM microseconds -> raw SBUS value
    static uint16_t pwmToChannel(uint16_t pwmUs);

    // Convenience: build a test frame from 16 channel values (raw SBUS range)
    static QByteArray buildFrame(const uint16_t channels[SBUS_NUM_CHANNELS],
                                 bool ch17 = false, bool ch18 = false,
                                 bool frameLost = false, bool failsafe = false);

    // Whether the given byte looks like a valid start byte (0x0F or 0xF0)
    static bool isStartByte(quint8 byte);

    // Whether the given byte looks like a valid end byte (0x00 or 0xFF)
    static bool isEndByte(quint8 byte);

    // Channel count supported by this parser
    static int channelCount() { return SBUS_NUM_CHANNELS; }

private:
    // Internal: unpack 16 x 11bit channels from a 22-byte payload (little-endian)
    static void unpackChannels(const quint8 *payload, uint16_t out[SBUS_NUM_CHANNELS]);
};

#endif // SBUSPROTOCOLPARSER_H
