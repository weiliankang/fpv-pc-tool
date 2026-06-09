#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_page_firmware.h"
#include "ui_page_serial_connection.h"
#include "ui_page_serial_keycontrol.h"
#include "ui_page_serial_wireless.h"
#include "ui_page_serial_custom.h"
#include "ui_page_serial_history.h"

#include "serialcommunicator.h"
#include "serialprotocolhandler.h"
#include "firmwareparser.h"
#include "ui_page_settings.h"
#include "translator.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QCloseEvent>
#include <QFont>
#include <QDateTime>
#include <QTextCursor>
#include <QSerialPortInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

// 调试宏：所有槽函数入口打印
#define TRACE qDebug() << "[TRACE]" << Q_FUNC_INFO

// =====================================================================
// 构造函数 / 析构函数
// =====================================================================
MainWindow::MainWindow(QWidget *parent)
 : QMainWindow(parent), ui(new Ui::MainWindow),
   uiFirmware(nullptr), uiConn(nullptr), uiKey(nullptr),
   uiWireless(nullptr), uiCustom(nullptr), uiHistory(nullptr)
{
    TRACE;

    m_comm = new SerialCommunicator(this);
    m_protocol = new SerialProtocolHandler(this);

    m_translator = new AppTranslator(this);
    m_currentLang = QStringLiteral("zh_CN");
    connect(m_translator, &AppTranslator::languageChanged, this, &MainWindow::retranslateUi);

    initUI();

    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshPorts);
    m_autoRefreshTimer->start(2000);

    connect(m_comm, &SerialCommunicator::statusChanged, this, &MainWindow::onSerialStatusChanged);
    connect(m_comm, &SerialCommunicator::dataReceived, this, &MainWindow::onDataReceived);
}

MainWindow::~MainWindow() {
    if (m_comm->isConnected()) m_comm->disconnect();
    delete ui;
    delete uiFirmware; delete uiConn; delete uiKey;
    delete uiWireless; delete uiCustom; delete uiHistory; delete uiSettings;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    TRACE;
    if (m_comm->isConnected()) m_comm->disconnect();
    event->accept();
}

// =====================================================================
// 初始化 UI
// =====================================================================
void MainWindow::initUI() {
    TRACE;
    ui->setupUi(this);
    setFont(QFont("Microsoft YaHei", 9));

    loadFirmwarePage();
    loadSerialPages();
    loadSettingsPage();

    connect(uiSettings->comboLang, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);
    connect(ui->listSidebar, &QListWidget::currentRowChanged,
            this, &MainWindow::onSidebarChanged);

    ui->listSidebar->setCurrentRow(0);

    // 加载中文并刷新 UI
    m_translator->setLanguage("zh_CN");
    retranslateUi();
}

void MainWindow::loadFirmwarePage() {
    TRACE;
    m_pageFirmware = new QWidget();
    uiFirmware = new Ui::PageFirmware();
    uiFirmware->setupUi(m_pageFirmware);

    connect(uiFirmware->btnSkyBrowse, &QPushButton::clicked, this, &MainWindow::onSkyBrowse);
    connect(uiFirmware->btnGroundBrowse, &QPushButton::clicked, this, &MainWindow::onGroundBrowse);
    connect(uiFirmware->btnSkyParse, &QPushButton::clicked, this, &MainWindow::onSkyParse);
    connect(uiFirmware->btnGroundParse, &QPushButton::clicked, this, &MainWindow::onGroundParse);

    ui->stackContent->addWidget(m_pageFirmware);
}

void MainWindow::loadSettingsPage() {
    TRACE;
    m_pageSettings = new QWidget();
    uiSettings = new Ui::PageSettings();
    uiSettings->setupUi(m_pageSettings);
    uiSettings->comboLang->setCurrentIndex(0);
    ui->stackContent->addWidget(m_pageSettings);
}

