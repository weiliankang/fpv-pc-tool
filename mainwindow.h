#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QTranslator>
#include "serialprotocolhandler.h"

// 串口相关
class SerialCommunicator;
class SerialProtocolHandler;
class AppTranslator;
class OsdGridWidget;
class CrsfTestPage;
class SbusTestPage;
class MavlinkTestPage;
class F0ReadPage;
class QScrollArea;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QProcess;

// UI namespace (from .ui files)
namespace Ui {
class MainWindow;
class PageFirmware;
class PageSerialConnection;
class PageSerialKeyControl;
class PageSerialWireless;
class PageSerialCustom;
class PageSerialHistory;
class PageSerialOsd;
class PageSettings;
class PageSerialF0Read;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

private slots:
    void onSidebarChanged(int index);
    void onSerialStatusChanged(const QString &msg);

    // 固件解析
    void onSkyBrowse();
    void onGroundBrowse();
    void onSkyParse();
    void onGroundParse();

    // 串口连接
    void onRefreshPorts();
    void onToggleConnection();

    // 按键控制
    void onKeyCommand(const QString &name);
    void onSendCustomKey();
    void onUpdateKeyPreview();
    void setupKeyButtons();

    // 无线参数
    void onSetFreq();
    void onGetFreq();
    void onGetStatus();
    void onGetBbPwr();
    void onSetBbPwr();
    void onGetStatusSky();
    void onGetDistance();
    void previewWirelessCmd(wireless_data_type_t type, const QByteArray &values, const QString &name);
    void onUpdatePreviewFromParams();

    // 中继(Relay)无线参数
    void onSetRelayFreq();
    void onGetRelayFreq();
    void onGetRelayStatus();
    void onGetRelayBbPwr();
    void onSetRelayBbPwr();
    void onGetRelayGndDist();
    void onGetRelaySkyDist();

    // ---- 无线参数页 0x57 OSD 获取 ----
    void onGetOsdDataWireless();
    void onClearOsdDataWireless();

    // OSD数据
    void onGetOsdData();
    void onClearOsd();
    void onFcTypeChanged(int index);
    void onAutoPollToggled(bool checked);
    void onOsdResolutionChanged(int index);
    void onPollTimer();
    void onOsdDataReceived(const QByteArray &dataContent);

    // 自定义命令
    void onCheckFormat();
    void onSendCustom();

    // 历史记录
    void onClearSend();
    void onClearRecv();
    void onExportSend();
    void onExportRecv();

    // 串口数据
    void onDataReceived(const QByteArray &data);

    // 设置
    void onLanguageChanged(int index);

    // ---- 打包工具 ----
    void onPkgBrowseExe();
    void onPkgBrowseDir();
    void onPkgBrowseWindeployqt();
    void onPkgRun();
    void onPkgOpenFolder();
    void onPkgOutput();
    void onPkgFinished();

private:
    void initUI();
    void buildPackagePage();
    void loadFirmwarePage();
    void loadSerialPages();
    void loadSettingsPage();
    void setupTemplateButtons();
    void logSend(const QString &desc, const QByteArray &packet);
    QScrollArea *makeScrollable(QWidget *page);
    void updateSerialPageStates();
    bool checkSerialConnected(const QString &actionName);
    bool loadOsdFont(const QString &fontName, int fontPixel);
    static QString bandToLetter(int band);

    void retranslateUi();
    void setLanguage(const QString &langCode);

    // UI
    Ui::MainWindow *ui;
    Ui::PageFirmware *uiFirmware;
    Ui::PageSerialConnection *uiConn;
    Ui::PageSerialKeyControl *uiKey;
    Ui::PageSerialWireless *uiWireless;
    Ui::PageSerialCustom *uiCustom;
    Ui::PageSerialHistory *uiHistory;
    Ui::PageSerialOsd *uiOsd;
    Ui::PageSettings *uiSettings;

    // 页面 widgets
    QWidget *m_pageFirmware;
    QWidget *m_pageConn;
    QWidget *m_pageKey;
    QWidget *m_pageWireless;
    QWidget *m_pageCustom;
    QWidget *m_pageHistory;
    QWidget *m_pageOsd;
    QWidget *m_pageSettings;
    QTabWidget *m_serialTabs;

    // ---- 打包工具 ----
    QWidget *m_pagePackage;
    QLineEdit *m_pkgExeEdit;
    QLineEdit *m_pkgDirEdit;
    QLineEdit *m_pkgWindeployqtEdit;
    QPlainTextEdit *m_pkgLog;
    QPushButton *m_pkgRunBtn;
    QPushButton *m_pkgOpenBtn;
    QProcess *m_pkgProc = nullptr;
    QString m_pkgOutputDir;   // 本次打包的输出目录（供"打开文件夹"使用）

    // 逻辑
    SerialCommunicator *m_comm;
    SerialProtocolHandler *m_protocol;
    QTimer *m_autoRefreshTimer;
    int m_previewType = -1;   // 当前预览的命令类型，-1 = 无

    // CRSF 测试
    CrsfTestPage *m_crsfPage = nullptr;

    // SBUS 测试
    SbusTestPage *m_sbusPage = nullptr;

    // MAVLink 测试
    MavlinkTestPage *m_mavlinkPage = nullptr;

    // F0 读取测试
    F0ReadPage *m_f0readPage = nullptr;

    // OSD
    QTimer *m_osdPollTimer;
    QByteArray m_osdRawData;       // 累积的原始接收数据
    QByteArray m_osdReassemblyBuf; // MSP分包重组缓冲区
    int m_osdExpectedLen = 0;      // 期望接收的MSP payload总长度（不含头部）
    OsdGridWidget *m_osdGrid;
    unsigned short m_osdCharMap[20][53];  // 20行 x 53列
    QByteArray m_wirelessBuf;      // 无线查询/设置响应分包重组缓冲区（解决响应被串口拆包导致首点无响应）
    bool m_wirelessSuppress = false; // true=快捷指令(F0)接收期间，抑制无线响应打印到 textStatus（仍更新组合框）

    // 从原始数据中扫描 OSD 数据段，直接渲染到网格
    void visualizeRawOsdData();

    // 无线参数页用户主动操作时调用：解除快捷指令抑制并清空无线历史缓冲
    void resetWirelessDisplay();

    // 翻译
    AppTranslator *m_translator;
    QString m_currentLang;
};

#endif // MAINWINDOW_H


