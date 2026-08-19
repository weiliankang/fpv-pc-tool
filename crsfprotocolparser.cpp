#include "crsfprotocolparser.h"
#include <QtEndian>
#include <QDebug>

// ============================================================
// CRC-8-DVB-S2: poly 0xD5, init 0x00
// Used by standard CRSF frames (0x14, 0x16, etc.)
// ============================================================
quint8 CrsfProtocolParser::crc8(const quint8 *data, int len, quint8 poly, quint8 init)
{
    quint8 crc = init;
    for (int i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x80)
                crc = (crc << 1) ^ poly;
            else
                crc <<= 1;
        }
    }
    return crc;
}

bool CrsfProtocolParser::validateFrameCrc(const quint8 *frameStart, int frameLen)
{
    // Frame structure: [Sync][Len][Type][Addr][Payload...][CRC]  (ELRS style)
    if (frameLen < 4) return false;
    if (frameStart[0] != 0xC8) return false;

    // CRC covers: Type + Addr + Payload (offset 2 to CRC byte, length = frameLen - 3)
    quint8 calcCrc = crc8(frameStart + 2, frameLen - 3, 0xD5, 0x00);
    return calcCrc == frameStart[frameLen - 1];
}

uint16_t CrsfProtocolParser::channelValueToPwm(uint16_t crsfValue)
{
    // CRSF range [172, 1811] → PWM [988, 2012] us
    // Linear: PWM = CRSF * 0.624771 + 881
    // To avoid floating point: use fixed-point
    // 0.624771 ≈ 10243 / 16384 (very close)
    return static_cast<uint16_t>((crsfValue * 10243) / 16384 + 881);
}

uint16_t CrsfProtocolParser::pwmToChannelValue(uint16_t pwmUs)
{
    // Inverse: CRSF = (PWM - 881) / 0.624771
    return static_cast<uint16_t>(((pwmUs - 881) * 16384 + 5120) / 10243);
}

CrsfRcChannels CrsfProtocolParser::parseRcChannels(const quint8 *payload, int payloadLen)
{
    CrsfRcChannels result;
    if (payloadLen < 22) return result;

    // 16 channels × 11 bits = 176 bits = 22 bytes
    // Layout (big endian byte stream, MSB first):
    // Byte 0: chan0[10:3], chan0[2:0] in lower 3 bits
    // Actually CRSF uses big-endian packing:
    //   chan0 occupies bits 10:0 of the 11-bit field
    //   The 22 bytes form a continuous bit stream, 11 bits per channel

    // Use a 64-bit accumulator to extract 11-bit values
    uint64_t buf = 0;
    int bitsInBuf = 0;
    int byteIdx = 0;

    for (int ch = 0; ch < 16 && byteIdx < payloadLen; ++ch) {
        while (bitsInBuf < 11 && byteIdx < payloadLen) {
            buf = (buf << 8) | payload[byteIdx++];
            bitsInBuf += 8;
        }
        // Extract 11 bits from the top of the buffer
        result.channels[ch] = static_cast<uint16_t>((buf >> (bitsInBuf - 11)) & 0x7FF);
        bitsInBuf -= 11;
        // Mask to keep only remaining bits
        buf &= (1ULL << bitsInBuf) - 1;
    }

    return result;
}

