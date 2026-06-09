#include "firmwareparser.h"
#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>
#include <QDateTime>
#include <cstdint>

// ============================================================
// 工具函数
// ============================================================

uint32_t FirmwareParser::readU32(const QByteArray &data, int &offset)
{
    if (offset + 4 > data.size()) throw QString("readU32越界");
    uint32_t val = static_cast<uint8_t>(data.at(offset))
                 | (static_cast<uint8_t>(data.at(offset + 1)) << 8)
                 | (static_cast<uint8_t>(data.at(offset + 2)) << 16)
                 | (static_cast<uint8_t>(data.at(offset + 3)) << 24);
    offset += 4;
    return val;
}

uint64_t FirmwareParser::readU64(const QByteArray &data, int &offset)
{
    if (offset + 8 > data.size()) throw QString("readU64越界");
    uint64_t val = static_cast<uint8_t>(data.at(offset))
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 1))) << 8)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 2))) << 16)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 3))) << 24)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 4))) << 32)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 5))) << 40)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 6))) << 48)
                 | (static_cast<uint64_t>(static_cast<uint8_t>(data.at(offset + 7))) << 56);
    offset += 8;
    return val;
}

uint16_t FirmwareParser::readU16(const QByteArray &data, int &offset)
{
    if (offset + 2 > data.size()) throw QString("readU16越界");
    uint16_t val = static_cast<uint8_t>(data.at(offset))
                 | (static_cast<uint8_t>(data.at(offset + 1)) << 8);
    offset += 2;
    return val;
}

uint8_t FirmwareParser::readU8(const QByteArray &data, int &offset)
{
    if (offset + 1 > data.size()) throw QString("readU8越界");
    uint8_t val = static_cast<uint8_t>(data.at(offset));
    offset += 1;
    return val;
}

QString FirmwareParser::readString(const QByteArray &data, int &offset, int length)
{
    if (offset + length > data.size()) {
        offset = data.size();
        return QString();
    }
    QString str = QString::fromUtf8(data.mid(offset, length)).trimmed();
    offset += length;
    return str;
}

// ============================================================
// 解析天空端固件
// ============================================================