void MainWindow::loadSerialPages() {
    TRACE;
    m_serialTabs = new QTabWidget(this);
    m_pageConn = new QWidget();
    m_pageKey = new QWidget();
    m_pageWireless = new QWidget();
    m_pageCustom = new QWidget();
    m_pageHistory = new QWidget();

    uiConn = new Ui::PageSerialConnection();
    uiKey = new Ui::PageSerialKeyControl();
    uiWireless = new Ui::PageSerialWireless();
    uiCustom = new Ui::PageSerialCustom();
    uiHistory = new Ui::PageSerialHistory();

    uiConn->setupUi(m_pageConn);
    uiKey->setupUi(m_pageKey);
    uiWireless->setupUi(m_pageWireless);
    uiCustom->setupUi(m_pageCustom);
    uiHistory->setupUi(m_pageHistory);

    // 波特率选项
    uiConn->comboBaud->addItems({"1200","2400","4800","9600","14400","19200","38400","57600","115200","230400","460800","921600"});
    uiConn->comboBaud->setCurrentText("115200");

    // 按键列表
    uiKey->comboKey->addItems(SerialProtocolHandler::keyNames());

    // 模板按钮
    setupTemplateButtons();

    // 连接信号
    connect(uiConn->btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshPorts);
    connect(uiConn->btnConnect, &QPushButton::clicked, this, &MainWindow::onToggleConnection);
    connect(uiConn->btnClearRecv, &QPushButton::clicked, uiConn->textRecv, &QTextEdit::clear);

    setupKeyButtons();
    connect(uiKey->btnSendCustomKey, &QPushButton::clicked, this, &MainWindow::onSendCustomKey);
    connect(uiKey->comboKey, &QComboBox::currentTextChanged, this, &MainWindow::onUpdateKeyPreview);
    connect(uiKey->comboPressType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdateKeyPreview);

    connect(uiWireless->btnSetFreq, &QPushButton::clicked, this, &MainWindow::onSetFreq);
    connect(uiWireless->btnGetFreq, &QPushButton::clicked, this, &MainWindow::onGetFreq);
    connect(uiWireless->btnGetStatus, &QPushButton::clicked, this, &MainWindow::onGetStatus);
    connect(uiWireless->btnGetBbPwr, &QPushButton::clicked, this, &MainWindow::onGetBbPwr);
    connect(uiWireless->btnSetBbPwr, &QPushButton::clicked, this, &MainWindow::onSetBbPwr);
    connect(uiWireless->btnGetStatusSky, &QPushButton::clicked, this, &MainWindow::onGetStatusSky);
    connect(uiWireless->btnGetDistance, &QPushButton::clicked, this, &MainWindow::onGetDistance);
    connect(uiWireless->comboBand, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdateFreqPreview);
    connect(uiWireless->spinChannel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onUpdateFreqPreview);
    connect(uiWireless->comboHop, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdateFreqPreview);

    connect(uiCustom->btnSendCustom, &QPushButton::clicked, this, &MainWindow::onSendCustom);
    connect(uiCustom->btnCheckFormat, &QPushButton::clicked, this, &MainWindow::onCheckFormat);

    connect(uiHistory->btnClearSend, &QPushButton::clicked, this, &MainWindow::onClearSend);
    connect(uiHistory->btnClearRecv, &QPushButton::clicked, this, &MainWindow::onClearRecv);
    connect(uiHistory->btnExportSend, &QPushButton::clicked, this, &MainWindow::onExportSend);
    connect(uiHistory->btnExportRecv, &QPushButton::clicked, this, &MainWindow::onExportRecv);

    // 标签页标题稍后在 retranslateUi 中设置
    m_serialTabs->addTab(m_pageConn, QString());
    m_serialTabs->addTab(m_pageKey, QString());
    m_serialTabs->addTab(m_pageWireless, QString());
    m_serialTabs->addTab(m_pageCustom, QString());
    m_serialTabs->addTab(m_pageHistory, QString());

    ui->stackContent->addWidget(m_serialTabs);

    onRefreshPorts();
    onUpdateKeyPreview();
    onUpdateFreqPreview();
}

void MainWindow::setupKeyButtons() {
    // KEY_MAPPINGS 中使用的键名
    connect(uiKey->btnUp, &QPushButton::clicked, this, [this](){ onKeyCommand("上键"); });
    connect(uiKey->btnDown, &QPushButton::clicked, this, [this](){ onKeyCommand("下键"); });
    connect(uiKey->btnLeft, &QPushButton::clicked, this, [this](){ onKeyCommand("左键"); });
    connect(uiKey->btnRight, &QPushButton::clicked, this, [this](){ onKeyCommand("右键"); });
    connect(uiKey->btnOk, &QPushButton::clicked, this, [this](){ onKeyCommand("确认键"); });
    connect(uiKey->btnBack, &QPushButton::clicked, this, [this](){ onKeyCommand("回退键"); });
    connect(uiKey->btnPair, &QPushButton::clicked, this, [this](){ onKeyCommand("配对按键"); });
    connect(uiKey->btnUpdate, &QPushButton::clicked, this, [this](){ onKeyCommand("升级按键"); });
    connect(uiKey->btnRecord, &QPushButton::clicked, this, [this](){ onKeyCommand("录像键"); });
    connect(uiKey->btnForce720p, &QPushButton::clicked, this, [this](){ onKeyCommand("强制720p60按键"); });
    connect(uiKey->btnDebug3, &QPushButton::clicked, this, [this](){ onKeyCommand("debug3模式按键"); });
}

