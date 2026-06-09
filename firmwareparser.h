#ifndef FIRMWAREPARSER_H
#define FIRMWAREPARSER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <cstdint>

// 天空端固件结构体常量
struct SkyStructs {
    static constexpr uint32_t CUSTOM_MAGIC = 0x43554D54; // "CUST"
    static constexpr int CUSTOM_HEAD_SIZE = 24;           // 4 + 4 + 16
    // 主头部固定字段前20字节: magic(4)+board(4)+sdk(4)+app(4)+qa(4)
    static constexpr int IMG_HEAD_BASE = 20;
};

// 地面端固件结构体常量（与天空端一致为IMG_HEAD_INFO_EX结构）
struct GroundStructs {
    static constexpr uint32_t CUSTOM_MAGIC = 0x43554D54;
    static constexpr int CUSTOM_HEAD_SIZE = 24;
    // 与天空端一致的IMG_HEAD_INFO_EX结构:
    // magic(4)+board(4)+sdk(4)+app(4)+qa(4)+subImg[5](5×12)+reserved(29)+custom(1)+chip_id(1)+upgrade_flags(1)+crc32(4)+total_size(4)+buildTime(8) = 128
    static constexpr int IMG_HEAD_SIZE = 128;
};

class FirmwareParser
{
public:
    // 解析天空端固件
    static QVariantMap parseSkyFirmware(const QByteArray &data);

    // 解析地面端固件
    static QVariantMap parseGroundFirmware(const QByteArray &data);

    // 生成格式化显示文本
    static QString formatResult(const QVariantMap &result, const QString &title);

private:
    static uint32_t readU32(const QByteArray &data, int &offset);
    static uint64_t readU64(const QByteArray &data, int &offset);
    static uint16_t readU16(const QByteArray &data, int &offset);
    static uint8_t  readU8(const QByteArray &data, int &offset);
    static QString  readString(const QByteArray &data, int &offset, int length);
};

#endif // FIRMWAREPARSER_H