QVariantMap FirmwareParser::parseSkyFirmware(const QByteArray &data)
{
    QVariantMap result;
    int offset = 0;

    // 先判断开头4字节是否为自定义魔数
    // 两种格式：
    //   格式1: CUSTOM_IMG_HEAD_INFO_EX + IMG_HEAD_INFO_EX + 固件数据
    //   格式2: IMG_HEAD_INFO_EX + 固件数据
    if ((uint32_t)data.size() >= 4) {
        uint32_t firstMagic = readU32(data, offset);
        if (firstMagic == SkyStructs::CUSTOM_MAGIC) {
            // 格式1: 解析自定义头部
            uint32_t customVersion = readU32(data, offset);
            QByteArray customerBytes = data.mid(offset, 16); offset += 16;

            result["custom_magic"]   = static_cast<quint32>(firstMagic);
            result["custom_version"] = static_cast<quint32>(customVersion);
            result["customer_code"]  = QString::fromLatin1(customerBytes.toHex(' ').toUpper());
            result["custom_valid"]   = true;
        } else {
            // 格式2: 没有自定义头部，回退到开头重新解析
            result["custom_valid"] = false;
            offset = 0;
        }
    } else {
        result["error"] = "文件太小";
        return result;
    }

    // --- 主头部 IMG_HEAD_INFO_EX ---
    if (offset + SkyStructs::IMG_HEAD_BASE > data.size()) {
        result["error"] = "文件不完整";
        return result;
    }

    try {
        uint32_t magic       = readU32(data, offset);
        uint32_t boardType   = readU32(data, offset);
        uint32_t sdkVersion  = readU32(data, offset);
        uint32_t appVersion  = readU32(data, offset);
        uint32_t qaVersion   = readU32(data, offset);

        result["magic"]       = static_cast<quint32>(magic);
        result["board_type"]  = static_cast<quint32>(boardType);
        result["sdk_version"] = static_cast<quint32>(sdkVersion);
        result["app_version"] = static_cast<quint32>(appVersion);
        result["qa_version"]  = static_cast<quint32>(qaVersion);

        // 子镜像信息 (MAX_FILE_NUM)
        QVariantList subImages;
        for (int i = 0; i < 5; ++i) {
            if (offset + 12 > data.size()) break;
            int8_t  imgType  = static_cast<int8_t>(readU8(data, offset));
            int8_t  upgrade  = static_cast<int8_t>(readU8(data, offset));
            int8_t  dualPart = static_cast<int8_t>(readU8(data, offset));
            // res[1] 跳过
            readU8(data, offset);
            uint32_t flashOffset    = readU32(data, offset);
            uint32_t subImgOffset   = readU32(data, offset);
            QVariantMap si;
            si["img_type"]      = static_cast<int>(imgType);
            si["upgrade"]       = static_cast<int>(upgrade);
            si["dual_part"]     = static_cast<int>(dualPart);
            si["flash_offset"]  = static_cast<quint32>(flashOffset);
            si["subimg_offset"] = static_cast<quint32>(subImgOffset);
            subImages.append(si);
        }
        result["sub_images"] = subImages;

        // 保留字段 29字节
        QString reserved = readString(data, offset, 29);
        result["reserved"] = reserved;

        // u8Custom, u8ChipId, u8UpgradeFlags
        if (offset + 3 <= data.size()) {
            uint8_t custom     = readU8(data, offset);
            uint8_t chipId     = readU8(data, offset);
            uint8_t upgradeFlg = readU8(data, offset);
            result["custom"]        = static_cast<quint8>(custom);
            result["chip_id"]       = static_cast<quint8>(chipId);
            result["upgrade_flags"] = static_cast<quint8>(upgradeFlg);
        }

        // CRC32
        if (offset + 4 <= data.size()) {
            uint32_t crc = readU32(data, offset);
            result["crc32"] = static_cast<quint32>(crc);
        }

        // total_size
        if (offset + 4 <= data.size()) {
            uint32_t totalSize = readU32(data, offset);
            result["total_size"] = static_cast<quint32>(totalSize);
        }

        // 编译时间 u64BuilTime (格式: YYYYMMDDHHMM，十进制)
        if (offset + 8 <= data.size()) {
            uint64_t buildTime = readU64(data, offset);
            if (buildTime > 0) {
                int year   = static_cast<int>(buildTime / 100000000ULL);
                int month  = static_cast<int>((buildTime % 100000000ULL) / 1000000);
                int day    = static_cast<int>((buildTime % 1000000) / 10000);
                int hour   = static_cast<int>((buildTime % 10000) / 100);
                int minute = static_cast<int>(buildTime % 100);
                result["build_time"] = QString("%1-%2-%3 %4:%5")
                    .arg(year, 4, 10, QChar('0'))
                    .arg(month, 2, 10, QChar('0'))
                    .arg(day, 2, 10, QChar('0'))
                    .arg(hour, 2, 10, QChar('0'))
                    .arg(minute, 2, 10, QChar('0'));
            }
        }

        result["data_offset"] = offset;
        result["data_size"]   = data.size() - offset;

    } catch (...) {
        result["error"] = "解析主头部时发生异常";
    }

    return result;
}

// ============================================================
// 解析地面端固件
// 结构1: CUSTOM_IMG_HEAD_INFO_EX(24) + img_wsa_header_info(128) + 固件数据
// 结构2: img_wsa_header_info(128) + 固件数据
// 其中 img_wsa_header_info:
//   magic(4) + boardType(4) + sdkVersion(4) + appVersion(4) + qaVersion(4) + res[99](99) + u8Custom(1) + u64BuildTime(8) = 128字节
// ============================================================