void MainWindow::setupTemplateButtons() {
    // 清除旧按钮
    QGridLayout *grid = uiCustom->gridTemplate;
    QLayoutItem *item;
    while ((item = grid->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 按键命令模板 - 文档中的按键命令
    struct { QString name; QString cmd; } tpls[] = {
        {QStringLiteral("上键"),         "FE EF 22 00 04 40 00 00 00 00 40 0D 0A"},
        {QStringLiteral("下键"),         "FE EF 22 00 04 40 01 00 00 00 41 0D 0A"},
        {QStringLiteral("左键"),         "FE EF 22 00 04 40 02 00 00 00 42 0D 0A"},
        {QStringLiteral("右键"),         "FE EF 22 00 04 40 03 00 00 00 43 0D 0A"},
        {QStringLiteral("确认键"),       "FE EF 22 00 04 40 04 00 00 00 44 0D 0A"},
        {QStringLiteral("配对键"),       "FE EF 22 00 04 40 05 00 00 00 45 0D 0A"},
        {QStringLiteral("录制键"),       "FE EF 22 00 04 40 06 00 00 00 46 0D 0A"},
        {QStringLiteral("返回键"),       "FE EF 22 00 04 40 07 00 00 00 47 0D 0A"},
        {QStringLiteral("升级键"),       "FE EF 22 00 04 40 05 01 00 00 46 0D 0A"},
        {QStringLiteral("强制720p60"),   "FE EF 22 00 04 40 07 01 00 00 48 0D 0A"},
        // 无线命令模板
        {QStringLiteral("获取频率"),     "FE EF A2 00 08 51 00 00 00 00 00 00 00 00 51 0D 0A"},
        {QStringLiteral("获取状态"),     "FE EF A2 00 08 52 00 00 00 00 00 00 00 00 52 0D 0A"},
        {QStringLiteral("获取功率"),     "FE EF A2 00 08 53 00 00 00 00 00 00 00 00 53 0D 0A"},
        {QStringLiteral("获取天空状态"),  "FE EF A2 00 08 55 00 00 00 00 00 00 00 00 55 0D 0A"},
        {QStringLiteral("获取距离"),     "FE EF A2 00 08 56 00 00 00 00 00 00 00 00 56 0D 0A"},
    };

    for (int i = 0; i < sizeof(tpls)/sizeof(tpls[0]); ++i) {
        auto *btn = new QPushButton(tpls[i].name);
        QString cmd = tpls[i].cmd;
        connect(btn, &QPushButton::clicked, this, [this, cmd]() {
            uiCustom->editCustomInput->setPlainText(cmd);
        });
        grid->addWidget(btn, i / 3, i % 3);
    }
}

// =====================================================================
// 翻译刷新
// =====================================================================
void MainWindow::retranslateUi() {
    TRACE;

    // ====== 验证翻译系统 ======
    QString test = m_translator->tr("btnUp");
    qDebug() << "[TRACE] tr(btnUp) =" << test;
    qDebug() << "[TRACE] currentLang =" << m_translator->currentLang();
    qDebug() << "[TRACE] translator addr =" << m_translator;

    // ====== 直接设置中文（绕过翻译系统验证） ======
    uiSettings->groupLanguage->setTitle(QStringLiteral("语言"));
    uiSettings->groupAbout->setTitle(QStringLiteral("关于"));
    uiSettings->labelAboutVer->setText(QStringLiteral("版本: 1.0.0"));
    uiSettings->labelAboutDesc->setText(QStringLiteral("FPV 调试工具，用于无线系统测试和固件分析"));
    uiSettings->labelLangNote->setText(QStringLiteral("切换语言后可能需要重启才能完全生效"));
    uiSettings->lbLang->setText(QStringLiteral("语言:"));

    uiKey->groupDirection->setTitle(QStringLiteral("方向"));
    uiKey->groupFunction->setTitle(QStringLiteral("功能"));
    uiKey->groupCustomKey->setTitle(QStringLiteral("自定义按键"));
    uiKey->btnUp->setText(QStringLiteral("上"));
    uiKey->btnDown->setText(QStringLiteral("下"));
    uiKey->btnLeft->setText(QStringLiteral("左"));
    uiKey->btnRight->setText(QStringLiteral("右"));
    uiKey->btnOk->setText(QStringLiteral("确认"));
    uiKey->btnBack->setText(QStringLiteral("回退"));
    uiKey->btnPair->setText(QStringLiteral("配对"));
    uiKey->btnUpdate->setText(QStringLiteral("升级按键"));
    uiKey->btnRecord->setText(QStringLiteral("录像键"));
    uiKey->btnForce720p->setText(QStringLiteral("720p60强制"));
    uiKey->btnDebug3->setText(QStringLiteral("debug3模式"));
    uiKey->lbKeyName->setText(QStringLiteral("按键:"));
    uiKey->lbPressType->setText(QStringLiteral("按压类型:"));
    uiKey->lbPreview->setText(QStringLiteral("预览:"));
    uiKey->btnSendCustomKey->setText(QStringLiteral("发送"));
    uiKey->comboPressType->setItemText(0, QStringLiteral("单击 (0)"));
    uiKey->comboPressType->setItemText(1, QStringLiteral("长按 (1)"));

    uiWireless->groupFreq->setTitle(QStringLiteral("频率设置"));
    uiWireless->groupStatus->setTitle(QStringLiteral("状态与参数"));
    uiWireless->lbBand->setText(QStringLiteral("频段:"));
    uiWireless->lbChan->setText(QStringLiteral("频道:"));
    uiWireless->lbHop->setText(QStringLiteral("模式:"));
    uiWireless->lbFreqPrev->setText(QStringLiteral("预览:"));
    uiWireless->lbBbPwr->setText(QStringLiteral("基带功率索引:"));
    uiWireless->btnSetFreq->setText(QStringLiteral("设置频点"));
    uiWireless->btnGetFreq->setText(QStringLiteral("获取频点"));
    uiWireless->btnGetStatus->setText(QStringLiteral("获取状态 (地面 0x52)"));
    uiWireless->btnGetStatusSky->setText(QStringLiteral("获取状态 (天空 0x55)"));
    uiWireless->btnGetDistance->setText(QStringLiteral("获取距离 (0x56)"));
    uiWireless->btnGetBbPwr->setText(QStringLiteral("获取基带功率 (0x53)"));
    uiWireless->btnSetBbPwr->setText(QStringLiteral("设置基带功率 (0x54)"));
    uiWireless->comboBand->setItemText(0, QStringLiteral("频段A (0)"));
    uiWireless->comboBand->setItemText(1, QStringLiteral("频段B (1)"));
    uiWireless->comboBand->setItemText(2, QStringLiteral("频段C (2)"));
    uiWireless->comboHop->setItemText(0, QStringLiteral("定频 (0)"));
    uiWireless->comboHop->setItemText(1, QStringLiteral("跳频 (1)"));

    uiCustom->groupTemplate->setTitle(QStringLiteral("模板"));
    uiCustom->groupInput->setTitle(QStringLiteral("自定义命令输入"));
    uiCustom->groupSendCtrl->setTitle(QStringLiteral("发送控制"));
    uiCustom->lbHex->setText(QStringLiteral("十六进制 (空格分隔):"));
    uiCustom->lbCount->setText(QStringLiteral("次数:"));
    uiCustom->lbInterval->setText(QStringLiteral("间隔(毫秒):"));
    uiCustom->btnCheckFormat->setText(QStringLiteral("检查格式"));
    uiCustom->btnSendCustom->setText(QStringLiteral("发送命令"));
    uiCustom->checkRepeat->setText(QStringLiteral("重复发送"));
#if 0
    QStringList tplTexts = {
        QStringLiteral("拍照"), QStringLiteral("录像"),
        QStringLiteral("居中"), QStringLiteral("锁定"),
        QStringLiteral("解锁"), QStringLiteral("返航")
    };
    QGridLayout *grid = uiCustom->gridTemplate;
    for (int i = 0; i < grid->count() && i < tplTexts.size(); ++i) {
        QWidget *w = grid->itemAt(i)->widget();
        if (auto *btn = qobject_cast<QPushButton *>(w))
            btn->setText(tplTexts[i]);
    }
#endif
    uiHistory->groupSend->setTitle(QStringLiteral("发送历史"));
    uiHistory->groupRecv->setTitle(QStringLiteral("接收历史"));
    uiHistory->btnClearSend->setText(QStringLiteral("清空"));
    uiHistory->btnExportSend->setText(QStringLiteral("导出"));
    uiHistory->btnClearRecv->setText(QStringLiteral("清空"));
    uiHistory->btnExportRecv->setText(QStringLiteral("导出"));

    uiConn->btnRefresh->setText(QStringLiteral("刷新"));
    bool connected = m_comm->isConnected();
    uiConn->btnConnect->setText(connected ? QStringLiteral("断开") : QStringLiteral("连接"));
    uiConn->labelStatus->setText(connected ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    uiConn->btnClearRecv->setText(QStringLiteral("清空"));
    uiConn->checkAutoScroll->setText(QStringLiteral("自动滚动"));

    m_serialTabs->setTabText(0, QStringLiteral("连接"));
    m_serialTabs->setTabText(1, QStringLiteral("按键控制"));
    m_serialTabs->setTabText(2, QStringLiteral("无线参数"));
    m_serialTabs->setTabText(3, QStringLiteral("自定义命令"));
    m_serialTabs->setTabText(4, QStringLiteral("历史记录"));

    // 侧边栏
    int sc = ui->listSidebar->count();
    if (sc > 0) ui->listSidebar->item(0)->setText(QStringLiteral("固件解析"));
    if (sc > 1) ui->listSidebar->item(1)->setText(QStringLiteral("串口工具"));
    if (sc > 2) ui->listSidebar->item(2)->setText(QStringLiteral("设置"));
}

void MainWindow::setLanguage(const QString &langCode) {
    TRACE << langCode;
    if (m_currentLang == langCode) return;
    m_currentLang = langCode;
    m_translator->setLanguage(langCode);
}

// =====================================================================
// 槽函数
// =====================================================================
void MainWindow::onSidebarChanged(int index) {
    TRACE << "index=" << index;
    ui->stackContent->setCurrentIndex(index);
}

void MainWindow::onSerialStatusChanged(const QString &msg) {
    TRACE << msg;
    ui->statusbar->showMessage(msg);
}

void MainWindow::onSkyBrowse() {
    TRACE;
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择天空/中继端固件"), "",
                                                QStringLiteral("所有文件 (*)"));
    if (!path.isEmpty()) uiFirmware->editSkyPath->setText(path);
}

void MainWindow::onGroundBrowse() {
    TRACE;
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择地面端固件"), "",
                                                QStringLiteral("所有文件 (*)"));
    if (!path.isEmpty()) uiFirmware->editGroundPath->setText(path);
}

void MainWindow::onSkyParse() {
    TRACE;
    QString path = uiFirmware->editSkyPath->text();
    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("TIP"), tr("SKY_SELECT_FIRST"));
        return;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("ERROR"), tr("CANT_OPEN"));
        return;
    }
    QByteArray skyData = f.readAll();
    f.close();
    // DEBUG: dump前60字节（自定义头24+magic4+board4+sdk4+app4+qa4+subimg0的8+subimg1的8=60）
    qDebug() << "[DEBUG] 天空端固件大小:" << skyData.size() << "字节";
    qDebug() << "[DEBUG] 前152字节原始数据:";
    for (int i = 0; i < 160 && i < skyData.size(); i += 16) {
        QByteArray line = skyData.mid(i, 16);
        QString hex = QString::fromLatin1(line.toHex(' ').toUpper());
        qDebug() << qPrintable(QString("  [%1] %2").arg(i, 4, 10, QChar('0')).arg(hex));
    }
    auto result = FirmwareParser::parseSkyFirmware(skyData);
    uiFirmware->textResult->setPlainText(FirmwareParser::formatResult(result, tr("天空/中继固件")));
}

