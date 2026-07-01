#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_page_firmware.h"
#include "ui_page_serial_connection.h"
#include "ui_page_serial_keycontrol.h"
#include "ui_page_serial_wireless.h"
#include "ui_page_serial_custom.h"
#include "ui_page_serial_history.h"
#include "ui_page_serial_osd.h"

#include "serialcommunicator.h"
#include "serialprotocolhandler.h"
#include "firmwareparser.h"
#include "osdgridwidget.h"
#include "ui_page_settings.h"
#include "translator.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QVBoxLayout>
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
   uiWireless(nullptr), uiCustom(nullptr), uiHistory(nullptr),
   uiOsd(nullptr)
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
    if (m_osdPollTimer) {
        m_osdPollTimer->stop();
        delete m_osdPollTimer;
        m_osdPollTimer = nullptr;
    }
    delete ui;
    delete uiFirmware; delete uiConn; delete uiKey;
    delete uiWireless; delete uiCustom; delete uiHistory; delete uiOsd; delete uiSettings;
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
    connect(uiConn->checkMockMode, &QCheckBox::toggled, this, &MainWindow::onMockModeToggled);

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

    // ---- OSD 页面 ----
    m_pageOsd = new QWidget();
    uiOsd = new Ui::PageSerialOsd();
    uiOsd->setupUi(m_pageOsd);

    // 替换占位的 widgetOsdCanvas 为 OsdGridWidget
    m_osdGrid = new OsdGridWidget(uiOsd->widgetOsdCanvas);
    QVBoxLayout *gridLayout = new QVBoxLayout(uiOsd->widgetOsdCanvas);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addWidget(m_osdGrid);

    // 自动加载 OSD 字体（从项目内部 osdChars 目录加载）
    bool fontLoaded = false;
    // 策略1: Qt 资源文件（编译时嵌入，最可靠）
    if (QFile::exists(":/osdchars/0.png")) {
        qDebug() << "[OSD] 从 Qt 资源加载 osdChars";
        fontLoaded = m_osdGrid->loadCharImages(":/osdchars", 256);
    }
    // 策略2: 源码目录下的 osdChars（开发模式）
    if (!fontLoaded) {
        QString fontDir = QFileInfo(__FILE__).absolutePath() + "/osdChars";
        if (QDir(fontDir).exists()) {
            qDebug() << "[OSD] 从源码目录加载:" << fontDir;
            fontLoaded = m_osdGrid->loadCharImages(fontDir, 256);
        }
    }
    // 策略3: exe 同级目录（发布模式）
    if (!fontLoaded) {
        QString fontDir = QCoreApplication::applicationDirPath() + "/osdChars";
        if (QDir(fontDir).exists()) {
            qDebug() << "[OSD] 从 exe 目录加载:" << fontDir;
            fontLoaded = m_osdGrid->loadCharImages(fontDir, 256);
        }
    }
    if (!fontLoaded) {
        qWarning() << "[OSD] 找不到 osdChars，使用 QPainter 回退模式";
    }

    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));

    connect(uiOsd->btnGetOsdData, &QPushButton::clicked, this, &MainWindow::onGetOsdData);
    connect(uiOsd->btnMockOsdData, &QPushButton::clicked, this, &MainWindow::onMockOsdData);
    connect(uiOsd->btnClearOsd, &QPushButton::clicked, this, &MainWindow::onClearOsd);
    connect(uiOsd->checkAutoPoll, &QCheckBox::toggled, this, &MainWindow::onAutoPollToggled);
    connect(uiOsd->cboFcType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFcTypeChanged);
    connect(uiOsd->cboOsdResolution, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onOsdResolutionChanged);
    // spinCellSize 现在用于控制无字体模式下的像素大小（已移除 setCellSize）
    connect(m_osdGrid, &OsdGridWidget::cellHovered,
            this, [this](int row, int col, unsigned short val) {
                uiOsd->textOsdInfo->setText(
                    QString("行:%1  列:%2  字符索引:%3 (0x%4)")
                        .arg(row).arg(col).arg(val).arg(val, 4, 16, QChar('0')));
            });

    m_osdPollTimer = new QTimer(this);
    m_osdPollTimer->setSingleShot(false);
    connect(m_osdPollTimer, &QTimer::timeout, this, &MainWindow::onPollTimer);

    // 标签页标题稍后在 retranslateUi 中设置
    m_serialTabs->addTab(m_pageConn, QString());
    m_serialTabs->addTab(m_pageKey, QString());
    m_serialTabs->addTab(m_pageWireless, QString());
    m_serialTabs->addTab(m_pageOsd, QString());
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

    uiOsd->groupOsdControl->setTitle(QStringLiteral("OSD数据控制"));
    uiOsd->groupOsdGrid->setTitle(QStringLiteral("OSD可视化视图 (20行 x 53列)"));
    uiOsd->groupOsdInfo->setTitle(QStringLiteral("数据详情"));
    uiOsd->btnGetOsdData->setText(QStringLiteral("获取OSD数据 (0x57)"));
    uiOsd->btnMockOsdData->setText(QStringLiteral("🎨 模拟 OSD"));
    uiOsd->btnClearOsd->setText(QStringLiteral("清空"));
    uiOsd->checkAutoPoll->setText(QStringLiteral("自动轮询"));
    uiOsd->spinPollInterval->setSuffix(QStringLiteral(" ms"));
    uiOsd->spinCellSize->setPrefix(QStringLiteral("字符大小: "));

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
    bool mock = m_comm->isMockEnabled();
    uiConn->btnConnect->setText(connected ? QStringLiteral("断开") : QStringLiteral("连接"));
    if (mock) {
        uiConn->labelStatus->setText(QStringLiteral("Mock 模式 (模拟串口)"));
    } else {
        uiConn->labelStatus->setText(connected ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    }
    uiConn->btnClearRecv->setText(QStringLiteral("清空"));
    uiConn->checkAutoScroll->setText(QStringLiteral("自动滚动"));
    uiConn->checkMockMode->setText(QStringLiteral("Mock"));

    m_serialTabs->setTabText(0, QStringLiteral("连接"));
    m_serialTabs->setTabText(1, QStringLiteral("按键控制"));
    m_serialTabs->setTabText(2, QStringLiteral("无线参数"));
    m_serialTabs->setTabText(3, QStringLiteral("OSD显示"));
    m_serialTabs->setTabText(4, QStringLiteral("自定义命令"));
    m_serialTabs->setTabText(5, QStringLiteral("历史记录"));

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

// =====================================================================
// OSD 数据槽函数
// =====================================================================
const quint8 OSD_CMD_TYPE = 0x57;  // WIRELESS_DATA_TYPE_GET_OSD_DATA

void MainWindow::onGetOsdData() {
    TRACE;
    if (checkSerialConnected("getOsdData")) return;

    // 构造 OSD 数据获取命令包（与无线参数命令格式一致）
    QByteArray values;  // 空 values 表示查询
    QByteArray packet = m_protocol->createWirelessCommand(
        static_cast<wireless_data_type_t>(OSD_CMD_TYPE), values);
    logSend("GetOsdData", packet);
    m_comm->sendData(packet);

    // ---- 在 OSD 页面显示协议格式解析 ----
    QString fmt;
    // 逐字节描述
    fmt += "===== OSD 请求协议包 =====\n";
    for (int i = 0; i < packet.size(); ++i) {
        quint8 b = static_cast<quint8>(packet.at(i));
        QString desc;
        if (i == 0) desc = " 头(高)";
        else if (i == 1) desc = " 头(低)";
        else if (i == 2) desc = " 命令(0xA2=查询)";
        else if (i == 3) desc = " 数据长度(高)";
        else if (i == 4) desc = " 数据长度(低)";
        else if (i == 5) desc = " 数据类型(0x57=OSD)";
        else if (i >= 6 && i <= 11) desc = QString(" 保留%1").arg(i - 5);
        else if (i == packet.size() - 4) desc = " 校验和(高)";
        else if (i == packet.size() - 3) desc = " 校验和(低)";
        else if (i == packet.size() - 2) desc = " 尾(0x0D)";
        else if (i == packet.size() - 1) desc = " 尾(0x0A)";
        fmt += QString("  [%1] 0x%2%3\n")
                   .arg(i, 2, 10, QChar('0'))
                   .arg(b, 2, 16, QChar('0'))
                   .arg(desc);
    }
    // 校验和计算验证
    int dataLen = (static_cast<quint8>(packet.at(3)) << 8) | static_cast<quint8>(packet.at(4));
    QByteArray dataContent = packet.mid(5, dataLen);
    quint16 calcCS = m_protocol->calculateChecksum(dataContent);
    fmt += QString("数据内容(%1 bytes): %2\n").arg(dataLen)
               .arg(QString::fromLatin1(dataContent.toHex(' ').toUpper()));
    fmt += QString("校验和(计算): 0x%1\n").arg(calcCS, 4, 16, QChar('0'));
    fmt += QString("校验和(包内): 0x%1%2\n")
               .arg(static_cast<quint8>(packet.at(packet.size() - 4)), 2, 16, QChar('0'))
               .arg(static_cast<quint8>(packet.at(packet.size() - 3)), 2, 16, QChar('0'));
    fmt += QString("总长度: %1 bytes\n").arg(packet.size());

    uiOsd->textOsdInfo->setText(fmt);
}

void MainWindow::onClearOsd() {
    TRACE;
    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));
    m_osdGrid->clear();
    uiOsd->textOsdInfo->clear();
    m_osdAccumulated.clear();
}