CrsfSubsetRcChannels CrsfProtocolParser::parseSubsetRcChannels(const quint8 *payload, int payloadLen)
{
    CrsfSubsetRcChannels result;
    if (payloadLen < 1) return result;

    // Config byte: bits 0-4 = start_channel, bits 5-6 = resolution, bit 7 = reserved
    uint8_t configByte = payload[0];
    result.startChannel = configByte & 0x1F;

    uint8_t resBits = (configByte >> 5) & 0x03;
    switch (resBits) {
    case 0: result.bitsPerChannel = 10; break;
    case 1: result.bitsPerChannel = 11; break;
    case 2: result.bitsPerChannel = 12; break;
    case 3: result.bitsPerChannel = 13; break;
    }

    // Remaining bytes after config byte contain packed channels
    int dataBytes = payloadLen - 1;
    if (dataBytes <= 0) return result;

    int numChannels = (dataBytes * 8) / result.bitsPerChannel;
    if (numChannels > 16) numChannels = 16;
    if (result.startChannel + numChannels > 16) numChannels = 16 - result.startChannel;

    result.channels.resize(numChannels);

    // Bit unpacking (same as Betaflight: LSB-first bit packing)
    uint64_t readValue = 0;
    int bitsMerged = 0;
    int readIdx = 1; // skip config byte

    for (int n = 0; n < numChannels; n++) {
        while (bitsMerged < result.bitsPerChannel && readIdx < payloadLen) {
            readValue |= static_cast<uint64_t>(payload[readIdx++]) << bitsMerged;
            bitsMerged += 8;
        }
        uint16_t mask = (1 << result.bitsPerChannel) - 1;
        result.channels[n] = readValue & mask;
        readValue >>= result.bitsPerChannel;
        bitsMerged -= result.bitsPerChannel;
    }

    return result;
}

// ============================================================
// Static maps
// ============================================================
const QMap<quint8, QString> &CrsfProtocolParser::typeNameMap()
{
    static const QMap<quint8, QString> map = {
        {0x02, "GPS"},
        {0x03, "GPS Time"},
        {0x06, "GPS Extended"},
        {0x07, "Vario"},
        {0x08, "Battery"},
        {0x09, "Baro Alt"},
        {0x0B, "Heartbeat"},
        {0x11, "Baro"},
        {0x12, "Mag"},
        {0x13, "ACC/GYRO"},
        {0x14, "Link Stats"},
        {0x16, "RC Channels"},
        {0x17, "Subset RC"},
        {0x1C, "Link Stats RX"},
        {0x1D, "Link Stats TX"},
        {0x1E, "Attitude"},
        {0x21, "Flight Mode"},
        {0x28, "Device Ping"},
        {0x29, "Device Info"},
        {0x2B, "Param Entry"},
        {0x2C, "Param Read"},
        {0x2D, "Param Write"},
        {0x32, "Command"},
        {0x7A, "MSP Req"},
        {0x7B, "MSP Resp"},
        {0x7C, "MSP Write"},
        {0x7D, "DisplayPort"},
    };
    return map;
}

const QMap<quint8, int> &CrsfProtocolParser::typePayloadLengthMap()
{
    static const QMap<quint8, int> map = {
        {0x02, 15},   // GPS
        {0x03, 9},    // GPS Time
        {0x06, 20},   // GPS Extended
        {0x07, 2},    // Vario
        {0x08, 8},    // Battery
        {0x09, 3},    // Baro Alt
        {0x0B, 2},    // Heartbeat
        {0x11, 8},    // Baro
        {0x12, 6},    // Mag
        {0x13, 18},   // ACC/GYRO
        {0x14, 10},   // Link Stats
        {0x16, 22},   // RC Channels
        {0x1C, 5},    // Link Stats RX
        {0x1D, 6},    // Link Stats TX
        {0x1E, 6},    // Attitude
        {0x21, 0},    // Flight Mode (dynamic)
        {0x28, 2},    // Device Ping
        {0x29, 13},   // Device Info (typical)
        {0x2B, 0},    // Param Entry (dynamic)
        {0x2C, 0},    // Param Read (dynamic)
        {0x2D, 0},    // Param Write (dynamic)
        {0x32, 0},    // Command (dynamic)
        {0x7A, 0},    // MSP Req (dynamic)
        {0x7B, 0},    // MSP Resp (dynamic)
        {0x7C, 0},    // MSP Write (dynamic)
        {0x7D, 0},    // DisplayPort (dynamic)
    };
    return map;
}

