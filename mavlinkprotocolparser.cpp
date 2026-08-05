#include "mavlinkprotocolparser.h"
#include <QtGlobal>
#include <cstring>
#include <cmath>

// ============================================================
// MAVLink Protocol Parser implementation
// ============================================================

// CRC table for CRC-16/X-25 (poly 0x1021, init 0xFFFF, reflected)
static quint16 s_crcTable[256] = {0};
static bool s_crcTableInit = false;

static void initCrcTable()
{
    if (s_crcTableInit) return;
    for (int i = 0; i < 256; ++i) {
        quint16 crc = static_cast<quint16>(i);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001)
                crc = static_cast<quint16>((crc >> 1) ^ 0x8408); // reflected 0x1021
            else
                crc = static_cast<quint16>(crc >> 1);
        }
        s_crcTable[i] = crc;
    }
    s_crcTableInit = true;
}

quint16 MavlinkProtocolParser::crc16(const quint8 *data, int len, quint16 crc)
{
    initCrcTable();
    for (int i = 0; i < len; ++i)
        crc = static_cast<quint16>((crc >> 8) ^ s_crcTable[(crc ^ data[i]) & 0xFF]);
    return crc;
}

// ============================================================
// Frame parsing
// ============================================================
ParsedMavlinkFrame MavlinkProtocolParser::parseFrame(const QByteArray &frame)
{
    ParsedMavlinkFrame result;
    if (frame.isEmpty()) return result;

    const quint8 *raw = reinterpret_cast<const quint8*>(frame.constData());
    quint8 stx = raw[0];

    if (stx == MAVLINK_STX_V1) {
        // Need at least 6 header + payload + 2 crc
        if (frame.size() < 8) return result;
        result.version     = 1;
        result.stx         = stx;
        result.payloadLen  = raw[1];
        result.seq         = raw[2];
        result.sysid       = raw[3];
        result.compid      = raw[4];
        result.msgid       = raw[5];
        int total = 6 + result.payloadLen + 2;
        if (frame.size() < total) return result;   // incomplete
        result.totalLength = total;
        result.payload     = frame.mid(6, result.payloadLen);
        result.crcReceived = static_cast<quint16>(raw[6 + result.payloadLen])
                           | (static_cast<quint16>(raw[6 + result.payloadLen + 1]) << 8);
        result.hasSignature = false;
    }
    else if (stx == MAVLINK_STX_V2) {
        if (frame.size() < 12) return result;
        result.version     = 2;
        result.stx         = stx;
        result.payloadLen  = raw[1];
        result.incompatFlags = raw[2];
        result.compatFlags = raw[3];
        result.seq         = raw[4];
        result.sysid       = raw[5];
        result.compid      = raw[6];
        result.msgid       = raw[7] | (static_cast<uint32_t>(raw[8]) << 8);
        int total = 10 + result.payloadLen + 2;
        bool sig = (result.incompatFlags & 0x01) != 0;
        if (sig) total += MAVLINK_SIGNATURE_LEN;
        if (frame.size() < total) return result;   // incomplete
        result.totalLength = total;
        result.payload     = frame.mid(10, result.payloadLen);
        result.crcReceived = static_cast<quint16>(raw[10 + result.payloadLen])
                           | (static_cast<quint16>(raw[10 + result.payloadLen + 1]) << 8);
        result.hasSignature = sig;
        if (sig)
            result.signature = frame.mid(12 + result.payloadLen, MAVLINK_SIGNATURE_LEN);
    }
    else {
        return result;  // unknown stx -> not a MAVLink frame
    }

    result.valid = true;

    // ============================================================
    // CRC check
    // ============================================================
    // MAVLink CRC covers header (without stx) + payload + message-specific
    // extra byte (crc_extra). We use crc_extra=0 for generic check and report
    // "valid" only when framing is right. A perfect CRC match requires the
    // per-message crc_extra; we do a best-effort match here.
    quint16 crc = crc16(raw + 1, (result.version == 1 ? 5 : 9) + result.payloadLen);
    // Best-effort: omit per-message crc_extra; report separately.
    result.crcCalculated = crc;
    result.crcValid = (crc == result.crcReceived);

    result.msgName = messageName(result.msgid);
    result.hexDump = QString::fromLatin1(frame.left(result.totalLength).toHex(' ').toUpper());

    // Decode known messages
    decodeMessage(result);

    return result;
}

