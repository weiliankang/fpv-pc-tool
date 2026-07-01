#include "serialprotocolhandler.h"
#include <QRegularExpression>
#include <QDebug>

const QByteArray SerialProtocolHandler::HEADER = QByteArrayLiteral("\xFE\xEF");
const QByteArray SerialProtocolHandler::FOOTER = QByteArrayLiteral("\x0D\x0A");
const quint8 SerialProtocolHandler::KEY_CMD = 0x22;
const quint8 SerialProtocolHandler::KEY_TYPE = 0x40;

const QMap<QString, quint8> SerialProtocolHandler::KEY_MAPPINGS = {
    {"上键", 0}, {"下键", 1}, {"左键", 2}, {"右键", 3},
    {"确认键", 4}, {"配对按键", 5}, {"升级按键", 5},
    {"录像键", 6}, {"回退键", 7}, {"强制720p60按键", 7},
    {"debug3模式按键", 9}
};

const QMap<QString, quint8> SerialProtocolHandler::KEY_PRESS_TYPES = {
    {"上键", 0}, {"下键", 0}, {"左键", 0}, {"右键", 0},
    {"确认键", 0}, {"配对按键", 0}, {"升级按键", 1},
    {"录像键", 0}, {"回退键", 0}, {"强制720p60按键", 1},
    {"debug3模式按键", 0}
};

const QMap<quint8, QString> SerialProtocolHandler::BAND_MAPPING = {
    {0, "BandA"}, {1, "BandB"}, {2, "BandC"}
};

SerialProtocolHandler::SerialProtocolHandler(QObject *parent)
    : QObject(parent)
{
}

quint16 SerialProtocolHandler::calculateChecksum(const QByteArray &data)
{
    quint32 sum = 0;
    for (int i = 0; i < data.size(); ++i)
        sum += static_cast<quint8>(data.at(i));
    return static_cast<quint16>(sum & 0xFFFF);
}

QByteArray SerialProtocolHandler::createKeyCommand(const QString &keyName, int pressType)
{
    if (!KEY_MAPPINGS.contains(keyName))
        throw QString("未知按键: %1").arg(keyName);

    quint8 keyNum = KEY_MAPPINGS[keyName];
    if (pressType < 0)
        pressType = KEY_PRESS_TYPES.value(keyName, 0);

    QByteArray dataContent;
    dataContent.append(static_cast<char>(KEY_TYPE));
    dataContent.append(static_cast<char>(keyNum));
    dataContent.append(static_cast<char>(pressType));
    dataContent.append(static_cast<char>(0));

    quint16 checksum = calculateChecksum(dataContent);

    QByteArray packet;
    packet.append(HEADER);
    packet.append(static_cast<char>(KEY_CMD));
    packet.append(static_cast<char>((dataContent.size() >> 8) & 0xFF));
    packet.append(static_cast<char>(dataContent.size() & 0xFF));
    packet.append(dataContent);
    packet.append(static_cast<char>((checksum >> 8) & 0xFF));
    packet.append(static_cast<char>(checksum & 0xFF));
    packet.append(FOOTER);

    return packet;
}

QByteArray SerialProtocolHandler::createWirelessCommand(quint8 dataType, const QByteArray &values)
{
    // 设置类命令用 0x22，查询类命令用 0xA2
    quint8 cmdByte;
    switch (dataType) {
    case WIRELESS_DATA_TYPE_SET_CHANNEL_HOP:
    case WIRELESS_DATA_TYPE_SET_BB_PWR:
        cmdByte = WIRELESS_CMD_SET_CHANNEL;
        break;
    default:
        cmdByte = WIRELESS_CMD_GET_CHANNEL;
        break;
    }

    QByteArray dataContent;
    dataContent.append(static_cast<char>(dataType));
    for (int i = 0; i < 7; ++i) {
        if (i < values.size())
            dataContent.append(values.at(i));
        else
            dataContent.append(static_cast<char>(0));
    }

    quint16 checksum = calculateChecksum(dataContent);

    QByteArray packet;
    packet.append(HEADER);
    packet.append(static_cast<char>(cmdByte));
    packet.append(static_cast<char>((dataContent.size() >> 8) & 0xFF));
    packet.append(static_cast<char>(dataContent.size() & 0xFF));
    packet.append(dataContent);
    packet.append(static_cast<char>((checksum >> 8) & 0xFF));
    packet.append(static_cast<char>(checksum & 0xFF));
    packet.append(FOOTER);

    return packet;
}

QVariantMap SerialProtocolHandler::parsePacket(const QByteArray &packet)
{
    QVariantMap result;

    if (packet.size() < 9)
        return result;

    if (packet.left(2) != HEADER || packet.right(2) != FOOTER)
        return result;

    result["command"]     = static_cast<quint8>(packet.at(2));
    int dataLen           = (static_cast<quint8>(packet.at(3)) << 8) | static_cast<quint8>(packet.at(4));
    result["data_length"] = dataLen;

    QByteArray dataContent = packet.mid(5, dataLen);
    result["data_content"] = dataContent;

    quint16 chk = (static_cast<quint8>(packet.at(5 + dataLen)) << 8) |
                   static_cast<quint8>(packet.at(5 + dataLen + 1));
    result["checksum"] = static_cast<quint16>(chk);
    result["checksum_valid"] = (calculateChecksum(dataContent) == chk);
    result["raw"] = packet;

    return result;
}