int CrsfProtocolParser::expectedPayloadLength(quint8 frameType)
{
    auto &map = typePayloadLengthMap();
    auto it = map.find(frameType);
    if (it != map.end() && it.value() > 0)
        return it.value();
    return -1; // variable
}

QString CrsfProtocolParser::frameTypeName(quint8 frameType)
{
    auto &map = typeNameMap();
    auto it = map.find(frameType);
    if (it != map.end())
        return it.value();
    return QString("0x%1").arg(frameType, 2, 16, QChar('0'));
}

QString CrsfProtocolParser::addressName(quint8 addr)
{
    switch (addr) {
    case 0x00: return "BCast";
    case 0x10: return "USB";
    case 0x1B: return "CorePNP";
    case 0x1C:
    case 0xC8: return "FC";
    case 0x1D: return "TX";  // iNav TX
    case 0xC0: return "CurSen";
    case 0xC2: return "GPS";
    case 0xEA: return "Radio";
    case 0xEC: return "RX";
    case 0xEE: return "TX";
    default:   return QString("0x%1").arg(addr, 2, 16, QChar('0'));
    }
}

CrsfPacketCategory CrsfProtocolParser::frameCategory(quint8 frameType)
{
    switch (frameType) {
    case 0x16:
    case 0x17:
        return CrsfPacketCategory::RcChannels;
    case 0x14:
    case 0x1C:
    case 0x1D:
        return CrsfPacketCategory::LinkStats;
    case 0x02:
    case 0x03:
    case 0x06:
    case 0x07:
    case 0x08:
    case 0x09:
    case 0x11:
    case 0x12:
    case 0x13:
    case 0x1E:
        return CrsfPacketCategory::Sensor;
    case 0x0B:
        return CrsfPacketCategory::Heartbeat;
    case 0x28:
    case 0x29:
    case 0x2B:
    case 0x2C:
    case 0x2D:
    case 0x32:
        return CrsfPacketCategory::Command;
    case 0x7A:
    case 0x7B:
    case 0x7C:
        return CrsfPacketCategory::Msp;
    case 0x7D:
        return CrsfPacketCategory::DisplayPort;
    default:
        return CrsfPacketCategory::Unknown;
    }
}

QString CrsfProtocolParser::rfModeName(uint8_t rfMode)
{
    switch (rfMode) {
    case 0: return "4fps";
    case 1: return "50fps";
    case 2: return "150fps";
    default: return QString("%1fps").arg(rfMode);
    }
}

QString CrsfProtocolParser::txPowerName(uint8_t powerIdx)
{
    static const char *names[] = {
        "0mW", "10mW", "25mW", "100mW", "500mW",
        "1000mW", "2000mW", "250mW"
    };
    if (powerIdx < 8)
        return names[powerIdx];
    return QString("Pwr%1").arg(powerIdx);
}