void MainWindow::onMockModeToggled(bool checked) {
    TRACE << "checked=" << checked;
    m_comm->setMockEnabled(checked);
    if (checked) {
        uiConn->labelStatus->setText("Mock 模式 (模拟串口)");
        uiConn->labelStatus->setStyleSheet("color:orange;font-weight:bold;font-size:12pt;");
    } else {
        uiConn->labelStatus->setText("Disconnected");
        uiConn->labelStatus->setStyleSheet("color:red;font-weight:bold;font-size:12pt;");
    }
}

void MainWindow::onMockOsdData() {
    TRACE;
    // 先确保 Mock 模式已开启
    if (!m_comm->isMockEnabled()) {
        uiConn->checkMockMode->setChecked(true);
    }
    // 走完整协议路径：发送请求 → Mock 回复 → 解析 → 显示
    // 先发送 OSD 请求命令
    onGetOsdData();
    // 然后立即触发模拟回复（代替下位机回复）
    m_comm->triggerMockOsdReply();
    uiOsd->textOsdInfo->append("\n🎨 模拟 OSD 回复已注入");
}

void MainWindow::onAutoPollToggled(bool checked) {
    TRACE << "checked=" << checked;
    if (checked) {
        int interval = uiOsd->spinPollInterval->value();
        m_osdPollTimer->start(interval);
        // 立即执行一次
        onGetOsdData();
    } else {
        m_osdPollTimer->stop();
    }
}