// ============================================================
// Little-endian read helpers
// ============================================================
double  MavlinkProtocolParser::leDouble(const quint8 *p, int off){ quint64 v=0; for(int i=0;i<8;++i) v|=((quint64)p[off+i])<<(8*i); double d; memcpy(&d,&v,8); return d; }
float   MavlinkProtocolParser::leFloat(const quint8 *p, int off){ quint32 v=0; for(int i=0;i<4;++i) v|=((quint32)p[off+i])<<(8*i); float f; memcpy(&f,&v,4); return f; }
quint64 MavlinkProtocolParser::leU64(const quint8 *p, int off){ quint64 v=0; for(int i=0;i<8;++i) v|=((quint64)p[off+i])<<(8*i); return v; }
qint32  MavlinkProtocolParser::leI32(const quint8 *p, int off){ quint32 v=0; for(int i=0;i<4;++i) v|=((quint32)p[off+i])<<(8*i); return (qint32)v; }
quint32 MavlinkProtocolParser::leU32(const quint8 *p, int off){ quint32 v=0; for(int i=0;i<4;++i) v|=((quint32)p[off+i])<<(8*i); return v; }
qint16  MavlinkProtocolParser::leI16(const quint8 *p, int off){ quint16 v=0; for(int i=0;i<2;++i) v|=((quint16)p[off+i])<<(8*i); return (qint16)v; }
quint16 MavlinkProtocolParser::leU16(const quint8 *p, int off){ quint16 v=0; for(int i=0;i<2;++i) v|=((quint16)p[off+i])<<(8*i); return v; }
qint8   MavlinkProtocolParser::leI8(const quint8 *p, int off){ return (qint8)p[off]; }

// ============================================================
// Message identification
// ============================================================
QString MavlinkProtocolParser::messageName(uint32_t msgid)
{
    switch (msgid) {
    case 0:   return "HEARTBEAT";
    case 1:   return "SYS_STATUS";
    case 2:   return "SYSTEM_TIME";
    case 4:   return "PING";
    case 24:  return "GPS_RAW_INT";
    case 30:  return "ATTITUDE";
    case 33:  return "GLOBAL_POSITION_INT";
    case 36:  return "SERVO_OUTPUT_RAW";
    case 65:  return "RC_CHANNELS";
    case 74:  return "VFR_HUD";
    case 147: return "BATTERY_STATUS";
    case 253: return "STATUSTEXT";
    default:  return QString("MSG_%1").arg(msgid);
    }
}

void MavlinkProtocolParser::decodeMessage(ParsedMavlinkFrame &f)
{
    if (!f.valid || f.payload.isEmpty()) {
        f.summary = QString("payload %1 bytes").arg(f.payloadLen);
        return;
    }
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    int len = f.payload.size();

    switch (f.msgid) {
    case 0:  decodeHeartbeat(f); break;
    case 1:  decodeSysStatus(f); break;
    case 24: decodeGpsRawInt(f); break;
    case 30: decodeAttitude(f); break;
    case 33: decodeGlobalPositionInt(f); break;
    case 36: decodeServoOutputRaw(f); break;
    case 65: decodeRcChannels(f); break;
    case 74: decodeVfrHud(f); break;
    case 147: decodeBatteryStatus(f); break;
    default:
        f.summary = QString("%1 bytes payload: %2")
                        .arg(len)
                        .arg(QString::fromLatin1(f.payload.toHex(' ').toUpper()));
        break;
    }
    (void)p; (void)len;
}