QVariantMap FirmwareParser::parseGroundFirmware(const QByteArray &data)
{
    QVariantMap result;
    int offset = 0;

    // --- 判断是否包含客户定制头部 ---
    bool hasCustom = false;
    if (data.size() >= 4) {
        uint32_t firstMagic = readU32(data, offset);
        if (firstMagic == GroundStructs::CUSTOM_MAGIC) {
            // 格式1: 有 CUSTOM_IMG_HEAD_INFO_EX
            uint32_t customVersion = readU32(data, offset);
            QByteArray customerBytes = data.mid(offset, 16); offset += 16;
            result["custom_magic"]   = static_cast<quint32>(firstMagic);
            result["custom_version"] = static_cast<quint32>(customVersion);
            result["customer_code"]  = QString::fromLatin1(customerBytes.toHex(' ').toUpper());
            hasCustom = true;
        } else {
            // 格式2: 无客户头部或魔数不匹配，回退到开头
            offset = 0;
        }
    }
    result["custom_valid"] = hasCustom;

    // --- 主头部 img_wsa_header_info (128字节) ---
    if (offset + GroundStructs::IMG_HEAD_SIZE > data.size()) {
        result["error"] = "文件不完整";
        return result;
    }

    try {
        uint32_t magic      = readU32(data, offset);
        uint32_t boardType  = readU32(data, offset);
        uint32_t sdkVersion = readU32(data, offset);
        uint32_t appVersion = readU32(data, offset);
        uint32_t qaVersion  = readU32(data, offset);
        // 保留字段 99字节
        QString reserved = readString(data, offset, 99);
        uint8_t  custom     = readU8(data, offset);
        uint64_t buildTime  = readU64(data, offset);

        result["magic"]       = static_cast<quint32>(magic);
        result["board_type"]  = static_cast<quint32>(boardType);
        result["sdk_version"] = static_cast<quint32>(sdkVersion);
        result["app_version"] = static_cast<quint32>(appVersion);
        result["qa_version"]  = static_cast<quint32>(qaVersion);
        result["reserved"]    = reserved;
        result["custom"]      = static_cast<quint8>(custom);

        if (buildTime > 0) {
            int year   = static_cast<int>(buildTime / 100000000ULL);
            int month  = static_cast<int>((buildTime % 100000000ULL) / 1000000);
            int day    = static_cast<int>((buildTime % 1000000) / 10000);
            int hour   = static_cast<int>((buildTime % 10000) / 100);
            int minute = static_cast<int>(buildTime % 100);
            result["build_time"] = QString("%1-%2-%3 %4:%5")
                .arg(year, 4, 10, QChar('0'))
                .arg(month, 2, 10, QChar('0'))
                .arg(day, 2, 10, QChar('0'))
                .arg(hour, 2, 10, QChar('0'))
                .arg(minute, 2, 10, QChar('0'));
        }

        result["data_offset"] = offset;
        result["data_size"]   = data.size() - offset;

    } catch (...) {
        result["error"] = "解析主头部时发生异常";
    }

    return result;
}

// ============================================================
// 格式化显示结果
// ============================================================

