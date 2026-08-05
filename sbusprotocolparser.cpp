#include "sbusprotocolparser.h"
#include <QtGlobal>

// ============================================================
// SBUS Protocol Parser implementation
// ============================================================

ParsedSbusFrame SbusProtocolParser::parseFrame(const QByteArray &frame)
{
    ParsedSbusFrame result;
    if (frame.size() < SBUS_FRAME_SIZE) {
        result.valid = false;
        return result;
    }

    const quint8 *raw = reinterpret_cast<const quint8*>(frame.constData());

    // Determine polarity from the start byte
    quint8 start = raw[0];
    quint8 end   = raw[SBUS_FRAME_SIZE - 1];

    bool inverted = false;
    if (start == SBUS_START_BYTE && end == SBUS_END_BYTE) {
        inverted = false;                       // normal polarity
    } else if (start == SBUS_START_BYTE_INV && end == SBUS_END_BYTE_INV) {
        inverted = true;                        // inverted capture (0xF0..0xFF)
    } else {
        // Not a valid SBUS frame bounding
        result.valid = false;
        return result;
    }

    result.valid = true;
    result.inverted = inverted;
    result.startByte = start;
    result.endByte = end;
    result.flagsByte = raw[23];
    result.payloadLength = 22;

    // If the frame was captured byte-inverted, invert payload bytes back
    // so channel values are recovered correctly. (In the common real case
    // the signal is inverted only electrically and the UART already delivers
    // normal bytes, so inverted=false.)
    QByteArray workPayload;
    if (inverted) {
        workPayload.resize(22);
        for (int i = 0; i < 22; ++i)
            workPayload[i] = static_cast<char>(~raw[1 + i]);
        result.flagsByte = static_cast<quint8>(~raw[23]);
    } else {
        workPayload = QByteArray::fromRawData(reinterpret_cast<const char*>(raw + 1), 22);
    }

    // Unpack 16 channels
    unpackChannels(reinterpret_cast<const quint8*>(workPayload.constData()), result.channels);

    // Convert to PWM
    for (int i = 0; i < SBUS_NUM_CHANNELS; ++i)
        result.channelsPwm[i] = channelToPwm(result.channels[i]);

    // Flag byte bits (use the polarity-corrected flagsByte)
    quint8 flags = result.flagsByte;
    result.ch17      = (flags & 0x01) != 0;
    result.ch18      = (flags & 0x02) != 0;
    result.frameLost = (flags & 0x04) != 0;
    result.failsafe  = (flags & 0x08) != 0;

    result.hexDump = QString::fromLatin1(frame.toHex(' ').toUpper());

    return result;
}

// ============================================================
// 16 x 11-bit channels from 22-byte little-endian bitstream
// ============================================================
void SbusProtocolParser::unpackChannels(const quint8 *payload, uint16_t out[SBUS_NUM_CHANNELS])
{
    // The 176 bits (16 * 11) are packed continuously, LSB first.
    // Bit index i (0-based) of the stream = byte i/8, bit i%8.
    for (int ch = 0; ch < SBUS_NUM_CHANNELS; ++ch) {
        uint16_t value = 0;
        int bitOffset = ch * SBUS_CHANNEL_BITS;
        for (int b = 0; b < SBUS_CHANNEL_BITS; ++b) {
            int bitIndex = bitOffset + b;
            int byteIdx  = bitIndex >> 3;        // /8
            int bitInByte = bitIndex & 0x07;     // %8
            bool bit = (payload[byteIdx] >> bitInByte) & 0x01;
            value |= (bit << b);
        }
        out[ch] = value & 0x07FF;
    }
}

uint16_t SbusProtocolParser::channelToPwm(uint16_t sbusValue)
{
    // Map SBUS range [0..2047] onto PWM [988..2012]
    if (sbusValue >= SBUS_CH_MAX)
        return SBUS_PWM_MAX;
    // pwm = pwmMin + (sbusValue / 2047) * (pwmMax - pwmMin)
    quint64 pwm = SBUS_PWM_MIN
                + (static_cast<quint64>(sbusValue) * (SBUS_PWM_MAX - SBUS_PWM_MIN) / SBUS_CH_MAX);
    return static_cast<uint16_t>(pwm);
}

uint16_t SbusProtocolParser::pwmToChannel(uint16_t pwmUs)
{
    if (pwmUs <= SBUS_PWM_MIN) return SBUS_CH_MIN;
    if (pwmUs >= SBUS_PWM_MAX) return SBUS_CH_MAX;
    quint64 v = (static_cast<quint64>(pwmUs - SBUS_PWM_MIN) * SBUS_CH_MAX)
              / (SBUS_PWM_MAX - SBUS_PWM_MIN);
    return static_cast<uint16_t>(v);
}

// ============================================================
// Build a test frame (normal polarity, non-inverted)
// ============================================================
QByteArray SbusProtocolParser::buildFrame(const uint16_t channels[SBUS_NUM_CHANNELS],
                                          bool ch17, bool ch18,
                                          bool frameLost, bool failsafe)
{
    QByteArray frame;
    frame.resize(SBUS_FRAME_SIZE);
    quint8 *raw = reinterpret_cast<quint8*>(frame.data());

    raw[0] = static_cast<quint8>(SBUS_START_BYTE);

    // Pack 16 channels into bytes 1..22 (LSB-first continuous bitstream)
    {
        for (int i = 1; i <= 22; ++i) raw[i] = 0;
        int bitOffset = 0;
        for (int ch = 0; ch < SBUS_NUM_CHANNELS; ++ch) {
            uint16_t v = channels[ch] & 0x07FF;
            for (int b = 0; b < SBUS_CHANNEL_BITS; ++b) {
                int bitIndex = bitOffset + b;
                int byteIdx  = 1 + (bitIndex >> 3);
                int bitInByte = bitIndex & 0x07;
                if (byteIdx <= 22) {
                    if ((v >> b) & 0x01)
                        raw[byteIdx] |= static_cast<quint8>(1 << bitInByte);
                }
            }
            bitOffset += SBUS_CHANNEL_BITS;
        }
    }

    // Flags byte
    quint8 flags = 0;
    if (ch17)      flags |= 0x01;
    if (ch18)      flags |= 0x02;
    if (frameLost) flags |= 0x04;
    if (failsafe)  flags |= 0x08;
    raw[23] = flags;

    // End byte
    raw[24] = static_cast<quint8>(SBUS_END_BYTE);

    return frame;
}

bool SbusProtocolParser::isStartByte(quint8 byte)
{
    return byte == SBUS_START_BYTE || byte == SBUS_START_BYTE_INV;
}

bool SbusProtocolParser::isEndByte(quint8 byte)
{
    return byte == SBUS_END_BYTE || byte == SBUS_END_BYTE_INV;
}