// ============================================================
// Main parse function
// ============================================================
ParsedCrsfFrame CrsfProtocolParser::parseFrame(const QByteArray &fullFrame, bool validateCrc) const
{
    ParsedCrsfFrame result;
    if (fullFrame.size() < 4) return result;

    const quint8 *data = reinterpret_cast<const quint8*>(fullFrame.constData());

    result.syncByte = data[0];
    result.frameLength = data[1];    // len = type + payload + crc (ELRS style)
    result.frameType = data[2];      // ELRS: [sync][len][type][addr][payload][crc]
    result.deviceAddress = data[3];  // addr 紧随 type 之后
    result.fullFrameBytes = fullFrame;

    // Build hex dump
    QStringList hexParts;
    for (int i = 0; i < fullFrame.size(); ++i)
        hexParts << QString("%1").arg(data[i], 2, 16, QChar('0'));
    result.hexDump = hexParts.join(" ");

    // Payload = everything after addr(offset 4) up to CRC(last byte)
    // frame = [sync][len][type][addr][payload...][crc]
    // payload_size = len_field - 2 (type+payload = len-1, minus type = len-2)
    //             = fullFrame.size() - 4
    //             = len_field + 2 - 4 = len_field - 2 ✓
    result.payloadLength = fullFrame.size() - 4;
    if (result.payloadLength < 0) {
        result.payloadLength = 0;
        return result;
    }
    result.rawPayload = fullFrame.mid(4, result.payloadLength);

    // CRC validation: covers type + addr + payload (from offset 2 to CRC byte)
    // CRC length = frameLen - 3 (type + addr + payload)
    result.crcReceived = data[fullFrame.size() - 1];
    if (validateCrc) {
        result.crcCalculated = crc8(data + 2, fullFrame.size() - 3, 0xD5, 0x00);
        result.crcValid = (result.crcCalculated == result.crcReceived);
    } else {
        result.crcValid = true;
    }

    result.sourceAddressName = addressName(result.deviceAddress);
    result.frameTypeName = frameTypeName(result.frameType);
    result.category = frameCategory(result.frameType);

    if (result.payloadLength > 0)
        parsePayload(result);

    return result;
}