void MainWindow::onFcTypeChanged(int /*index*/) {
    int fcIndex = uiOsd->cboFcType->currentIndex();
    int resIndex = uiOsd->cboOsdResolution->currentIndex(); // 0=720p, 1=1080p
    QString fcName = uiOsd->cboFcType->currentText();
    QString resName = uiOsd->cboOsdResolution->currentText();
    uiOsd->textOsdInfo->append(QString("\n🔄 切换: %1 (%2)")
                                   .arg(fcName).arg(resName));

    // fontPixel: 720p→24, 1080p→36
    int fontPixel = (resIndex == 0) ? 24 : 36;
    bool loaded = false;

    switch (fcIndex) {
    case 0: // Auto — 保持当前不变
        return;
    case 1: // Betaflight
        loaded = loadOsdFont(QString("font_bf_%1.png").arg(fontPixel), fontPixel);
        break;
    case 2: // INAV
        loaded = loadOsdFont(QString("font_inav_%1.png").arg(fontPixel), fontPixel);
        break;
    case 3: // ArduPilot
        loaded = loadOsdFont(QString("font_ardu_%1.png").arg(fontPixel), fontPixel);
        break;
    case 4: // Fettec
        loaded = loadOsdFont(QString("font_FTTC%1.png").arg(fontPixel), fontPixel);
        break;
    case 5: // KISS
        loaded = loadOsdFont(QString("font_kiss_%1.png").arg(fontPixel), fontPixel);
        break;
    case 6: // QUIC (WalkSnail)
        loaded = loadOsdFont(QString("WS_QUIC_%1.png").arg(fontPixel), fontPixel);
        break;
    case 7: // Custom → 使用 osdChars (1080p 36x36 独立字符)
        loaded = m_osdGrid->loadCharImages(":/osdchars", 256);
        if (loaded) {
            uiOsd->textOsdInfo->append("✓ 切换字体: osdChars (Custom)");
        }
        break;
    default:
        break;
    }

    if (!loaded) {
        uiOsd->textOsdInfo->append("⚠ 字体加载失败，使用 osdChars 回退");
        qWarning() << "[OSD] onFcTypeChanged: 字体加载失败, fcIndex=" << fcIndex << "使用osdChars回退";
        m_osdGrid->loadCharImages(":/osdchars", 256);
    }
}