QString FirmwareParser::formatResult(const QVariantMap &result, const QString &title)
{
    QString text;
    text += QString("=").repeated(60) + "\n";
    text += title + QStringLiteral("固件信息\n");
    text += QString("=").repeated(60) + " \n";

    // --- 客户定制头部 ---
    bool hasCustom = result.value("custom_valid").toBool();
    if (hasCustom) {
        text += QStringLiteral("【客户定制头部信息】\n");
        text += QString("魔数: 0x%1                   // 客户定制头部魔数，CUSTOM_MAGIC=0x43554D54\n").arg(result.value("custom_magic").toUInt(), 8, 16, QChar('0'));
        //text += QString("有效性: %1 \n").arg(result.value("custom_valid").toBool() ? "有效" : "无效");
        {   // 自定义版本映射
            uint32_t cv = result.value("custom_version").toUInt();
            QString verName;
            if (cv == 0x1001)       verName = QStringLiteral("版本A");
            else if (cv == 0x1002)  verName = QStringLiteral("版本B");
            else                    verName = QString("0x%1").arg(cv, 0, 16);
            text += QString("版本: %1 \n").arg(verName);
        }
        {   // 客户定制码匹配显示
            QString code = result.value("customer_code").toString();
            QString codeName;
            // "99 D5 AB 09 4A FC 0D D1 5A 36 CB A2 5A 0D 0C 65"
            if (code == "99 D5 AB 09 4A FC 0D D1 5A 36 CB A2 5A 0D 0C 65")
                codeName = QStringLiteral("ROI客户定制版本");
            else
                codeName = code;
            text += QString("客户定制码: %1 \n").arg(codeName);
        }
    }

    // --- 主头部 ---
    bool hasMainHeader = result.contains("magic");
    if (hasMainHeader) {
        text += QStringLiteral("【主头部信息】\n");
        text += QString("固件魔数: 0x%1 \n").arg(result.value("magic").toUInt(), 8, 16, QChar('0'));
        text += QString("板卡类型: %1 \n").arg(result.value("board_type").toUInt());
        text += QString("SDK版本: %1 \n").arg(result.value("sdk_version").toUInt());
        text += QString("应用版本: %1 \n").arg(result.value("app_version").toUInt());
        text += QString("QA版本: %1 \n").arg(result.value("qa_version").toUInt());

        QVariantList subImages = result.value("sub_images").toList();
        for (int i = 0; i < subImages.size(); ++i) {
            QVariantMap si = subImages[i].toMap();
            text += QString("子镜像%1: type=%2, upgrade=%3, dual=%4, flash=0x%5, subimg_offset=0x%6\n")
                        .arg(i + 1)
                        .arg(si.value("img_type").toInt())
                        .arg(si.value("upgrade").toInt())
                        .arg(si.value("dual_part").toInt())
                        .arg(si.value("flash_offset").toUInt(), 8, 16, QChar('0'))
                        .arg(si.value("subimg_offset").toUInt(), 8, 16, QChar('0'));
        }

        QString reserved = result.value("reserved").toString();
        if (!reserved.isEmpty()) {
            // 保留字段太长时截断显示（避免二进制数据导致的卡顿）
            if (reserved.length() > 20)
                text += QString("保留字段: %1 ... (共%2字节)\n\n").arg(reserved.left(20)).arg(reserved.length());
            else
                text += QString("保留字段: %1 \n\n").arg(reserved);
        }

        if (result.contains("custom")) {
            uint8_t cf = static_cast<uint8_t>(result.value("custom").toUInt());
            QString customName;
            if (cf == 0)        customName = QStringLiteral("普通固件");
            else if (cf == 1)   customName = QStringLiteral("RIO客户定制版本");
            else if (cf == 2)   customName = QStringLiteral("YXX客户定制版本");
            else                customName = QString::number(cf);
            text += QString("定制标志: %1 \n").arg(customName);
        }
        if (result.contains("chip_id")) {
            uint8_t ci = static_cast<uint8_t>(result.value("chip_id").toUInt());
            QString chipName;
            if (ci == 0)        chipName = QStringLiteral("支持芯片: CV610_20S (0x3516c613)");
            else if (ci == 0x7) chipName = QStringLiteral("支持芯片: CV610_20S, CV610_10B, CV610_00S");
            else                chipName = QString("0x%1").arg(ci, 2, 16, QChar('0'));
            text += QString("ChipId标志: %1 \n").arg(chipName);
        }
        if (result.contains("upgrade_flags")) {
            uint8_t uf = static_cast<uint8_t>(result.value("upgrade_flags").toUInt());
            QString ufName;
            if (uf == 0)        ufName = QStringLiteral("旧SDK003");
            else if(uf == 2)               ufName = QStringLiteral("新SDK020");
            text += QString("SDK标志: %1 \n").arg(ufName);
        }

        if (result.contains("crc32"))
            text += QString("CRC32: 0x%1 \n").arg(result.value("crc32").toUInt(), 8, 16, QChar('0'));
        if (result.contains("total_size"))
            text += QString("总大小: %1 \n").arg(result.value("total_size").toUInt());
        if (result.contains("build_time"))
            text += QString("编译时间: %1 \n").arg(result.value("build_time").toString());

        text += "\n";
    }

    text += QString("数据偏移: %1 字节 \n").arg(result.value("data_offset").toInt());
    text += QString("数据大小: %1 字节 \n").arg(result.value("data_size").toInt());

    if (result.contains("error"))
        text += QString("\n错误: %1 \n").arg(result.value("error").toString());

    return text;
}