void MainWindow::onGroundParse() {
    TRACE;
    QString path = uiFirmware->editGroundPath->text();
    if (path.isEmpty()) {
        QMessageBox::warning(this, tr("TIP"), tr("GND_SELECT_FIRST"));
        return;
    }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("ERROR"), tr("CANT_OPEN"));
        return;
    }
    auto result = FirmwareParser::parseGroundFirmware(f.readAll());
    f.close();
    uiFirmware->textResult->setPlainText(FirmwareParser::formatResult(result, tr("地面端固件")));
}

void MainWindow::onRefreshPorts() {
    QString current = uiConn->comboPort->currentText();
    uiConn->comboPort->clear();
    for (const auto &info : QSerialPortInfo::availablePorts())
        uiConn->comboPort->addItem(info.portName());
    if (!current.isEmpty()) {
        int idx = uiConn->comboPort->findText(current);
        if (idx >= 0) uiConn->comboPort->setCurrentIndex(idx);
    }
}

void MainWindow::onToggleConnection() {
    TRACE;
    if (m_comm->isConnected()) {
        m_comm->disconnect();
        retranslateUi();
    } else {
        QString port = uiConn->comboPort->currentText();
        if (port.isEmpty()) {
            QMessageBox::warning(this, tr("TIP"), tr("SELECT_PORT_FIRST"));
            return;
        }
        int baud = uiConn->comboBaud->currentText().toInt();
        if (m_comm->connectToPort(port, baud))
            retranslateUi();
    }
}

