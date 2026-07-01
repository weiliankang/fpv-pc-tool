#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QStatusBar>
#include <QTabWidget>
#include <QTimer>
#include <QTranslator>

// 串口相关
class SerialCommunicator;
class SerialProtocolHandler;
class AppTranslator;
class OsdGridWidget;

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
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

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
    void onMockModeToggled(bool checked);

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
    void onUpdateFreqPreview();

    // OSD数据
    void onGetOsdData();
    void onMockOsdData();
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

private:
    void initUI();
    void loadFirmwarePage();
    void loadSerialPages();
    void loadSettingsPage();
    void setupTemplateButtons();
    void logSend(const QString &desc, const QByteArray &packet);
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

    // 逻辑
    SerialCommunicator *m_comm;
    SerialProtocolHandler *m_protocol;
    QTimer *m_autoRefreshTimer;

    // OSD
    QTimer *m_osdPollTimer;
    QByteArray m_osdAccumulated;
    QByteArray m_osdRawData;       // 累积的原始接收数据
    OsdGridWidget *m_osdGrid;
    unsigned short m_osdCharMap[20][53];  // FCOSD_MAX_HEIGHT x FCOSD_MAX_WIDTH

    // 从原始数据中扫描 OSD 数据段，直接渲染到网格
    void visualizeRawOsdData();

    // 翻译
    AppTranslator *m_translator;
    QString m_currentLang;
};

#endif // MAINWINDOW_H