void CrsfProtocolParser::parsePayload(ParsedCrsfFrame &frame) const
{
    const quint8 *pl = reinterpret_cast<const quint8*>(frame.rawPayload.constData());
    int plen = frame.rawPayload.size();

    switch (frame.frameType) {
    case 0x02: { // GPS
        if (plen >= 15) {
            CrsfGps gps;
            // Big-endian int32 for lat/lon
            gps.latitude  = static_cast<double>(static_cast<int32_t>(qFromBigEndian<qint32>(pl + 0))) / 10000000.0;
            gps.longitude = static_cast<double>(static_cast<int32_t>(qFromBigEndian<qint32>(pl + 4))) / 10000000.0;
            gps.groundspeed = qFromBigEndian<uint16_t>(pl + 8);
            gps.heading     = qFromBigEndian<uint16_t>(pl + 10);
            gps.altitude    = qFromBigEndian<uint16_t>(pl + 12);
            gps.satellites  = pl[14];
            frame.gps = gps;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("Lat=%1 Lon=%2 Spd=%3km/h Hdg=%4 Alt=%5m Sat=%6")
                .arg(gps.latitude, 0, 'f', 6)
                .arg(gps.longitude, 0, 'f', 6)
                .arg(gps.groundspeed * 0.036, 0, 'f', 1)
                .arg(gps.heading / 10.0, 0, 'f', 1)
                .arg(gps.altitude / 100.0, 0, 'f', 1)
                .arg(gps.satellites);
        }
        break;
    }
    case 0x03: { // GPS Time
        if (plen >= 9) {
            CrsfGpsTime gt;
            gt.year   = qFromBigEndian<uint16_t>(pl + 0);
            gt.month  = pl[2];
            gt.day    = pl[3];
            gt.hours  = pl[4];
            gt.minutes= pl[5];
            gt.seconds= pl[6];
            gt.millis = pl[7];
            frame.gpsTime = gt;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("%1-%2-%3 %4:%5:%6.%7")
                .arg(gt.year).arg(gt.month,2,10,QChar('0')).arg(gt.day,2,10,QChar('0'))
                .arg(gt.hours,2,10,QChar('0')).arg(gt.minutes,2,10,QChar('0'))
                .arg(gt.seconds,2,10,QChar('0')).arg(gt.millis,2,10,QChar('0'));
        }
        break;
    }
    case 0x07: { // Vario
        if (plen >= 2) {
            CrsfVario v;
            v.verticalSpeed = static_cast<int16_t>(qFromBigEndian<int16_t>(pl));
            frame.vario = v;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("VSpeed=%1m/s").arg(v.verticalSpeed / 100.0, 0, 'f', 2);
        }
        break;
    }
    case 0x08: { // Battery
        if (plen >= 8) {
            CrsfBattery bat;
            bat.voltage   = qFromBigEndian<uint16_t>(pl + 0);
            bat.current   = qFromBigEndian<uint16_t>(pl + 2);
            // capacity is 24-bit big-endian
            bat.capacity  = (static_cast<uint32_t>(pl[4]) << 16) | (static_cast<uint32_t>(pl[5]) << 8) | pl[6];
            bat.remaining = pl[7];
            frame.battery = bat;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("Volt=%1V Curr=%2A Cap=%3mAh Rem=%4%")
                .arg(bat.voltage * 0.01, 0, 'f', 2)
                .arg(bat.current * 0.01, 0, 'f', 2)
                .arg(bat.capacity)
                .arg(bat.remaining);
        }
        break;
    }
    case 0x09: { // Baro Alt
        if (plen >= 3) {
            CrsfBaroAlt ba;
            ba.altitude = qFromBigEndian<uint16_t>(pl + 0);
            // 3rd byte: vspeed
            ba.vspeed = static_cast<uint16_t>(pl[2]) * 10; // approx
            frame.baroAlt = ba;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("Alt=%1m VSpeed=%2m/s")
                .arg(ba.altitude / 100.0, 0, 'f', 1)
                .arg(ba.vspeed / 100.0, 0, 'f', 1);
        }
        break;
    }
    case 0x0B: { // Heartbeat
        frame.payloadDecoded = true;
        frame.payloadSummary = QString("(%1 bytes)").arg(plen);
        break;
    }
    case 0x13: { // ACC/GYRO
        if (plen >= 18) {
            CrsfAccGyro ag;
            int o = 0;
            ag.accX = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o)); o += 2;
            ag.accY = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o)); o += 2;
            ag.accZ = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o)); o += 2;
            ag.gyroX = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o)); o += 2;
            ag.gyroY = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o)); o += 2;
            ag.gyroZ = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + o));
            frame.accGyro = ag;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("Acc=%1,%2,%3 Gyro=%4,%5,%6")
                .arg(ag.accX).arg(ag.accY).arg(ag.accZ)
                .arg(ag.gyroX).arg(ag.gyroY).arg(ag.gyroZ);
        }
        break;
    }
    case 0x14: { // Link Statistics
        if (plen >= 10) {
            CrsfLinkStatistics ls;
            ls.uplinkRSSI1        = -static_cast<int8_t>(pl[0]);
            ls.uplinkRSSI2        = -static_cast<int8_t>(pl[1]);
            ls.uplinkLinkQuality  = pl[2];
            ls.uplinkSNR          = pl[3];
            ls.activeAntenna      = pl[4];
            ls.rfMode             = pl[5];
            ls.uplinkTxPower      = pl[6];
            ls.downlinkRSSI       = -static_cast<int8_t>(pl[7]);
            ls.downlinkLinkQuality= pl[8];
            ls.downlinkSNR        = static_cast<int8_t>(pl[9]);
            frame.linkStats = ls;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("RSSI=%1/%2 LQ=%3% SNR=%4 Ant=%5 RF=%6 Pwr=%7 DL_RSSI=%8 DL_LQ=%9 DL_SNR=%10dB")
                .arg(ls.uplinkRSSI1).arg(ls.uplinkRSSI2)
                .arg(ls.uplinkLinkQuality).arg(ls.uplinkSNR)
                .arg(ls.activeAntenna).arg(rfModeName(ls.rfMode))
                .arg(txPowerName(ls.uplinkTxPower))
                .arg(ls.downlinkRSSI).arg(ls.downlinkLinkQuality)
                .arg(ls.downlinkSNR);
        }
        break;
    }
    case 0x16: { // RC Channels Packed
        if (plen >= 22) {
            frame.rcChannels = parseRcChannels(pl, plen);
            frame.payloadDecoded = true;
            QStringList chParts;
            for (int i = 0; i < 16; ++i) {
                uint16_t pwm = channelValueToPwm(frame.rcChannels.channels[i]);
                chParts << QString("CH%1=%2").arg(i + 1).arg(pwm);
            }
            frame.payloadSummary = chParts.join(" ");
        }
        break;
    }
    case 0x17: { // Subset RC Channels
        auto subset = parseSubsetRcChannels(pl, plen);
        QStringList chParts;
        for (int i = 0; i < subset.channels.size(); ++i) {
            uint16_t chIdx = subset.startChannel + i;
            uint16_t pwm = channelValueToPwm(subset.channels[i]);
            chParts << QString("CH%1=%2").arg(chIdx + 1).arg(pwm);
        }
        frame.payloadSummary = QString("Start=%1 Res=%2bit ").arg(subset.startChannel).arg(subset.bitsPerChannel)
            + chParts.join(" ");
        if (!chParts.isEmpty())
            frame.payloadDecoded = true;
        break;
    }
    case 0x1C: { // Link Statistics RX (CRSFv3)
        if (plen >= 5) {
            CrsfLinkStatistics ls;
            ls.uplinkRSSI1        = -static_cast<int8_t>(pl[0]);
            ls.uplinkLinkQuality  = pl[2];
            ls.uplinkSNR          = static_cast<int8_t>(pl[3]);
            frame.linkStats = ls;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("RSSI=%1 LQ=%2% SNR=%3")
                .arg(ls.uplinkRSSI1).arg(ls.uplinkLinkQuality).arg(ls.uplinkSNR);
        }
        break;
    }
    case 0x1D: { // Link Statistics TX (CRSFv3)
        if (plen >= 6) {
            CrsfLinkStatistics ls;
            ls.uplinkRSSI1        = -static_cast<int8_t>(pl[0]);
            ls.uplinkLinkQuality  = pl[2];
            ls.uplinkSNR          = static_cast<int8_t>(pl[3]);
            frame.linkStats = ls;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("RSSI=%1 LQ=%2% SNR=%3")
                .arg(ls.uplinkRSSI1).arg(ls.uplinkLinkQuality).arg(ls.uplinkSNR);
        }
        break;
    }
    case 0x1E: { // Attitude
        if (plen >= 6) {
            CrsfAttitude att;
            att.pitch = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + 0));
            att.roll  = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + 2));
            att.yaw   = static_cast<int16_t>(qFromBigEndian<int16_t>(pl + 4));
            frame.attitude = att;
            frame.payloadDecoded = true;
            frame.payloadSummary = QString("Pitch=%1° Roll=%2° Yaw=%3°")
                .arg(att.pitch / 100.0, 0, 'f', 1)
                .arg(att.roll / 100.0, 0, 'f', 1)
                .arg(att.yaw / 100.0, 0, 'f', 1);
        }
        break;
    }
    case 0x21: { // Flight Mode
        if (plen > 0) {
            frame.payloadSummary = QString("Mode=%1")
                .arg(QString::fromUtf8(frame.rawPayload.mid(0, qMin(plen, 16))));
            frame.payloadDecoded = true;
        }
        break;
    }
    case 0x28: // Device Ping
    case 0x29: // Device Info
    case 0x2B: // Param Entry
    case 0x2C: // Param Read
    case 0x2D: // Param Write
    case 0x32: // Command
    {
        frame.payloadSummary = QString("(%1 bytes) ").arg(plen);
        if (plen >= 2) {
            QString src = addressName(pl[0]);
            QString dst = addressName(pl[1]);
            frame.payloadSummary += QString("Src=%1 Dst=%2").arg(src).arg(dst);
        }
        if (plen > 0) {
            QStringList plHex;
            for (int i = 0; i < qMin(plen, 16); ++i)
                plHex << QString("%1").arg(pl[i], 2, 16, QChar('0'));
            QString raw = plHex.join(" ");
            if (plen > 16) raw += "...";
            frame.payloadSummary += QString(" Raw=%1").arg(raw);
        }
        frame.payloadDecoded = true;
        break;
    }
    case 0x7A: // MSP Req
    case 0x7B: // MSP Resp
    case 0x7C: // MSP Write
    {
        frame.payloadSummary = QString("MSP(%1 bytes) ").arg(plen);
        if (plen >= 2) {
            frame.payloadSummary += QString("Seq=%1 Cmd=0x%2")
                .arg(pl[0]).arg(pl[1], 2, 16, QChar('0'));
        }
        if (plen > 2) {
            frame.payloadSummary += QString(" DataLen=%1").arg(plen - 2);
        }
        frame.payloadDecoded = true;
        break;
    }
    case 0x7D: { // DisplayPort
        if (plen >= 1) {
            QString subCmd;
            switch (pl[0]) {
            case 0x01: subCmd = "Update";  break;
            case 0x02: subCmd = "Clear";   break;
            case 0x03: subCmd = "Open";    break;
            case 0x04: subCmd = "Close";   break;
            case 0x05: subCmd = "Poll";    break;
            default:   subCmd = QString("0x%1").arg(pl[0], 2, 16, QChar('0')); break;
            }
            frame.payloadSummary = QString("DisplayPort %1 (%2 bytes)").arg(subCmd).arg(plen);
            frame.payloadDecoded = true;
        }
        break;
    }
    default: {
        // Unknown type - show hex of first few bytes
        if (plen > 0) {
            QStringList plHex;
            for (int i = 0; i < qMin(plen, 24); ++i)
                plHex << QString("%1").arg(pl[i], 2, 16, QChar('0'));
            frame.payloadSummary = QString("(%1 bytes) %2").arg(plen).arg(plHex.join(" "));
            if (plen > 24) frame.payloadSummary += "...";
        }
        break;
    }
    }
}

