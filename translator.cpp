#include "translator.h"

AppTranslator::AppTranslator(QObject *parent)
    : QObject(parent), m_currentLang("zh_CN")
{
    initZh();
    initEn();
    m_initialized = true;
}

bool AppTranslator::setLanguage(const QString &langCode)
{
    m_currentLang = langCode;
    emit languageChanged(langCode);
    return true;
}

QString AppTranslator::tr(const QString &key) const
{
    if (!m_initialized) return key;
    const auto &dict = (m_currentLang == "zh_CN") ? m_zh : m_en;
    auto it = dict.constFind(key);
    if (it != dict.constEnd()) return it.value();
    return key;
}

void AppTranslator::initZh()
{
    m_zh["groupLanguage"] = QStringLiteral("语言");
    m_zh["groupAbout"] = QStringLiteral("关于");
    m_zh["labelAboutVer"] = QStringLiteral("版本: 1.0.0");
    m_zh["labelAboutDesc"] = QStringLiteral("FPV 调试工具，用于无线系统测试和固件分析");
    m_zh["labelLangNote"] = QStringLiteral("切换语言后可能需要重启才能完全生效");
    m_zh["lbLang"] = QStringLiteral("语言:");
    m_zh["firmware"] = QStringLiteral("固件解析");
    m_zh["serial"] = QStringLiteral("串口工具");
    m_zh["settings"] = QStringLiteral("设置");
    m_zh["tabConn"] = QStringLiteral("连接");
    m_zh["tabKey"] = QStringLiteral("按键控制");
    m_zh["tabWireless"] = QStringLiteral("无线参数");
    m_zh["tabCustom"] = QStringLiteral("自定义命令");
    m_zh["tabHistory"] = QStringLiteral("历史记录");
    m_zh["btnRefresh"] = QStringLiteral("刷新");
    m_zh["btnConnect"] = QStringLiteral("连接");
    m_zh["btnDisconnect"] = QStringLiteral("断开");
    m_zh["btnClearRecv"] = QStringLiteral("清空");
    m_zh["checkAutoScroll"] = QStringLiteral("自动滚动");
    m_zh["labelStatus"] = QStringLiteral("未连接");
    m_zh["labelStatusConn"] = QStringLiteral("已连接");
    m_zh["groupDirection"] = QStringLiteral("方向");
    m_zh["groupFunction"] = QStringLiteral("功能");
    m_zh["groupCustomKey"] = QStringLiteral("自定义按键");
    m_zh["btnUp"] = QStringLiteral("上");
    m_zh["btnDown"] = QStringLiteral("下");
    m_zh["btnLeft"] = QStringLiteral("左");
    m_zh["btnRight"] = QStringLiteral("右");
    m_zh["btnOk"] = QStringLiteral("确认");
    m_zh["btnBack"] = QStringLiteral("返回");
    m_zh["btnPair"] = QStringLiteral("配对");
    m_zh["btnUpdate"] = QStringLiteral("升级");
    m_zh["btnRecord"] = QStringLiteral("录像");
    m_zh["btnForce720p"] = QStringLiteral("720p60");
    m_zh["btnDebug3"] = QStringLiteral("调试3");
    m_zh["lbKeyName"] = QStringLiteral("按键:");
    m_zh["lbPressType"] = QStringLiteral("按压类型:");
    m_zh["lbPreview"] = QStringLiteral("预览:");
    m_zh["btnSendCustomKey"] = QStringLiteral("发送");
    m_zh["pressType0"] = QStringLiteral("单击 (0)");
    m_zh["pressType1"] = QStringLiteral("长按 (1)");
    m_zh["groupFreq"] = QStringLiteral("频率设置");
    m_zh["groupStatus"] = QStringLiteral("状态与参数");
    m_zh["lbBand"] = QStringLiteral("频段:");
    m_zh["lbChan"] = QStringLiteral("频道:");
    m_zh["lbHop"] = QStringLiteral("模式:");
    m_zh["lbFreqPrev"] = QStringLiteral("预览:");
    m_zh["lbBbPwr"] = QStringLiteral("基带功率索引:");
    m_zh["btnSetFreq"] = QStringLiteral("设置频点");
    m_zh["btnGetFreq"] = QStringLiteral("获取频点");
    m_zh["btnGetStatus"] = QStringLiteral("获取状态 (地面 0x52)");
    m_zh["btnGetStatusSky"] = QStringLiteral("获取状态 (天空 0x55)");
    m_zh["btnGetDistance"] = QStringLiteral("获取距离 (0x56)");
    m_zh["btnGetBbPwr"] = QStringLiteral("获取基带功率 (0x53)");
    m_zh["btnSetBbPwr"] = QStringLiteral("设置基带功率 (0x54)");
    m_zh["band0"] = QStringLiteral("频段A (0)");
    m_zh["band1"] = QStringLiteral("频段B (1)");
    m_zh["band2"] = QStringLiteral("频段C (2)");
    m_zh["hop0"] = QStringLiteral("定频 (0)");
    m_zh["hop1"] = QStringLiteral("跳频 (1)");
    m_zh["groupTemplate"] = QStringLiteral("模板");
    m_zh["groupInput"] = QStringLiteral("自定义命令输入");
    m_zh["groupSendCtrl"] = QStringLiteral("发送控制");
    m_zh["lbHex"] = QStringLiteral("十六进制 (空格分隔):");
    m_zh["lbCount"] = QStringLiteral("次数:");
    m_zh["lbInterval"] = QStringLiteral("间隔(毫秒):");
    m_zh["btnCheckFormat"] = QStringLiteral("检查格式");
    m_zh["btnSendCustom"] = QStringLiteral("发送命令");
    m_zh["checkRepeat"] = QStringLiteral("重复发送");
    m_zh["tplPhoto"] = QStringLiteral("拍照");
    m_zh["tplRecord"] = QStringLiteral("录像");
    m_zh["tplCenter"] = QStringLiteral("居中");
    m_zh["tplLock"] = QStringLiteral("锁定");
    m_zh["tplUnlock"] = QStringLiteral("解锁");
    m_zh["tplRth"] = QStringLiteral("返航");
    m_zh["groupSend"] = QStringLiteral("发送历史");
    m_zh["groupRecv"] = QStringLiteral("接收历史");
    m_zh["btnClearSend"] = QStringLiteral("清空");
    m_zh["btnExportSend"] = QStringLiteral("导出");
    m_zh["btnClearRecv"] = QStringLiteral("清空");
    m_zh["btnExportRecv"] = QStringLiteral("导出");
    m_zh["firmwareFilter"] = QStringLiteral("固件文件 (*.bin *.fw *.img);;所有文件 (*)");
    m_zh["skyBrowse"] = QStringLiteral("选择天空端固件");
    m_zh["gndBrowse"] = QStringLiteral("选择地面端固件");
}