void MavlinkProtocolParser::decodeHeartbeat(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 9) { f.summary = "HEARTBEAT (short)"; return; }
    quint32 type = leU32(p, 0);
    quint32 autopilot = leU32(p, 4);
    // base_mode byte 8, custom_mode bytes 9..12, system_status 13, mavlink_version 14
    quint8 baseMode = (f.payload.size() > 8) ? p[8] : 0;
    quint8 sysStatus = (f.payload.size() > 13) ? p[13] : 0;

    QString mode;
    if (baseMode & 0x80) mode += "armed ";
    if (baseMode & 0x04) mode += "guided ";
    if (baseMode & 0x01) mode += "manual ";
    if (mode.isEmpty()) mode = "unknown";

    f.summary = QString("type=%1 autopilot=%2 mode[%3] status=%4")
                    .arg(type).arg(autopilot).arg(mode.trimmed()).arg(sysStatus);
}

void MavlinkProtocolParser::decodeSysStatus(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 31) { f.summary = "SYS_STATUS (short)"; return; }
    // voltage_battery mV @0, current_battery cA @2, battery_remaining % @4
    quint16 voltage = leU16(p, 0);
    qint16 current = leI16(p, 2);
    qint8  remaining = (f.payload.size() > 4) ? (qint8)p[4] : -1;
    f.summary = QString("Vbat=%1.%2V I=%3.%4A rem=%5%")
                    .arg(voltage / 1000).arg((voltage % 1000) / 100)
                    .arg(current / 10).arg(qAbs(current) % 10)
                    .arg(remaining < 0 ? QString("--") : QString::number(remaining));
}

void MavlinkProtocolParser::decodeGpsRawInt(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 30) { f.summary = "GPS_RAW_INT (short)"; return; }
    quint64 t = leU64(p, 0);   // time_usec (J)
    qint32 lat = leI32(p, 8);  // degE7
    qint32 lon = leI32(p, 12);
    qint32 alt = leI32(p, 16); // mm
    quint16 eph = leU16(p, 20);
    quint16 epv = leU16(p, 22);
    quint16 vel = leU16(p, 24); // cm/s
    quint16 cog = leU16(p, 26); // cdeg
    quint8 fix = (f.payload.size() > 28) ? p[28] : 0;
    quint8 sat = (f.payload.size() > 29) ? p[29] : 0;

    f.summary = QString("fix=%1 sats=%2 lat=%3 lon=%4 alt=%5m vel=%6m/s cog=%7°")
                    .arg(fix).arg(sat)
                    .arg(lat / 1e7, 0, 'f', 7).arg(lon / 1e7, 0, 'f', 7)
                    .arg(alt / 1000.0, 0, 'f', 1)
                    .arg(vel / 100.0, 0, 'f', 1)
                    .arg(cog / 100.0, 0, 'f', 1);
    (void)t; (void)eph; (void)epv;
}

void MavlinkProtocolParser::decodeAttitude(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 28) { f.summary = "ATTITUDE (short)"; return; }
    // roll, pitch, yaw rad @0,4,8 ; rollspeed.. @12,16,20 (rad/s)
    float roll = leFloat(p, 0);
    float pitch = leFloat(p, 4);
    float yaw = leFloat(p, 8);
    auto deg = [](float r){ return r * 180.0f / 3.14159265f; };
    f.summary = QString("roll=%1° pitch=%2° yaw=%3°")
                    .arg(deg(roll), 0, 'f', 1)
                    .arg(deg(pitch), 0, 'f', 1)
                    .arg(deg(yaw), 0, 'f', 1);
}