// ============================================================
// Constructor
// ============================================================
CrsfProtocolParser::CrsfProtocolParser()
{
}

// ============================================================
// Interval analysis
// ============================================================
CrsfIntervalAnalysis CrsfProtocolParser::analyzeInterval(qint64 newIntervalMs)
{
    CrsfIntervalAnalysis result;
    result.currentIntervalMs = newIntervalMs;
    result.sampleCount = m_intervalCount + 1;

    if (m_intervalCount == 0) {
        m_intervalMin = newIntervalMs;
        m_intervalMax = newIntervalMs;
        m_intervalSum = newIntervalMs;
    } else {
        if (newIntervalMs < m_intervalMin) m_intervalMin = newIntervalMs;
        if (newIntervalMs > m_intervalMax) m_intervalMax = newIntervalMs;
        m_intervalSum += newIntervalMs;
    }
    m_intervalCount++;

    result.minIntervalMs = m_intervalMin;
    result.maxIntervalMs = m_intervalMax;
    result.avgIntervalMs = m_intervalSum / m_intervalCount;

    // Try to detect pattern
    if (m_intervalCount >= 10) {
        qint64 avg = result.avgIntervalMs;
        if (avg >= 6 && avg <= 7) {
            result.detectedPattern = QString("%1ms ≈ 150Hz").arg(avg, 0, 'f', 1);
        } else if (avg >= 19 && avg <= 21) {
            result.detectedPattern = QString("%1ms ≈ 50Hz").arg(avg, 0, 'f', 1);
        } else if (avg >= 9 && avg <= 11) {
            result.detectedPattern = QString("%1ms ≈ 100Hz").arg(avg, 0, 'f', 1);
        } else if (avg >= 99 && avg <= 101) {
            result.detectedPattern = QString("%1ms ≈ 10Hz").arg(avg, 0, 'f', 1);
        } else {
            result.detectedPattern = QString("~%1Hz").arg(1000.0 / avg, 0, 'f', 0);
        }
    }

    return result;
}