void MainWindow::onKeyCommand(const QString &name) {
    TRACE << "key=" << name;
    if (checkSerialConnected("keyCommand")) return;

    int pressType = uiKey->comboPressType->currentIndex();
    QByteArray packet = m_protocol->createKeyCommand(name, pressType);
    logSend(QString("Key:%1 type:%2").arg(name).arg(pressType), packet);
    m_comm->sendData(packet);
}

void MainWindow::onSendCustomKey() {
    TRACE;
    if (checkSerialConnected("sendCustomKey")) return;

    QString keyName = uiKey->comboKey->currentText();
    int pressType = uiKey->comboPressType->currentIndex();
    QByteArray packet = m_protocol->createKeyCommand(keyName, pressType);
    logSend(QString("CustomKey:%1 type:%2").arg(keyName).arg(pressType), packet);
    m_comm->sendData(packet);
}

void MainWindow::onUpdateKeyPreview() {
    TRACE;
    QString keyName = uiKey->comboKey->currentText();
    int pressType = uiKey->comboPressType->currentIndex();
    QByteArray packet = m_protocol->createKeyCommand(keyName, pressType);
    uiKey->editKeyPreview->setText(packet.toHex(' ').toUpper());
}

void MainWindow::onSetFreq() {
    TRACE;
    if (checkSerialConnected("setFreq")) return;
    int band = uiWireless->comboBand->currentIndex();
    int channel = uiWireless->spinChannel->value();
    int hop = uiWireless->comboHop->currentIndex();
    QByteArray values;
    values.append(static_cast<char>(band));
    values.append(static_cast<char>(channel));
    values.append(static_cast<char>(hop));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_CHANNEL_HOP, values);
    logSend(QString("SetFreq band=%1 ch=%2 hop=%3").arg(band).arg(channel).arg(hop), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetFreq() {
    TRACE;
    if (checkSerialConnected("getFreq")) return;
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_CHANNEL_HOP, QByteArray());
    logSend("GetFreq", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetStatus() {
    TRACE;
    if (checkSerialConnected("getStatus")) return;
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_STATUS, QByteArray());
    logSend("GetStatus(Gnd)", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetBbPwr() {
    TRACE;
    if (checkSerialConnected("getBbPwr")) return;
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_BB_PWR, QByteArray());
    logSend("GetBbPwr", packet);
    m_comm->sendData(packet);
}

void MainWindow::onSetBbPwr() {
    TRACE;
    if (checkSerialConnected("setBbPwr")) return;
    int idx = uiWireless->comboBbPwr->currentText().toInt();
    QByteArray values;
    values.append(static_cast<char>(idx));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_BB_PWR, values);
    logSend(QString("SetBbPwr idx=%1").arg(idx), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetStatusSky() {
    TRACE;
    if (checkSerialConnected("getStatusSky")) return;
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_STATUS_SKY, QByteArray());
    logSend("GetStatus(Sky)", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetDistance() {
    TRACE;
    if (checkSerialConnected("getDistance")) return;
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_DISTANCE, QByteArray());
    logSend("GetDistance", packet);
    m_comm->sendData(packet);
}

void MainWindow::onUpdateFreqPreview() {
    TRACE;
    int band = uiWireless->comboBand->currentIndex();
    int channel = uiWireless->spinChannel->value();
    int hop = uiWireless->comboHop->currentIndex();
    QByteArray values;
    values.append(static_cast<char>(band));
    values.append(static_cast<char>(channel));
    values.append(static_cast<char>(hop));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_CHANNEL_HOP, values);
    uiWireless->editFreqPreview->setText(packet.toHex(' ').toUpper());
}

void MainWindow::onCheckFormat() {
    TRACE;
    QString input = uiCustom->editCustomInput->toPlainText().trimmed();
    QString hex = input.simplified().remove(' ');
    if (hex.isEmpty() || hex.length() % 2 != 0) {
        QMessageBox::warning(this, tr("TIP"), tr("INVALID_HEX"));
        return;
    }
    QByteArray data = QByteArray::fromHex(hex.toLatin1());
    QString formatted;
    for (int i = 0; i < data.size(); ++i)
        formatted += QString("%1 ").arg((unsigned char)data[i], 2, 16, QChar('0'));
    uiCustom->editCustomInput->setPlainText(formatted.trimmed().toUpper());
    QMessageBox::information(this, tr("TIP"), tr("FORMAT_SUCCESS"));
}

void MainWindow::onSendCustom() {
    TRACE;
    if (checkSerialConnected("sendCustom")) return;
    QString input = uiCustom->editCustomInput->toPlainText().trimmed();
    QString hex = input.simplified().remove(' ');
    QByteArray data = QByteArray::fromHex(hex.toLatin1());
    if (data.isEmpty()) {
        QMessageBox::warning(this, tr("TIP"), tr("EMPTY_CMD"));
        return;
    }
    logSend("Custom", data);
    m_comm->sendData(data);
}

void MainWindow::onClearSend() {
    TRACE;
    uiHistory->textSendHistory->clear();
}
void MainWindow::onClearRecv() {
    TRACE;
    uiHistory->textRecvHistory->clear();
}
void MainWindow::onExportSend() {
    TRACE;
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("导出发送历史"), "send_log.txt", QStringLiteral("文本文件 (*.txt)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(uiHistory->textSendHistory->toPlainText().toUtf8());
}
void MainWindow::onExportRecv() {
    TRACE;
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("导出接收历史"), "recv_log.txt", QStringLiteral("文本文件 (*.txt)"));
    if (path.isEmpty()) return;
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text))
        f.write(uiHistory->textRecvHistory->toPlainText().toUtf8());
}