void MainWindow::onOsdResolutionChanged(int /*index*/) {
    // 分辨率改变 → 重新加载当前飞控字体
    onFcTypeChanged(uiOsd->cboFcType->currentIndex());
}

void MainWindow::onPollTimer() {
    onGetOsdData();
}

void MainWindow::onOsdDataReceived(const QByteArray &dataContent) {
    TRACE << "dataContent size=" << dataContent.size();
    if (dataContent.isEmpty()) return;

    // 第一个字节是 cmd_type (0x57 = OSD_DATA)
    int osdPayloadSize = dataContent.size() - 1;
    if (osdPayloadSize <= 0) return;

    QByteArray osdPayload = dataContent.mid(1);  // 跳过 cmd_type

    // ---- 像素级 OSD 数据 —— character_map[20][53] 二进制 ----
    // OSD character_map 是 unsigned short 二维数组 (20行 x 53列)
    // 每个 unsigned short = 2 bytes, 小端序
    // 总大小 = 20 * 53 * 2 = 2120 bytes
    const int expectedMapSize = FCOSD_MAX_HEIGHT * FCOSD_MAX_WIDTH * 2; // 2120

    if (osdPayloadSize >= expectedMapSize) {
        // 解析 character_map
        const quint8 *data = reinterpret_cast<const quint8*>(osdPayload.constData());
        for (int row = 0; row < FCOSD_MAX_HEIGHT; ++row) {
            for (int col = 0; col < FCOSD_MAX_WIDTH; ++col) {
                int idx = row * FCOSD_MAX_WIDTH + col;
                // 小端 unsigned short
                quint16 val = data[idx * 2] | (data[idx * 2 + 1] << 8);
                m_osdCharMap[row][col] = val;
            }
        }
        m_osdGrid->setCharacterMap(m_osdCharMap);

        // 统计非零字符（活跃字符数）
        int activeCount = 0;
        quint16 minVal = 0xFFFF, maxVal = 0;
        for (int i = 0; i < FCOSD_MAX_HEIGHT * FCOSD_MAX_WIDTH; ++i) {
            quint16 v = reinterpret_cast<const quint16*>(osdPayload.constData())[i];
            if (v != 0) {
                activeCount++;
                if (v < minVal) minVal = v;
                if (v > maxVal) maxVal = v;
            }
        }

        QString info = QString("OSD 可视化视图已更新 | 活跃字符: %1/%2 | 索引范围: %3 ~ %4")
                           .arg(activeCount)
                           .arg(FCOSD_MAX_HEIGHT * FCOSD_MAX_WIDTH)
                           .arg(minVal == 0xFFFF ? 0 : minVal)
                           .arg(maxVal);
        uiOsd->textOsdInfo->setText(info);
    } else if (osdPayloadSize < 100) {
        // 可能是简短的回复（查询确认），显示到信息框
        QString hexStr = QString::fromLatin1(osdPayload.toHex(' ').toUpper());
        uiOsd->textOsdInfo->setText(
            QString("[%1] OSD 回复 (%2 bytes): %3")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                .arg(osdPayloadSize)
                .arg(hexStr));
    } else {
        // 数据大小不对，当作原始数据信息显示
        QString hexStr = QString::fromLatin1(osdPayload.toHex(' ').toUpper());
        uiOsd->textOsdInfo->setText(
            QString("[%1] OSD 数据 (%2 bytes, 预期 %3 bytes)\n%4")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                .arg(osdPayloadSize)
                .arg(expectedMapSize)
                .arg(hexStr.left(200)));
    }
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

    // 把收到的原始数据保存到 m_osdRawData，累加
    m_osdRawData.append(data);

    QString hexStr = QString::fromLatin1(data.toHex(' ').toUpper());

    // 调试：打印原始数据到 qDebug
    qDebug().noquote() << QString("===== 收到原始数据 %1 bytes =====").arg(data.size());
    qDebug().noquote() << QString("HEX: %1").arg(hexStr);
    qDebug().noquote() << QString("累计: %1 bytes").arg(m_osdRawData.size());

    // 打印到 OSD 信息框
    uiOsd->textOsdInfo->append(QString("[%1] RX: %2 bytes (累计 %3)")
                                   .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                                   .arg(data.size())
                                   .arg(m_osdRawData.size()));

    uiHistory->textRecvHistory->append(
        QString("[%1] RX: %2")
            .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
            .arg(hexStr)
    );
    // 自动滚动
    if (uiConn->checkAutoScroll->isChecked()) {
        QTextCursor c = uiHistory->textRecvHistory->textCursor();
        c.movePosition(QTextCursor::End);
        uiHistory->textRecvHistory->setTextCursor(c);
    }

    // ======== 直接从原始数据可视化 OSD (不依赖协议解析) ========
    // 放在 return 之前执行，确保即使协议解析失败也能显示
    visualizeRawOsdData();

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
    case 0x57: // OSD数据响应
        {
            int osdPayloadSize = content.size() - 1;
            displayText += QString("OSD数据: %1 bytes\n").arg(osdPayloadSize);
            if (osdPayloadSize > 0) {
                displayText += QString("首字节: 0x%1\n")
                               .arg(static_cast<quint8>(content.at(1)), 2, 16, QChar('0'));
                // 发送到 OSD 页面进行详细显示
                onOsdDataReceived(content);
            }
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

// 尝试从项目 fonts 目录或 SDK 路径加载指定 OSD 字体图
bool MainWindow::loadOsdFont(const QString &fontName, int fontPixel) {
    // fontPixel SDK 中对应 font_width（720p=24, 1080p=36）
    // font_height 对应关系：720p=36, 1080p=54
    int fontW = fontPixel;
    int fontH = (fontPixel <= 24) ? 36 : 54;

    qDebug() << "[OSD] 加载字体:" << fontName
             << "font_width:" << fontW << "font_height:" << fontH;
    QFileInfo fi(fontName);
    bool absolutePath = fi.isAbsolute();

    QStringList searchPaths;
    if (absolutePath) {
        searchPaths << fontName;
    } else {
        searchPaths << QCoreApplication::applicationDirPath() + "/fonts/" + fontName
                    << QFileInfo(__FILE__).absolutePath() + "/fonts/" + fontName;
    }
    // SDK 原始路径
    QString sdkImages = "X:/prj3_FPV60X/FPV40X_60X_SDK/gnd-ars31-fpv2-sdk-0.16.02-15-release-20260126-cx485"
                        "/base/arprj/apps/fpv40x/ar_ldy_gui/images/";
    QString sdkImages1080 = "X:/prj3_FPV60X/FPV40X_60X_SDK/gnd-ars31-fpv2-sdk-0.16.02-15-release-20260126-cx485"
                            "/base/arprj/apps/fpv40x/ar_ldy_gui/images_1080/";

    // 根据字体像素选择 SDK 目录
    if (fontPixel <= 24)
        searchPaths << sdkImages + fontName;
    else
        searchPaths << sdkImages1080 + fontName;

    searchPaths << sdkImages + fontName;
    searchPaths << sdkImages1080 + fontName;

    for (const QString &path : searchPaths) {
        qDebug() << "[OSD]  尝试:" << path;
        if (QFile::exists(path)) {
            qDebug() << "[OSD]  找到! 加载中...";
            bool ok = m_osdGrid->loadFontImage(path, fontW, fontH);
            if (ok) {
                qDebug() << "[OSD] 字体加载成功:" << path;
                uiOsd->textOsdInfo->append(QString("✓ 切换字体: %1 (W:%2,H:%3)")
                                               .arg(path).arg(fontW).arg(fontH));
                return true;
            } else {
                qDebug() << "[OSD]  loadFontImage失败:" << path;
            }
        }
    }
    qWarning() << "[OSD] 所有路径都找不到字体:" << fontName;
    return false;
}

bool MainWindow::checkSerialConnected(const QString &actionName) {
    // Mock 模式下视为已连接
    if (m_comm->isMockEnabled() || m_comm->isConnected()) {
        return false;
    }
    TRACE << "BLOCKED:" << actionName << "- not connected";
    QMessageBox::warning(this, tr("TIP"), tr("NOT_CONNECTED"));
    return true;
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

// ======== 从原始接收数据中直接扫描 OSD 数据段并可视化 ========
// OSD 数据显示格式说明（完全对齐 SDK msp_process_displayport）：
//
// 飞控端通过自定义协议 0x57 回复 OSD 数据
// osd_data 中包含 MSP V2 协议包
// MSP V2 header: $X><flags=0><cmd=0x00B6><size=N> (9 bytes)
//
// MSP V2 payload 中是 MSP DISPLAYPORT 命令：
//   子命令 0x35 (MSP_DISPLAY_DRAW_NORMAL_FULL_PKT) = 整帧更新
//     0x35 | seq(1) | rsv(1) | 逐个条目...
//   每个条目:
//     [len=4+textLen][row][col][attr][chars...]
//     attr 高字节: ((attr & 0x0f) << 8) 用于 4096 字符模式
//
//   子命令 0x09 (MSP_DISPLAY_DRAW_SCREEN) = 刷新屏幕
//   子命令 0x0A (MSP_DISPLAY_CLEAR_SCREEN) = 清屏
//
// SDK 端汇总到 MSPdisplayBuffer[20][53] (unsigned short)
// 然后通过 ring buffer 传递给 formfcosd 渲染
//
void MainWindow::visualizeRawOsdData()
{
    qDebug().noquote() << "visualizeRawOsdData: 总数据" << m_osdRawData.size() << "bytes";

    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));

    // 查找 OSD 数据包
    for (int offset = 0; offset <= m_osdRawData.size() - 10; ++offset) {
        const quint8 *raw = reinterpret_cast<const quint8*>(m_osdRawData.constData()) + offset;

        int dataLen = 0;
        int payloadStart = 0; // 指向 osd_data 起始

        // 模式1: FE EF A2 (len) 57 [osd_data...]
        if (raw[0] == 0xFE && raw[1] == 0xEF && raw[2] == 0xA2) {
            dataLen = (raw[3] << 8) | raw[4];
            int cmdOffset = offset + 5;
            if (cmdOffset >= m_osdRawData.size()) continue;
            if (m_osdRawData.at(cmdOffset) != 0x57) continue;
            payloadStart = cmdOffset + 1;
        }
        // 模式2: A2 (len) 57 [osd_data...]
        else if (raw[0] == 0xA2) {
            dataLen = (raw[1] << 8) | raw[2];
            int cmdOffset = offset + 3;
            if (cmdOffset >= m_osdRawData.size()) continue;
            if (m_osdRawData.at(cmdOffset) != 0x57) continue;
            payloadStart = cmdOffset + 1;
        }
        else continue;

        if (dataLen < 1 || dataLen > 5000) continue;

        const quint8 *osd = reinterpret_cast<const quint8*>(m_osdRawData.constData()) + payloadStart;
        int osdSize = qMin(dataLen - 1, static_cast<int>(m_osdRawData.size() - payloadStart));

        qDebug().noquote() << QString("  OSD包找到: offset=%1, dataLen=%2, osdSize=%3")
                              .arg(offset).arg(dataLen).arg(osdSize);

        // 打印首部 40 字节用于调试
        QString headerHex;
        for (int i = 0; i < qMin(40, osdSize); i++)
            headerHex += QString("%1 ").arg(osd[i], 2, 16, QChar('0'));
        qDebug().noquote() << "  osd数据头:" << headerHex;

        // 跳过 MSP V2 头部 ($X> 或 $XM 开头)
        // 格式: $X > [flags(1)] [len(2) little-endian] [cmd(2)]
        // 或   $X M [flags(1)] [len(2) little-endian] [cmd(2)]
        int dataStart = 0;
        if (osdSize >= 8 && osd[0] == 0x24 && osd[1] == 0x58) {
            // $X 头部: 2 + 1 + 1 + 2 + 2 = 8 字节
            dataStart = 8;
            int mspLen = osd[3] | (osd[4] << 8); // 小端
            int mspCmd = osd[5] | (osd[6] << 8); // 小端
            qDebug().noquote() << QString("  跳过 MSP V2 头: len=%1 cmd=0x%2").arg(mspLen).arg(mspCmd, 4, 16, QChar('0'));
        }

        // 解析 MSP DISPLAYPORT 格式的 OSD 数据
        // 参考 SDK fcmsp_proc.cpp:
        //   0x35 [seq] [rsv] -- 后续 entries: [len][row][col][attr][chars...]
        //   0x05 [row] [col] [attr] [chars...]  -- MSP_DISPLAY_TEXT (不带 len!)
        //   0x09 DRAW_SCREEN
        //   0x0A CLEAR_SCREEN
        int parsedCount = 0;
        int p = dataStart;

        while (p < osdSize) {
            quint8 subcmd = osd[p];

            if (subcmd == 0x35) {
                // MSP_DISPLAY_DRAW_NORMAL_FULL_PKT
                if (p + 7 > osdSize) break;
                int entryStart = p + 3; // 跳过 0x35 [seq] [rsv]
                int entryPos = entryStart;
                while (entryPos + 4 <= osdSize) {
                    int entryLen = osd[entryPos];
                    if (entryLen < 4) break;
                    int textLen = entryLen - 4;
                    if (entryPos + entryLen > osdSize) break;
                    int row = osd[entryPos + 1];
                    int col = osd[entryPos + 2];
                    int rawAttr = osd[entryPos + 3];
                    unsigned short attrs = (rawAttr & 0x0f) << 8;
                    if (row >= 0 && row < FCOSD_MAX_HEIGHT && col >= 0 && col < FCOSD_MAX_WIDTH) {
                        for (int i = 0; i < textLen && (col + i) < FCOSD_MAX_WIDTH; ++i) {
                            m_osdCharMap[row][col + i] = osd[entryPos + 4 + i] | attrs;
                        }
                        parsedCount++;
                    }
                    entryPos += entryLen;
                }
                p = entryPos;
            }
            else if (subcmd == 0x05) {
                // MSP_DISPLAY_TEXT: 0x05 [row] [col] [attr] [chars...]
                // 注意: 0x05 子命令不带 len 字段！文本直到遇到下一个子命令或 0x00 结束
                // 简化解法: 先取前4字节(05+row+col+attr)，剩余直到下一个子命令都是文本
                if (p + 4 > osdSize) break;
                int row = osd[p + 1];
                int col = osd[p + 2];
                int rawAttr = osd[p + 3];
                unsigned short attrs = (rawAttr & 0x0f) << 8;
                // 文本从 pos+4 开始，直到 osdSize 或下一个子命令开头
                int textStart = p + 4;
                int textEnd;
                for (textEnd = textStart; textEnd < osdSize; textEnd++) {
                    quint8 b = osd[textEnd];
                    if (b == 0x00 || b == 0x05 || b == 0x09 || b == 0x0A || b == 0x35 || b >= 0x80)
                        break; // 遇到下一个子命令或非文本字节
                }
                int textLen = textEnd - textStart;
                if (row >= 0 && row < FCOSD_MAX_HEIGHT) {
                    for (int i = 0; i < textLen && (col + i) < FCOSD_MAX_WIDTH; ++i) {
                        m_osdCharMap[row][col + i] = osd[textStart + i] | attrs;
                    }
                    parsedCount++;
                }
                p = qMax(textEnd, p + 4); // 至少前进 4 字节
            }
            else if (subcmd == 0x09 || subcmd == 0x0A || subcmd == 0x00) {
                // DRAW_SCREEN, CLEAR_SCREEN, 或填充零
                p++;
            }
            else if (subcmd >= 4) {
                // 容错: 尝试按 entry 格式 [len][row][col][attr][chars...]
                int entryLen = subcmd;
                if (p + entryLen <= osdSize) {
                    int textLen = entryLen - 4;
                    if (textLen > 0 && textLen < 60) {
                        int row = osd[p + 1];
                        int col = osd[p + 2];
                        int rawAttr = osd[p + 3];
                        unsigned short attrs = (rawAttr & 0x0f) << 8;
                        if (row >= 0 && row < FCOSD_MAX_HEIGHT && col >= 0 && col < FCOSD_MAX_WIDTH) {
                            for (int i = 0; i < textLen && (col + i) < FCOSD_MAX_WIDTH; ++i) {
                                m_osdCharMap[row][col + i] = osd[p + 4 + i] | attrs;
                            }
                            parsedCount++;
                        }
                        p += entryLen;
                        continue;
                    }
                }
                p++;
            }
            else {
                p++;
            }
        }

        if (parsedCount > 0) {
            // 打印 OSD 内容
            qDebug().noquote() << QString("  ✅ 解析了 %1 个条目").arg(parsedCount);
            qDebug().noquote() << "--- OSD 内容 ---";
            for (int row = 0; row < FCOSD_MAX_HEIGHT; ++row) {
                QString line;
                for (int col = 0; col < FCOSD_MAX_WIDTH; ++col) {
                    unsigned short v = m_osdCharMap[row][col];
                    if ((v & 0xFF) >= 0x20 && (v & 0xFF) <= 0x7E)
                        line += QChar(v & 0xFF);
                    else if (v == 0)
                        line += ' ';
                    else
                        line += '?';
                }
                if (line.simplified().length() > 0) {
                    qDebug().noquote() << QString("  [%1] ").arg(row) + line;
                }
            }

            m_osdGrid->setCharacterMap(m_osdCharMap);
            uiOsd->textOsdInfo->append(
                QString("✅ OSD: %1 条目 (%2 bytes)").arg(parsedCount).arg(osdSize));
            return;
        }
    }

    qDebug().noquote() << "未找到 OSD 数据包，当前" << m_osdRawData.size() << "bytes";
}