void AppTranslator::initEn()
{
    m_en["groupLanguage"] = QStringLiteral("Language");
    m_en["groupAbout"] = QStringLiteral("About");
    m_en["labelAboutVer"] = QStringLiteral("Version: 1.0.0");
    m_en["labelAboutDesc"] = QStringLiteral("FPV Debug Tool for wireless testing and firmware analysis");
    m_en["labelLangNote"] = QStringLiteral("Restart may be required to fully apply language change");
    m_en["lbLang"] = QStringLiteral("Language:");
    m_en["firmware"] = QStringLiteral("Firmware");
    m_en["serial"] = QStringLiteral("Serial");
    m_en["settings"] = QStringLiteral("Settings");
    m_en["tabConn"] = QStringLiteral("Connection");
    m_en["tabKey"] = QStringLiteral("Key Control");
    m_en["tabWireless"] = QStringLiteral("Wireless");
    m_en["tabCustom"] = QStringLiteral("Custom");
    m_en["tabHistory"] = QStringLiteral("History");
    m_en["btnRefresh"] = QStringLiteral("Refresh");
    m_en["btnConnect"] = QStringLiteral("Connect");
    m_en["btnDisconnect"] = QStringLiteral("Disconnect");
    m_en["btnClearRecv"] = QStringLiteral("Clear");
    m_en["checkAutoScroll"] = QStringLiteral("Auto Scroll");
    m_en["labelStatus"] = QStringLiteral("Disconnected");
    m_en["labelStatusConn"] = QStringLiteral("Connected");
    m_en["groupDirection"] = QStringLiteral("Direction");
    m_en["groupFunction"] = QStringLiteral("Functions");
    m_en["groupCustomKey"] = QStringLiteral("Custom Key");
    m_en["btnUp"] = QStringLiteral("UP");
    m_en["btnDown"] = QStringLiteral("DOWN");
    m_en["btnLeft"] = QStringLiteral("LEFT");
    m_en["btnRight"] = QStringLiteral("RIGHT");
    m_en["btnOk"] = QStringLiteral("OK");
    m_en["btnBack"] = QStringLiteral("BACK");
    m_en["btnPair"] = QStringLiteral("Pair");
    m_en["btnUpdate"] = QStringLiteral("Update");
    m_en["btnRecord"] = QStringLiteral("Record");
    m_en["btnForce720p"] = QStringLiteral("720p60");
    m_en["btnDebug3"] = QStringLiteral("Debug3");
    m_en["lbKeyName"] = QStringLiteral("Key:");
    m_en["lbPressType"] = QStringLiteral("Press Type:");
    m_en["lbPreview"] = QStringLiteral("Preview:");
    m_en["btnSendCustomKey"] = QStringLiteral("Send");
    m_en["pressType0"] = QStringLiteral("Click (0)");
    m_en["pressType1"] = QStringLiteral("Long (1)");
    m_en["groupFreq"] = QStringLiteral("Frequency");
    m_en["groupStatus"] = QStringLiteral("Status && Parameters");
    m_en["lbBand"] = QStringLiteral("Band:");
    m_en["lbChan"] = QStringLiteral("Channel:");
    m_en["lbHop"] = QStringLiteral("Mode:");
    m_en["lbFreqPrev"] = QStringLiteral("Preview:");
    m_en["lbBbPwr"] = QStringLiteral("BB Pwr idx:");
    m_en["btnSetFreq"] = QStringLiteral("Set Freq");
    m_en["btnGetFreq"] = QStringLiteral("Get Freq");
    m_en["btnGetStatus"] = QStringLiteral("Get Status (Gnd 0x52)");
    m_en["btnGetStatusSky"] = QStringLiteral("Get Status (Sky 0x55)");
    m_en["btnGetDistance"] = QStringLiteral("Get Distance (0x56)");
    m_en["btnGetBbPwr"] = QStringLiteral("Get BB Pwr (0x53)");
    m_en["btnSetBbPwr"] = QStringLiteral("Set BB Pwr (0x54)");
    m_en["band0"] = QStringLiteral("BandA (0)");
    m_en["band1"] = QStringLiteral("BandB (1)");
    m_en["band2"] = QStringLiteral("BandC (2)");
    m_en["hop0"] = QStringLiteral("Fixed (0)");
    m_en["hop1"] = QStringLiteral("Hopping (1)");
    m_en["groupTemplate"] = QStringLiteral("Templates");
    m_en["groupInput"] = QStringLiteral("Custom Command Input");
    m_en["groupSendCtrl"] = QStringLiteral("Send Control");
    m_en["lbHex"] = QStringLiteral("Hex (space separated):");
    m_en["lbCount"] = QStringLiteral("Count:");
    m_en["lbInterval"] = QStringLiteral("Interval(ms):");
    m_en["btnCheckFormat"] = QStringLiteral("Check Format");
    m_en["btnSendCustom"] = QStringLiteral("Send Command");
    m_en["checkRepeat"] = QStringLiteral("Repeat");
    m_en["tplPhoto"] = QStringLiteral("Photo");
    m_en["tplRecord"] = QStringLiteral("Record");
    m_en["tplCenter"] = QStringLiteral("Center");
    m_en["tplLock"] = QStringLiteral("Lock");
    m_en["tplUnlock"] = QStringLiteral("Unlock");
    m_en["tplRth"] = QStringLiteral("RTH");
    m_en["groupSend"] = QStringLiteral("Send History");
    m_en["groupRecv"] = QStringLiteral("Receive History");
    m_en["btnClearSend"] = QStringLiteral("Clear");
    m_en["btnExportSend"] = QStringLiteral("Export");
    m_en["btnClearRecv"] = QStringLiteral("Clear");
    m_en["btnExportRecv"] = QStringLiteral("Export");
    m_en["firmwareFilter"] = QStringLiteral("Firmware files (*.bin *.fw *.img);;All files (*)");
    m_en["skyBrowse"] = QStringLiteral("Select Sky Firmware");
    m_en["gndBrowse"] = QStringLiteral("Select Ground Firmware");
}