void MainWindow::onDataReceived(const QByteArray &data) {
    TRACE << "size=" << data.size();
    uiHistory->textRecvHistory->append(
        QString("[%1] RX: %2")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
            .arg(QString::fromLatin1(data.toHex(' ').toUpper()))
    );
    // 自动滚动
    if (uiConn->checkAutoScroll->isChecked()) {
        QTextCursor c = uiHistory->textRecvHistory->textCursor();
        c.movePosition(QTextCursor::End);
        uiHistory->textRecvHistory->setTextCursor(c);
    }

    // 解析数据包，如果是无线查询响应则显示到 textStatus
    QVariantMap parsed = m_protocol->parsePacket(data);
    if (parsed.isEmpty()) return;

    quint8 cmd = static_cast<quint8>(parsed.value("command").toUInt());
    // 无线命令响应的 cmd 为 0xA2
    if (cmd != 0xA2) return;

    QByteArray content = parsed.value("data_content").toByteArray();
    if (content.isEmpty()) return;

    quint8 subCmd = static_cast<quint8>(content.at(0));

    // ---- 显示无线参数响应到 textStatus ----
    QString displayText;
    displayText += QString("命令: 0x%1 响应\n").arg(subCmd, 2, 16, QChar('0'));
    displayText += QString("原始数据: %1\n").arg(QString::fromLatin1(content.toHex(' ').toUpper()));

    switch (subCmd) {
    case 0x51: // 获取频点响应
        if (content.size() >= 4) {
            int band    = static_cast<quint8>(content.at(1));
            int channel = static_cast<quint8>(content.at(2));
            int hop     = static_cast<quint8>(content.at(3));
            displayText += QString("频段: %1 (%2)\n").arg(band).arg(bandToLetter(band));
            displayText += QString("信道: %1 (chn%2)\n").arg(channel).arg(channel+1);
            displayText += QString("跳频模式: %1\n").arg(hop ? QStringLiteral("跳频") : QStringLiteral("定频"));
        }
        break;
    case 0x52: // 获取地面端无线状态
        if (content.size() >= 6) {
            int rssi1 = static_cast<signed char>(content.at(1));
            int rssi2 = static_cast<signed char>(content.at(2));
            int rate  = static_cast<quint8>(content.at(3));
            int delay = static_cast<quint8>(content.at(4));
            int conn  = static_cast<quint8>(content.at(5));
            displayText += QString("RSSI1: %1\n").arg(rssi1);
            displayText += QString("RSSI2: %1\n").arg(rssi2);
            displayText += QString("码率: %1 Mbps\n").arg(rate);
            displayText += QString("延迟: %1 ms\n").arg(delay);
            displayText += QString("连接状态: %1\n").arg(conn ? QStringLiteral("已连接") : QStringLiteral("未连接"));
        }
        break;
    case 0x53: // 获取基带功率响应
        if (content.size() >= 5) {
            int pwrIdx = static_cast<quint8>(content.at(1));
            quint32 bitmap = (static_cast<quint8>(content.at(2))) |
                             (static_cast<quint8>(content.at(3)) << 8) |
                             (static_cast<quint8>(content.at(4)) << 16) |
                             (static_cast<quint8>(content.at(5)) << 24);
            displayText += QString("当前功率索引: %1\n").arg(pwrIdx);
            displayText += QString("可设置位图: 0x%1\n").arg(bitmap, 8, 16, QChar('0'));

            // 更新功率索引下拉框
            QComboBox *combo = uiWireless->comboBbPwr;
            combo->blockSignals(true);
            combo->clear();
            QStringList settable;
            for (int i = 0; i < 32; ++i) {
                if (bitmap & (1U << i)) {
                    combo->addItem(QString::number(i));
                    settable << QString::number(i);
                }
            }
            combo->blockSignals(false);
            // 默认选中当前功率索引
            int curIdx = combo->findText(QString::number(pwrIdx));
            if (curIdx >= 0) combo->setCurrentIndex(curIdx);

            if (!settable.isEmpty())
                displayText += QString("可设置功率: %1\n").arg(settable.join(", "));
        }
        break;
    case 0x55: // 获取天空端无线状态
        if (content.size() >= 3) {
            int rssi1 = static_cast<signed char>(content.at(1));
            int rssi2 = static_cast<signed char>(content.at(2));
            displayText += QString("RSSI1: %1\n").arg(rssi1);
            displayText += QString("RSSI2: %1\n").arg(rssi2);
        }
        break;
    case 0x56: // 获取距离 (小端32位)
        if (content.size() >= 5) {
            quint32 dist = static_cast<quint8>(content.at(1)) |
                          (static_cast<quint8>(content.at(2)) << 8) |
                          (static_cast<quint8>(content.at(3)) << 16) |
                          (static_cast<quint8>(content.at(4)) << 24);
            displayText += QString("距离: %1 m\n").arg(dist);
        }
        break;
    default:
        displayText += QStringLiteral("未知命令类型\n");
        break;
    }

    uiWireless->textStatus->append(displayText);
    // 自动滚动到最新
    QTextCursor c = uiWireless->textStatus->textCursor();
    c.movePosition(QTextCursor::End);
    uiWireless->textStatus->setTextCursor(c);
}