void MavlinkProtocolParser::decodeGlobalPositionInt(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 28) { f.summary = "GLOBAL_POSITION_INT (short)"; return; }
    qint32 lat = leI32(p, 4);
    qint32 lon = leI32(p, 8);
    qint32 alt = leI32(p, 12);        // mm MSL
    qint32 relAlt = leI32(p, 16);     // mm relative
    qint16 vx = leI16(p, 20);         // cm/s
    qint16 vy = leI16(p, 22);
    qint16 vz = leI16(p, 24);
    quint16 hdg = leU16(p, 26);       // cdeg
    f.summary = QString("lat=%1 lon=%2 alt=%3m rel=%4m hdg=%5°")
                    .arg(lat / 1e7, 0, 'f', 7).arg(lon / 1e7, 0, 'f', 7)
                    .arg(alt / 1000.0, 0, 'f', 1).arg(relAlt / 1000.0, 0, 'f', 1)
                    .arg(hdg / 100.0, 0, 'f', 1);
    (void)vx; (void)vy; (void)vz;
}

void MavlinkProtocolParser::decodeServoOutputRaw(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 21) { f.summary = "SERVO_OUTPUT_RAW (short)"; return; }
    quint32 timeUsec = leU32(p, 0);
    quint8 port = p[4];
    quint16 servo1 = leU16(p, 5);
    quint16 servo2 = leU16(p, 7);
    quint16 servo3 = leU16(p, 9);
    quint16 servo4 = leU16(p, 11);
    f.summary = QString("port=%1 s1=%2 s2=%3 s3=%4 s4=%5 (us)")
                    .arg(port).arg(servo1).arg(servo2).arg(servo3).arg(servo4);
    (void)timeUsec;
}

void MavlinkProtocolParser::decodeRcChannels(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 37) { f.summary = "RC_CHANNELS (short)"; return; }
    quint32 timeBoot = leU32(p, 0);
    quint8 count = p[4];
    quint16 ch[8];
    for (int i = 0; i < 8; ++i) ch[i] = leU16(p, 5 + i * 2);
    f.summary = QString("count=%1 ch1..8: %2").arg(count);
    QStringList vals;
    for (int i = 0; i < 8; ++i) vals << QString::number(ch[i]);
    f.summary += " " + vals.join(",");
    (void)timeBoot;
}

void MavlinkProtocolParser::decodeBatteryStatus(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 36) { f.summary = "BATTERY_STATUS (short)"; return; }
    quint8 id = p[0];
    quint16 voltage = leU16(p, 10);          // mV, cell 0
    qint16 current = leI16(p, 12);           // cA (10 mA)
    qint8 remaining = (f.payload.size() > 14) ? (qint8)p[14] : -1;
    quint16 temp = leU16(p, 36); // temperature cdegC if present
    f.summary = QString("id=%1 V=%2.%3V I=%4.%5A rem=%6%")
                    .arg(id)
                    .arg(voltage / 1000).arg((voltage % 1000) / 100)
                    .arg(current / 10).arg(qAbs(current) % 10)
                    .arg(remaining < 0 ? QString("--") : QString::number(remaining));
    (void)temp;
}

void MavlinkProtocolParser::decodeVfrHud(ParsedMavlinkFrame &f)
{
    const quint8 *p = reinterpret_cast<const quint8*>(f.payload.constData());
    if (f.payload.size() < 28) { f.summary = "VFR_HUD (short)"; return; }
    float airspeed = leFloat(p, 0);
    float groundspeed = leFloat(p, 4);
    float heading = leFloat(p, 8);
    quint16 throttle = leU16(p, 12);
    float alt = leFloat(p, 14);
    float climb = leFloat(p, 18);
    f.summary = QString("airspeed=%1m/s gnd=%2m/s hdg=%3° thr=%4% alt=%5m climb=%6m/s")
                    .arg(airspeed, 0, 'f', 1).arg(groundspeed, 0, 'f', 1)
                    .arg(heading, 0, 'f', 1).arg(throttle)
                    .arg(alt, 0, 'f', 1).arg(climb, 0, 'f', 1);
}
