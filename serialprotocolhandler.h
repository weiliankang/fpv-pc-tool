#ifndef SERIALPROTOCOLHANDLER_H
#define SERIALPROTOCOLHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <QStringList>

// 命令定义
#define WIRELESS_CMD_SET_CHANNEL 0x22
#define WIRELESS_CMD_GET_CHANNEL 0xA2

// 无线参数数据类型
typedef enum {
    WIRELESS_DATA_TYPE_SET_CHANNEL_HOP = 0x50,
    WIRELESS_DATA_TYPE_GET_CHANNEL_HOP = 0x51,
    WIRELESS_DATA_TYPE_GET_STATUS      = 0x52,
    WIRELESS_DATA_TYPE_GET_BB_PWR     = 0x53,
    WIRELESS_DATA_TYPE_SET_BB_PWR     = 0x54,
    WIRELESS_DATA_TYPE_GET_STATUS_SKY = 0x55,
    WIRELESS_DATA_TYPE_GET_DISTANCE   = 0x56,
    WIRELESS_DATA_TYPE_GET_OSD_DATA   = 0x57,
    // 中继(Relay)命令
    WIRELESS_DATA_TYPE_SET_RELAY_CHANNEL_HOP     = 0xA0,
    WIRELESS_DATA_TYPE_GET_RELAY_CHANNEL_HOP     = 0xA1,
    WIRELESS_DATA_TYPE_GET_RELAY_STATUS          = 0xA2,
    WIRELESS_DATA_TYPE_GET_RELAY_BB_PWR          = 0xA3,
    WIRELESS_DATA_TYPE_SET_RELAY_BB_PWR          = 0xA4,
    WIRELESS_DATA_TYPE_GET_RELAY_GND_DISTANCE    = 0xA5,
    WIRELESS_DATA_TYPE_GET_RELAY_SKY_DISTANCE    = 0xA6,
    WIRELESS_DATA_TYPE_GET_RELAY_OSD_DATA        = 0xA7,
} wireless_data_type_t;

class SerialProtocolHandler : public QObject
{
    Q_OBJECT
public:
    explicit SerialProtocolHandler(QObject *parent = nullptr);

    // 协议常量
    static const QByteArray HEADER;
    static const QByteArray FOOTER;
    static const quint8 KEY_CMD;
    static const quint8 KEY_TYPE;

    // 按键映射
    static const QMap<QString, quint8> KEY_MAPPINGS;
    static const QMap<QString, quint8> KEY_PRESS_TYPES;
    static const QMap<quint8, QString> BAND_MAPPING;

    // 计算校验和
    static quint16 calculateChecksum(const QByteArray &data);

    // 创建按键命令
    static QByteArray createKeyCommand(const QString &keyName, int pressType = -1);

    // 无线数据类型常量（对应 fpv_wireless_system.h）
    static constexpr quint8 CMD_SET_CHANNEL_HOP  = 0x50;
    static constexpr quint8 CMD_GET_CHANNEL_HOP  = 0x51;
    static constexpr quint8 CMD_GET_STATUS       = 0x52;
    static constexpr quint8 CMD_GET_BB_PWR       = 0x53;
    static constexpr quint8 CMD_SET_BB_PWR       = 0x54;
    static constexpr quint8 CMD_GET_STATUS_SKY   = 0x55;
    static constexpr quint8 CMD_GET_DISTANCE     = 0x56;
    static constexpr quint8 CMD_GET_OSD_DATA     = 0x57;

    // 中继(Relay)命令
    static constexpr quint8 CMD_SET_RELAY_CHANNEL_HOP  = 0xA0;
    static constexpr quint8 CMD_GET_RELAY_CHANNEL_HOP  = 0xA1;
    static constexpr quint8 CMD_GET_RELAY_STATUS       = 0xA2;
    static constexpr quint8 CMD_GET_RELAY_BB_PWR       = 0xA3;
    static constexpr quint8 CMD_SET_RELAY_BB_PWR       = 0xA4;
    static constexpr quint8 CMD_GET_RELAY_GND_DISTANCE = 0xA5;
    static constexpr quint8 CMD_GET_RELAY_SKY_DISTANCE = 0xA6;
    static constexpr quint8 CMD_GET_RELAY_OSD_DATA     = 0xA7;

    // 创建无线命令
    static QByteArray createWirelessCommand(quint8 cmdType, const QByteArray &values);

    // 解析数据包
    static QVariantMap parsePacket(const QByteArray &packet);

    // 解析无线回复内容并返回格式化的状态文本
    static QString parseWirelessResponse(const QByteArray &dataContent);

    // 16进制字符串转字节
    static QByteArray hexStringToBytes(const QString &hexStr);

    // 字节转16进制字符串
    static QString bytesToHexString(const QByteArray &data);

    // 获取按键名称列表
    static QStringList keyNames();
};

#endif // SERIALPROTOCOLHANDLER_H