void MainWindow::onLanguageChanged(int index) {
    TRACE << "index=" << index;
    setLanguage(index == 0 ? "zh_CN" : "en");
}

// =====================================================================
// 辅助函数
// =====================================================================
void MainWindow::logSend(const QString &desc, const QByteArray &packet) {
    QString line = QString("[%1] TX [%2]: %3")
                       .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                       .arg(desc)
                       .arg(QString::fromLatin1(packet.toHex(' ').toUpper()));
    uiHistory->textSendHistory->append(line);
}

void MainWindow::updateSerialPageStates() {
    // 如果串口断开，禁用相关控件
    bool connected = m_comm->isConnected();
    uiKey->groupDirection->setEnabled(connected);
    uiKey->groupFunction->setEnabled(connected);
    uiKey->groupCustomKey->setEnabled(connected);
    uiWireless->groupFreq->setEnabled(connected);
    uiWireless->groupStatus->setEnabled(connected);
    uiCustom->groupInput->setEnabled(connected);
    uiCustom->groupSendCtrl->setEnabled(connected);
    uiCustom->groupTemplate->setEnabled(connected);
}

bool MainWindow::checkSerialConnected(const QString &actionName) {
    if (!m_comm->isConnected()) {
        TRACE << "BLOCKED:" << actionName << "- not connected";
        QMessageBox::warning(this, tr("TIP"), tr("NOT_CONNECTED"));
        return true;
    }
    return false;
}

QString MainWindow::bandToLetter(int band) {
    switch (band) {
    case 0: return QStringLiteral("BandA");
    case 1: return QStringLiteral("BandB");
    case 2: return QStringLiteral("BandC");
    case 3: return QStringLiteral("BandD");
    case 4: return QStringLiteral("BandE");
    case 5: return QStringLiteral("BandF");
    default: return QStringLiteral("Band%1").arg(band);
    }
}