QByteArray SerialProtocolHandler::hexStringToBytes(const QString &hexStr)
{
    QString cleaned = hexStr;
    cleaned.remove(QRegularExpression("[\\s\\n\\r\\t]"));

    if (cleaned.isEmpty())
        throw QString("空的16进制字符串");

    if (!QRegularExpression("^[0-9A-Fa-f]+$").match(cleaned).hasMatch())
        throw QString("无效的16进制字符串");

    if (cleaned.length() % 2 != 0)
        cleaned.prepend('0');

    return QByteArray::fromHex(cleaned.toLatin1());
}

QString SerialProtocolHandler::bytesToHexString(const QByteArray &data)
{
    QStringList parts;
    for (int i = 0; i < data.size(); ++i) {
        parts.append(QString("%1").arg(static_cast<quint8>(data.at(i)), 2, 16, QChar('0')).toUpper());
    }
    return parts.join(" ");
}

QStringList SerialProtocolHandler::keyNames()
{
    return KEY_MAPPINGS.keys();
}

QString SerialProtocolHandler::parseWirelessResponse(const QByteArray &dataContent)
{
    if (dataContent.isEmpty())
        return QString();

    quint8 cmdType = static_cast<quint8>(dataContent.at(0));

    switch (cmdType) {
    case CMD_GET_CHANNEL_HOP: // 0x51 频点回复
        if (dataContent.size() >= 4) {
            quint8 band     = static_cast<quint8>(dataContent.at(1));
            quint8 channel  = static_cast<quint8>(dataContent.at(2));
            quint8 hop      = static_cast<quint8>(dataContent.at(3));
            QString bandName = BAND_MAPPING.value(band, QString("未知(%1)").arg(band));
            QString hopMode  = (hop == 1) ? "跳频" : "定频";
            return QString("当前频点: %1, Channel=%2, %3").arg(bandName).arg(channel).arg(hopMode);
        }
        break;

    case CMD_GET_STATUS: // 0x52 地面端无线状态
        if (dataContent.size() >= 7) {
            qint8 rssi1  = static_cast<qint8>(dataContent.at(1));
            qint8 rssi2  = static_cast<qint8>(dataContent.at(2));
            quint8 rate  = static_cast<quint8>(dataContent.at(4));
            quint8 delay = static_cast<quint8>(dataContent.at(5));
            quint8 conn  = static_cast<quint8>(dataContent.at(6));
            return QString("RSSI1=%1dB, RSSI2=%2dB, 速率=%3Mbps, 延迟=%4ms, 连接=%5")
                       .arg(rssi1).arg(rssi2).arg(rate).arg(delay)
                       .arg(conn ? "是" : "否");
        }
        break;

    case CMD_GET_BB_PWR: // 0x53 获取BB功率
        if (dataContent.size() >= 7) {
            quint8 curIdx = static_cast<quint8>(dataContent.at(1));
            QString pwrMap;
            for (int i = 0; i < 4; ++i)
                pwrMap += QString("%1 ").arg(static_cast<quint8>(dataContent.at(2 + i)));
            return QString("当前功率索引: %1, 功率表: %2").arg(curIdx).arg(pwrMap.trimmed());
        }
        break;

    case CMD_SET_BB_PWR: // 0x54 设置BB功率回复
        return QString("BB功率设置完成");

    case CMD_GET_STATUS_SKY: // 0x55 天空端无线状态
        if (dataContent.size() >= 3) {
            qint8 rssi1 = static_cast<qint8>(dataContent.at(1));
            qint8 rssi2 = static_cast<qint8>(dataContent.at(2));
            return QString("天空端 RSSI1=%1dB, RSSI2=%2dB").arg(rssi1).arg(rssi2);
        }
        break;

    case CMD_GET_DISTANCE: // 0x56 获取距离
        if (dataContent.size() >= 5) {
            // distance[4] 是小端 int32
            qint32 dist = *reinterpret_cast<const qint32*>(dataContent.constData() + 1);
            return QString("距离: %1 米").arg(dist);
        }
        break;

    case CMD_GET_OSD_DATA: // 0x57 OSD数据
        if (dataContent.size() > 1) {
            int payloadSize = dataContent.size() - 1;
            const int displayLen = (payloadSize > 64) ? 64 : payloadSize;
            QString hexDump;
            for (int i = 0; i < displayLen; ++i) {
                hexDump += QString("%1 ").arg(static_cast<quint8>(dataContent.at(1 + i)), 2, 16, QChar('0'));
            }
            if (payloadSize > 64) hexDump += "...";
            return QString("OSD数据: %1 字节\n原始数据前64字节: %2").arg(payloadSize).arg(hexDump);
        }
        break;
    }

    return QString();
}
