#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QScrollArea>
#include <QFrame>
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
#include "crsftestpage.h"
#include "sbustestpage.h"
#include "mavlinktestpage.h"
#include "f0readpage.h"
#include "ui_page_settings.h"
#include "translator.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QEvent>
#include <QRect>
#include <QFont>
#include <QDateTime>
#include <QTextCursor>
#include <QSerialPortInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSettings>
#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QUrl>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>

// 调试宏：所有槽函数入口打印
#define TRACE qDebug() << "[TRACE]" << Q_FUNC_INFO

// 基带功率索引 -> 功率显示文本映射表
    static const char* bbPwrNames[32] = {
        "25", "100", "150", "200",
        "300", "350", "500", "700",
        "1000", "1500", "2500", "3000",
        "4000", "1000 AUTO", "2000 AUTO", "4000 AUTO",
        "100 AUTO", "500 AUTO", "5000", "5000 AUTO",
        "?", "?", "?", "?",
        "?", "?", "?", "?",
        "?", "?", "?", "?",
    };

// =====================================================================
// 构造函数 / 析构函数
// =====================================================================
static QString detectWindeployqt();   // 自动探测 windeployqt.exe 位置
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

    // 恢复上次窗口大小/位置（支持自定义缩放并记忆）
    QSettings winSettings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QByteArray geom = winSettings.value(QStringLiteral("window/geometry")).toByteArray();
    if (!geom.isEmpty()) {
        restoreGeometry(geom);
    } else {
        resize(1300, 800);
    }
    // 允许自由缩放（下限 800x500，防止缩到过小而重叠/错乱）
    setMinimumSize(800, 500);
}

MainWindow::~MainWindow() {
    if (m_comm->isConnected()) m_comm->disconnect();
    if (m_osdPollTimer) {
        m_osdPollTimer->stop();
        delete m_osdPollTimer;
        m_osdPollTimer = nullptr;
    }
    // 保存 OSD 页 splitter 位置
    if (uiOsd && uiOsd->splitterOsd) {
        QSettings osdSettings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
        osdSettings.setValue(QStringLiteral("osd/splitterState"), uiOsd->splitterOsd->saveState());
    }
    delete ui;
    delete uiFirmware; delete uiConn; delete uiKey;
    delete uiWireless; delete uiCustom; delete uiHistory; delete uiOsd; delete uiSettings;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    TRACE;
    // 保存窗口大小/位置，下次启动恢复（自定义缩放记忆）
    QSettings winSettings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    winSettings.setValue(QStringLiteral("window/geometry"), saveGeometry());
    if (m_comm->isConnected()) m_comm->disconnect();
    event->accept();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev) {
    // 让 OSD 数据详情框(textOsdInfo)始终填满 groupOsdInfo 客户区
    if (obj == uiOsd->groupOsdInfo && ev->type() == QEvent::Resize) {
        if (uiOsd->textOsdInfo) {
            QRect cr = uiOsd->groupOsdInfo->contentsRect();
            uiOsd->textOsdInfo->setGeometry(cr);
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}

// =====================================================================
// 初始化 UI
// =====================================================================
void MainWindow::initUI() {
    TRACE;
    ui->setupUi(this);
    setFont(QFont("Microsoft YaHei", 9));

    loadSerialPages();
    loadFirmwarePage();
    buildPackagePage();
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

// 把页面内容包进可滚动区域（QScrollArea）：
// 窗口足够大时内容拉伸填满（平滑缩放）；窗口缩小到小于内容时出现滚动条，
// 内容滚动而不重叠。允许窗口任意缩放，不锁死。
QScrollArea *MainWindow::makeScrollable(QWidget *page) {
    QScrollArea *sa = new QScrollArea;
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setWidget(page);
    // 允许滚动区随窗口收缩（不把内容最小尺寸强加给窗口）
    sa->setMinimumSize(0, 0);
    return sa;
}

void MainWindow::loadSettingsPage() {
    TRACE;
    m_pageSettings = new QWidget();
    uiSettings = new Ui::PageSettings();
    uiSettings->setupUi(m_pageSettings);
    uiSettings->comboLang->setCurrentIndex(0);
    ui->stackContent->addWidget(m_pageSettings);
}

// =====================================================================
// 打包工具页
// =====================================================================
void MainWindow::buildPackagePage() {
    TRACE;
    m_pagePackage = new QWidget();
    QVBoxLayout *outer = new QVBoxLayout(m_pagePackage);
    outer->setContentsMargins(12, 12, 12, 12);
    outer->setSpacing(10);

    // ---- 打包设置组 ----
    QGroupBox *grp = new QGroupBox(QStringLiteral("打包设置"));
    QGridLayout *grid = new QGridLayout(grp);
    grid->setColumnStretch(1, 1);

    // exe 行
    grid->addWidget(new QLabel(QStringLiteral("目标 exe:")), 0, 0);
    m_pkgExeEdit = new QLineEdit;
    m_pkgExeEdit->setPlaceholderText(QStringLiteral("选择要打包的 .exe 文件"));
    QPushButton *btnExe = new QPushButton(QStringLiteral("浏览..."));
    connect(btnExe, &QPushButton::clicked, this, &MainWindow::onPkgBrowseExe);
    grid->addWidget(m_pkgExeEdit, 0, 1);
    grid->addWidget(btnExe, 0, 2);

    // 输出目录行
    grid->addWidget(new QLabel(QStringLiteral("输出目录:")), 1, 0);
    m_pkgDirEdit = new QLineEdit;
    m_pkgDirEdit->setPlaceholderText(QStringLiteral("打包产物存放目录"));
    QPushButton *btnDir = new QPushButton(QStringLiteral("浏览..."));
    connect(btnDir, &QPushButton::clicked, this, &MainWindow::onPkgBrowseDir);
    grid->addWidget(m_pkgDirEdit, 1, 1);
    grid->addWidget(btnDir, 1, 2);

    // windeployqt 路径行
    grid->addWidget(new QLabel(QStringLiteral("windeployqt:")), 2, 0);
    m_pkgWindeployqtEdit = new QLineEdit;
    m_pkgWindeployqtEdit->setPlaceholderText(QStringLiteral("windeployqt.exe 路径（留空自动探测）"));
    QPushButton *btnWd = new QPushButton(QStringLiteral("浏览..."));
    connect(btnWd, &QPushButton::clicked, this, &MainWindow::onPkgBrowseWindeployqt);
    grid->addWidget(m_pkgWindeployqtEdit, 2, 1);
    grid->addWidget(btnWd, 2, 2);

    outer->addWidget(grp);

    // ---- 操作按钮行 ----
    QHBoxLayout *btnRow = new QHBoxLayout;
    m_pkgRunBtn = new QPushButton(QStringLiteral("开始打包"));
    m_pkgRunBtn->setMinimumHeight(32);
    connect(m_pkgRunBtn, &QPushButton::clicked, this, &MainWindow::onPkgRun);
    btnRow->addWidget(m_pkgRunBtn);

    m_pkgOpenBtn = new QPushButton(QStringLiteral("打开文件夹"));
    m_pkgOpenBtn->setMinimumHeight(32);
    m_pkgOpenBtn->setVisible(false);   // 打包成功后才显示
    connect(m_pkgOpenBtn, &QPushButton::clicked, this, &MainWindow::onPkgOpenFolder);
    btnRow->addWidget(m_pkgOpenBtn);

    btnRow->addStretch(1);
    outer->addLayout(btnRow);

    // ---- 日志 ----
    m_pkgLog = new QPlainTextEdit;
    m_pkgLog->setReadOnly(true);
    outer->addWidget(m_pkgLog, 1);

    // 恢复历史路径（QSettings 记忆）
    QSettings s(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString lastExe = s.value(QStringLiteral("package/exe")).toString();
    QString lastDir = s.value(QStringLiteral("package/dir")).toString();
    QString lastWd  = s.value(QStringLiteral("package/windeployqt")).toString();
    if (!lastExe.isEmpty()) m_pkgExeEdit->setText(lastExe);
    if (!lastDir.isEmpty()) m_pkgDirEdit->setText(lastDir);
    if (!lastWd.isEmpty())  m_pkgWindeployqtEdit->setText(lastWd);
    else {
        // 自动探测 windeployqt：exe 同级 → 常见 Qt bin 目录
        QString probed = detectWindeployqt();
        if (!probed.isEmpty()) m_pkgWindeployqtEdit->setText(probed);
    }

    ui->stackContent->addWidget(m_pagePackage);
}

// 自动探测 windeployqt.exe 的位置
static QString detectWindeployqt() {
    QStringList candidates;
    // 常见 Qt bin 目录
    candidates << QStringLiteral("E:/Qt/5.15.2/mingw81_32/bin/windeployqt.exe")
               << QStringLiteral("C:/Qt/5.15.2/mingw81_32/bin/windeployqt.exe");
    // 从 PATH 查找
    QString pathEnv = qEnvironmentVariable("PATH");
    const QStringList dirs = pathEnv.split(';', Qt::SkipEmptyParts);
    for (const QString &d : dirs) {
        QFileInfo fi(d + QLatin1String("/windeployqt.exe"));
        if (fi.isFile()) candidates << fi.absoluteFilePath();
    }
    for (const QString &c : candidates) {
        if (QFileInfo::exists(c)) return c;
    }
    return QString();
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
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->spinChannel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->comboHop, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->comboBbPwr, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->comboRelayBand, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->spinRelayChannel, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->comboRelayHop, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdatePreviewFromParams);
    connect(uiWireless->comboRelayBbPwr, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUpdatePreviewFromParams);

    // ---- 中继(Relay)无线参数按钮 ----
    connect(uiWireless->btnSetRelayFreq, &QPushButton::clicked, this, &MainWindow::onSetRelayFreq);
    connect(uiWireless->btnGetRelayFreq, &QPushButton::clicked, this, &MainWindow::onGetRelayFreq);
    connect(uiWireless->btnGetRelayStatus, &QPushButton::clicked, this, &MainWindow::onGetRelayStatus);
    connect(uiWireless->btnGetRelayBbPwr, &QPushButton::clicked, this, &MainWindow::onGetRelayBbPwr);
    connect(uiWireless->btnSetRelayBbPwr, &QPushButton::clicked, this, &MainWindow::onSetRelayBbPwr);
    connect(uiWireless->btnGetRelayGndDist, &QPushButton::clicked, this, &MainWindow::onGetRelayGndDist);
    connect(uiWireless->btnGetRelaySkyDist, &QPushButton::clicked, this, &MainWindow::onGetRelaySkyDist);
    connect(uiWireless->btnGetOsdDataWireless, &QPushButton::clicked, this, &MainWindow::onGetOsdDataWireless);
    connect(uiWireless->btnClearOsdDataWireless, &QPushButton::clicked, this, &MainWindow::onClearOsdDataWireless);

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

    // OSD 页 splitter（黑框/数据详情 上下分割）拉伸因子 + 状态持久化
    if (uiOsd->splitterOsd) {
        uiOsd->splitterOsd->setChildrenCollapsible(false);
        uiOsd->splitterOsd->setStretchFactor(0, 6); // 黑框区占大
        uiOsd->splitterOsd->setStretchFactor(1, 2); // 数据详情区占小
        QSettings osdSettings(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
        QByteArray st = osdSettings.value(QStringLiteral("osd/splitterState")).toByteArray();
        if (!st.isEmpty())
            uiOsd->splitterOsd->restoreState(st);
    }

    // 数据详情框(textOsdInfo)随 groupOsdInfo 自由拉伸填满
    if (uiOsd->textOsdInfo) {
        // 脱离 osdInfoLayout，改由 eventFilter 手动几何管理（彻底规避 QPlainTextEdit 布局约束）
        if (uiOsd->osdInfoLayout) {
            uiOsd->osdInfoLayout->removeWidget(uiOsd->textOsdInfo);
            delete uiOsd->osdInfoLayout;
            uiOsd->osdInfoLayout = nullptr;
        }
        uiOsd->textOsdInfo->setMinimumSize(0, 0);
        uiOsd->textOsdInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        uiOsd->textOsdInfo->setParent(uiOsd->groupOsdInfo);
        uiOsd->textOsdInfo->show();
        // 监听 groupOsdInfo 尺寸变化，强制 textOsdInfo 填满
        uiOsd->groupOsdInfo->installEventFilter(this);
    }

    // 自动加载 OSD 字体（从项目内部 osdChars 目录加载）
    bool fontLoaded = false;
    // 策略1: Qt 资源文件（编译时嵌入，最可靠）
    if (QFile::exists(":/osdchars/0.png")) {
        qDebug() << "[OSD] 从 Qt 资源加载 osdChars";
        fontLoaded = m_osdGrid->loadCharImages(":/osdchars", 512);
    }
    // 策略2: 源码目录下的 osdChars_new（开发模式）
    if (!fontLoaded) {
        QString fontDir = QFileInfo(__FILE__).absolutePath() + "/osdChars_new";
        if (QDir(fontDir).exists()) {
            qDebug() << "[OSD] 从源码目录加载:" << fontDir;
            fontLoaded = m_osdGrid->loadCharImages(fontDir, 512);
        }
    }
    // 策略3: exe 同级目录（发布模式）
    if (!fontLoaded) {
        QString fontDir = QCoreApplication::applicationDirPath() + "/osdChars_new";
        if (QDir(fontDir).exists()) {
            qDebug() << "[OSD] 从 exe 目录加载:" << fontDir;
            fontLoaded = m_osdGrid->loadCharImages(fontDir, 512);
        }
    }
    if (!fontLoaded) {
        qWarning() << "[OSD] 找不到 osdChars，使用 QPainter 回退模式";
    }

    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));

    connect(uiOsd->btnGetOsdData, &QPushButton::clicked, this, &MainWindow::onGetOsdData);
    connect(uiOsd->btnClearOsd, &QPushButton::clicked, this, &MainWindow::onClearOsd);
    connect(uiOsd->checkAutoPoll, &QCheckBox::toggled, this, &MainWindow::onAutoPollToggled);
    connect(uiOsd->cboFcType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFcTypeChanged);
    connect(uiOsd->cboOsdResolution, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onOsdResolutionChanged);
    // spinCellSize 现在用于控制无字体模式下的像素大小（已移除 setCellSize）
    connect(m_osdGrid, &OsdGridWidget::cellHovered,
            this, [this](int row, int col, unsigned short val) {
                // 悬停格子信息显示到状态栏，不占用数据详情框（数据详情框只放接收返回值）
                statusBar()->showMessage(
                    QString("行:%1  列:%2  字符索引:%3 (0x%4)")
                        .arg(row).arg(col).arg(val).arg(val, 4, 16, QChar('0')));
            });

    m_osdPollTimer = new QTimer(this);
    m_osdPollTimer->setSingleShot(false);
    connect(m_osdPollTimer, &QTimer::timeout, this, &MainWindow::onPollTimer);

    // 标签页标题稍后在 retranslateUi 中设置
    m_serialTabs->addTab(makeScrollable(m_pageConn), QString());
    m_serialTabs->addTab(makeScrollable(m_pageKey), QString());
    m_serialTabs->addTab(makeScrollable(m_pageWireless), QString());
    m_serialTabs->addTab(makeScrollable(m_pageOsd), QString());

    // ---- F0 读取测试页面（快捷指令），放在 OSD 显示后面 ----
    m_f0readPage = new F0ReadPage(m_comm, this);
    m_serialTabs->addTab(makeScrollable(m_f0readPage), QString());

    // 快捷指令(F0)数据 → 无线参数页同步：
    //  - 发送 F0 时清空无线历史缓冲，避免快捷指令的历史响应堆积污染无线打印栏
    //  - 收到频点(0x51)/功率(0x53)响应时更新无线参数页的频率/功率设置
    connect(m_f0readPage, &F0ReadPage::f0CommandSent, this, [this]() {
        m_wirelessSuppress = true;      // 快捷指令接收期间，不打印响应到无线 textStatus
        m_wirelessBuf.clear();
        // 保底：2 秒后自动解除抑制，避免一直屏蔽无线打印
        QTimer::singleShot(2000, this, [this]() { m_wirelessSuppress = false; });
    });
    connect(m_f0readPage, &F0ReadPage::freqUpdated, this, [this](int band, int channel, int hop) {
        uiWireless->comboBand->setCurrentIndex(band);
        uiWireless->spinChannel->setValue(channel + 1);
        uiWireless->comboHop->setCurrentIndex(hop);
    });
    connect(m_f0readPage, &F0ReadPage::powerUpdated, this, [this](int pwrIdx, quint32 bitmap) {
        QComboBox *combo = uiWireless->comboBbPwr;
        combo->blockSignals(true);
        combo->clear();
        for (int i = 0; i < 20; ++i)
            if (bitmap & (1U << i))
                combo->addItem(QString::fromUtf8(bbPwrNames[i]), i);
        combo->blockSignals(false);
        int curIdx = combo->findData(pwrIdx);
        if (curIdx >= 0) combo->setCurrentIndex(curIdx);
    });

    m_serialTabs->addTab(makeScrollable(m_pageCustom), QString());

    // ---- CRSF 测试页面 ----
    m_crsfPage = new CrsfTestPage(m_comm, this);
    // 将 CRSF 测试页作为独立的 widget 加入 stacked widget（串口标签页之后）
    m_serialTabs->addTab(makeScrollable(m_crsfPage), QString());

    // ---- SBUS 测试页面 ----
    m_sbusPage = new SbusTestPage(m_comm, this);
    m_serialTabs->addTab(makeScrollable(m_sbusPage), QString());

    // ---- MAVLink 测试页面 ----
    m_mavlinkPage = new MavlinkTestPage(m_comm, this);
    m_serialTabs->addTab(makeScrollable(m_mavlinkPage), QString());

    // ---- 历史记录，放在最右侧 ----
    m_serialTabs->addTab(makeScrollable(m_pageHistory), QString());

    ui->stackContent->addWidget(m_serialTabs);

    onRefreshPorts();
    onUpdateKeyPreview();
    onUpdatePreviewFromParams();

    // ===== 全局样式 (商业软件风格) =====
    setStyleSheet(QStringLiteral(R"(
        QMainWindow, QDialog, QWidget#centralwidget {
            background: #f5f6f8;
            color: #2b2b2b;
        }
        QGroupBox {
            background: #ffffff;
            border: 1px solid #d8dce2;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 4px;
            font-weight: 600;
            color: #333;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 4px;
            background: #ffffff;
            color: #1f6feb;
            font-size: 13px;
        }
        QPushButton {
            background: #fafbfc;
            border: 1px solid #c9ced6;
            border-radius: 4px;
            padding: 4px 10px;
            color: #24292f;
        }
        QPushButton:hover { background: #f0f4f8; border-color: #1f6feb; }
        QPushButton:pressed { background: #e3e9f0; }
        QPushButton:focus { border-color: #1f6feb; }
        QLineEdit, QComboBox, QSpinBox, QPlainTextEdit, QTextEdit {
            background: #ffffff;
            border: 1px solid #c9ced6;
            border-radius: 4px;
            padding: 3px 6px;
            color: #24292f;
            selection-background-color: #cfe3ff;
        }
        QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border-color: #1f6feb; }

        QLabel { color: #3a3f47; }
        QTabWidget::pane {
            border: 1px solid #d8dce2;
            background: #f5f6f8;
            border-radius: 4px;
            top: -1px;
        }
        QTabBar::tab {
            background: #e9ebef;
            border: 1px solid #d8dce2;
            padding: 6px 14px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            color: #444;
        }
        QTabBar::tab:selected { background: #ffffff; color: #1f6feb; font-weight: 600; }
        QTabBar::tab:hover:!selected { background: #f0f2f5; }
        QStatusBar { background: #ffffff; border-top: 1px solid #d8dce2; color: #555; }
    )"));
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
    uiSettings->labelAboutVer->setText(QStringLiteral("版本: 2.1.1"));
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
    uiKey->btnForce720p->setText(QStringLiteral("强制720p60"));
    uiKey->btnDebug3->setText(QStringLiteral("debug3模式"));
    uiKey->lbKeyName->setText(QStringLiteral("按键:"));
    uiKey->lbPressType->setText(QStringLiteral("按压类型:"));
    uiKey->lbPreview->setText(QStringLiteral("预览:"));
    uiKey->btnSendCustomKey->setText(QStringLiteral("发送"));
    uiKey->comboPressType->setItemText(0, QStringLiteral("单击 (0)"));
    uiKey->comboPressType->setItemText(1, QStringLiteral("长按 (1)"));

    uiWireless->groupFreq->setTitle(QStringLiteral("频点设置"));
    uiWireless->groupStatus->setTitle(QStringLiteral("状态与参数"));
    uiWireless->lbBand->setText(QStringLiteral("频段:"));
    uiWireless->lbChan->setText(QStringLiteral("通道:"));
    uiWireless->lbHop->setText(QStringLiteral("模式:"));
    uiWireless->lbBbPwr->setText(QStringLiteral("基带功率:"));
    uiWireless->btnSetFreq->setText(QStringLiteral("设置频点 (0x50)"));
    uiWireless->btnGetFreq->setText(QStringLiteral("获取频点 (0x51)"));
    uiWireless->btnGetStatus->setText(QStringLiteral("获取地面状态 (0x52)"));
    uiWireless->btnGetStatusSky->setText(QStringLiteral("获取天空状态 (0x55)"));
    uiWireless->btnGetDistance->setText(QStringLiteral("获取距离 (0x56)"));
    uiWireless->btnGetBbPwr->setText(QStringLiteral("获取基带功率 (0x53)"));
    uiWireless->btnSetBbPwr->setText(QStringLiteral("设置基带功率 (0x54)"));
    uiWireless->comboBand->setItemText(0, QStringLiteral("频段A (0)"));
    uiWireless->comboBand->setItemText(1, QStringLiteral("频段B (1)"));
    uiWireless->comboBand->setItemText(2, QStringLiteral("频段C (2)"));
    uiWireless->comboBand->setItemText(3, QStringLiteral("频段D (3)"));
    uiWireless->comboBand->setItemText(4, QStringLiteral("频段E (4)"));
    uiWireless->comboHop->setItemText(0, QStringLiteral("定频 (0)"));
    uiWireless->comboHop->setItemText(1, QStringLiteral("跳频 (1)"));

    // 频道 UI 显示1开头, 实际下发从0开始 (显示值 = 实际值 + 1)
    uiWireless->spinChannel->setRange(1, 40);
    uiWireless->spinRelayChannel->setRange(1, 40);

    // 中继(Relay)翻译
    uiWireless->groupRelay->setTitle(QStringLiteral("中继 (Relay)"));
    uiWireless->lbRelayBand->setText(QStringLiteral("频段:"));
    uiWireless->lbRelayChan->setText(QStringLiteral("通道:"));
    uiWireless->lbRelayHop->setText(QStringLiteral("模式:"));
    uiWireless->lbRelayBbPwr->setText(QStringLiteral("基带功率:"));
    uiWireless->btnSetRelayFreq->setText(QStringLiteral("设置中继频点 (0xA0)"));
    uiWireless->btnGetRelayFreq->setText(QStringLiteral("获取中继频点 (0xA1)"));
    uiWireless->btnGetRelayStatus->setText(QStringLiteral("获取中继状态 (0xA2)"));
    uiWireless->btnGetRelayBbPwr->setText(QStringLiteral("获取中继功率 (0xA3)"));
    uiWireless->btnSetRelayBbPwr->setText(QStringLiteral("设置中继功率 (0xA4)"));
    uiWireless->btnGetRelayGndDist->setText(QStringLiteral("获取地面距离 (0xA5)"));
    uiWireless->btnGetRelaySkyDist->setText(QStringLiteral("获取天空距离 (0xA6)"));
    uiWireless->groupOsdData->setTitle(QStringLiteral("OSD数据"));
    uiWireless->btnGetOsdDataWireless->setText(QStringLiteral("获取OSD数据 (0x57)"));
    uiWireless->btnClearOsdDataWireless->setText(QStringLiteral("清空"));
    uiWireless->groupRelayData->setTitle(QStringLiteral("中继/无线数据打印"));
    uiWireless->groupPreview->setTitle(QStringLiteral("命令预览"));
    uiWireless->lbPreviewHint->setText(QStringLiteral("预览:"));
    uiWireless->comboRelayBand->setItemText(0, QStringLiteral("频段A (0)"));
    uiWireless->comboRelayBand->setItemText(1, QStringLiteral("频段B (1)"));
    uiWireless->comboRelayBand->setItemText(2, QStringLiteral("频段C (2)"));
    uiWireless->comboRelayBand->setItemText(3, QStringLiteral("频段D (3)"));
    uiWireless->comboRelayBand->setItemText(4, QStringLiteral("频段E (4)"));
    uiWireless->comboRelayBand->setCurrentIndex(1); // 默认频段B
    uiWireless->comboRelayHop->setItemText(0, QStringLiteral("定频 (0)"));
    uiWireless->comboRelayHop->setItemText(1, QStringLiteral("跳频 (1)"));

    uiOsd->groupOsdControl->setTitle(QStringLiteral("OSD数据控制"));
    uiOsd->groupOsdGrid->setTitle(QStringLiteral("OSD可视化视图 (20行 x 53列)"));
    uiOsd->groupOsdInfo->setTitle(QStringLiteral("数据详情"));
    uiOsd->btnGetOsdData->setText(QStringLiteral("获取OSD数据 (0x57)"));
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
    uiConn->btnConnect->setText(connected ? QStringLiteral("断开") : QStringLiteral("连接"));
    uiConn->labelStatus->setText(connected ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    uiConn->btnClearRecv->setText(QStringLiteral("清空"));
    uiConn->checkAutoScroll->setText(QStringLiteral("自动滚动"));

    m_serialTabs->setTabText(0, QStringLiteral("连接"));
    m_serialTabs->setTabText(1, QStringLiteral("按键控制"));
    m_serialTabs->setTabText(2, QStringLiteral("无线参数"));
    m_serialTabs->setTabText(3, QStringLiteral("OSD显示"));
    m_serialTabs->setTabText(4, QStringLiteral("快捷指令"));
    m_serialTabs->setTabText(5, QStringLiteral("自定义数据"));
    m_serialTabs->setTabText(6, QStringLiteral("CRSF数据"));
    m_serialTabs->setTabText(7, QStringLiteral("SBUS数据"));
    m_serialTabs->setTabText(8, QStringLiteral("MAVLink数据"));
    m_serialTabs->setTabText(9, QStringLiteral("历史记录"));

    // 侧边栏
    int sc = ui->listSidebar->count();
    if (sc > 0) ui->listSidebar->item(0)->setText(QStringLiteral("串口工具"));
    if (sc > 1) ui->listSidebar->item(1)->setText(QStringLiteral("固件解析"));
    if (sc > 2) ui->listSidebar->item(2)->setText(QStringLiteral("打包工具"));
    if (sc > 3) ui->listSidebar->item(3)->setText(QStringLiteral("设置"));
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

// =====================================================================
// 打包工具：槽函数
// =====================================================================
void MainWindow::onPkgBrowseExe() {
    TRACE;
    QSettings s(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString startDir = s.value(QStringLiteral("package/exeDir")).toString();
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择要打包的可执行文件"), startDir,
                                                QStringLiteral("可执行文件 (*.exe);;所有文件 (*)"));
    if (path.isEmpty()) return;
    m_pkgExeEdit->setText(path);
    s.setValue(QStringLiteral("package/exe"), path);
    s.setValue(QStringLiteral("package/exeDir"), QFileInfo(path).absolutePath());
}

void MainWindow::onPkgBrowseDir() {
    TRACE;
    QSettings s(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString startDir = s.value(QStringLiteral("package/dir")).toString();
    if (startDir.isEmpty()) startDir = s.value(QStringLiteral("package/exeDir")).toString();
    QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("选择打包输出目录"), startDir);
    if (dir.isEmpty()) return;
    m_pkgDirEdit->setText(dir);
    s.setValue(QStringLiteral("package/dir"), dir);
}

void MainWindow::onPkgBrowseWindeployqt() {
    TRACE;
    QSettings s(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString startDir = s.value(QStringLiteral("package/windeployqtDir")).toString();
    if (startDir.isEmpty()) startDir = QStringLiteral("E:/Qt/5.15.2/mingw81_32/bin");
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择 windeployqt.exe"), startDir,
                                                QStringLiteral("可执行文件 (*.exe);;所有文件 (*)"));
    if (path.isEmpty()) return;
    m_pkgWindeployqtEdit->setText(path);
    s.setValue(QStringLiteral("package/windeployqt"), path);
    s.setValue(QStringLiteral("package/windeployqtDir"), QFileInfo(path).absolutePath());
}

void MainWindow::onPkgRun() {
    TRACE;
    if (m_pkgProc && m_pkgProc->state() != QProcess::NotRunning) {
        m_pkgLog->appendPlainText(QStringLiteral("[错误] 打包正在进行中，请等待完成。"));
        return;
    }

    QString exe = m_pkgExeEdit->text().trimmed();
    QString dir = m_pkgDirEdit->text().trimmed();
    QString wd  = m_pkgWindeployqtEdit->text().trimmed();

    if (exe.isEmpty() || !QFileInfo::exists(exe)) {
        m_pkgLog->appendPlainText(QStringLiteral("[错误] 请先选择有效的目标 exe 文件。"));
        return;
    }
    if (dir.isEmpty()) {
        m_pkgLog->appendPlainText(QStringLiteral("[错误] 请先选择输出目录。"));
        return;
    }
    if (wd.isEmpty()) wd = detectWindeployqt();
    if (wd.isEmpty() || !QFileInfo::exists(wd)) {
        m_pkgLog->appendPlainText(QStringLiteral("[错误] 未找到 windeployqt.exe，请手动选择其路径。"));
        return;
    }

    // 确保输出目录存在
    if (!QDir().mkpath(dir)) {
        m_pkgLog->appendPlainText(QStringLiteral("[错误] 无法创建输出目录: ") + dir);
        return;
    }

    // 若 exe 不在输出目录，先复制过去（windeployqt 需要 exe 在目标目录内打包依赖）
    QString targetExe = dir + QLatin1Char('/') + QFileInfo(exe).fileName();
    if (QFileInfo::exists(targetExe)) QFile::remove(targetExe);
    if (QFileInfo(exe).canonicalFilePath() != QFileInfo(targetExe).canonicalFilePath()) {
        if (!QFile::copy(exe, targetExe)) {
            m_pkgLog->appendPlainText(QStringLiteral("[错误] 复制 exe 到输出目录失败: ") + targetExe);
            return;
        }
        m_pkgLog->appendPlainText(QStringLiteral("[信息] 已复制: ") + targetExe);
    }

    m_pkgOutputDir = dir;
    m_pkgLog->clear();
    m_pkgLog->appendPlainText(QStringLiteral("开始打包..."));
    m_pkgLog->appendPlainText(wd + QStringLiteral(" --dir \"") + dir + QStringLiteral("\" \"") + targetExe + QStringLiteral("\""));
    m_pkgRunBtn->setEnabled(false);
    m_pkgOpenBtn->setVisible(false);

    m_pkgProc = new QProcess(this);
    connect(m_pkgProc, &QProcess::readyReadStandardOutput, this, &MainWindow::onPkgOutput);
    connect(m_pkgProc, &QProcess::readyReadStandardError,  this, &MainWindow::onPkgOutput);
    connect(m_pkgProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onPkgFinished);
    m_pkgProc->setProcessChannelMode(QProcess::MergedChannels);
    m_pkgProc->start(wd, {QStringLiteral("--dir"), dir, targetExe});
}

void MainWindow::onPkgOutput() {
    if (!m_pkgProc) return;
    QByteArray out = m_pkgProc->readAll();
    m_pkgLog->moveCursor(QTextCursor::End);
    m_pkgLog->insertPlainText(QString::fromLocal8Bit(out));
    m_pkgLog->moveCursor(QTextCursor::End);
}

void MainWindow::onPkgFinished() {
    TRACE;
    if (!m_pkgProc) return;
    int code = m_pkgProc->exitCode();
    QProcess::ExitStatus status = m_pkgProc->exitStatus();
    m_pkgProc->deleteLater();
    m_pkgProc = nullptr;

    m_pkgRunBtn->setEnabled(true);
    if (status == QProcess::NormalExit && code == 0) {
        m_pkgLog->appendPlainText(QStringLiteral("\n[完成] 打包成功！点击\"打开文件夹\"查看产物。"));
        m_pkgOpenBtn->setVisible(true);
        ui->statusbar->showMessage(QStringLiteral("打包完成"));
    } else {
        m_pkgLog->appendPlainText(QStringLiteral("\n[失败] 打包异常退出 (code=%1)").arg(code));
        ui->statusbar->showMessage(QStringLiteral("打包失败"));
    }
}

void MainWindow::onPkgOpenFolder() {
    TRACE;
    if (m_pkgOutputDir.isEmpty()) return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_pkgOutputDir));
}

void MainWindow::onSerialStatusChanged(const QString &msg) {
    TRACE << msg;
    ui->statusbar->showMessage(msg);
}

void MainWindow::onSkyBrowse() {
    TRACE;
    // 记忆上次浏览路径，下次直接定位到上次目录
    QSettings fset(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString lastDir = fset.value(QStringLiteral("filedialog/sky/lastDir")).toString();
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择天空/中继端固件"), lastDir,
                                                QStringLiteral("所有文件 (*)"));
    if (!path.isEmpty()) {
        uiFirmware->editSkyPath->setText(path);
        fset.setValue(QStringLiteral("filedialog/sky/lastDir"), QFileInfo(path).absolutePath());
    }
}

void MainWindow::onGroundBrowse() {
    TRACE;
    // 记忆上次浏览路径
    QSettings fset(QStringLiteral("lkwei"), QStringLiteral("fpv-pc-tool"));
    QString lastDir = fset.value(QStringLiteral("filedialog/ground/lastDir")).toString();
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("选择地面端固件"), lastDir,
                                                QStringLiteral("所有文件 (*)"));
    if (!path.isEmpty()) {
        uiFirmware->editGroundPath->setText(path);
        fset.setValue(QStringLiteral("filedialog/ground/lastDir"), QFileInfo(path).absolutePath());
    }
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

    // 功能键区：使用该按键固有按压类型（KEY_PRESS_TYPES 默认值），
    // 通过传 -1 让 createKeyCommand 走默认，避免被自定义区的下拉框干扰
    QByteArray packet = m_protocol->createKeyCommand(name, -1);
    logSend(QString("Key:%1 type:%2").arg(name).arg(-1), packet);
    m_comm->sendData(packet);
}

// 无线参数页用户主动操作时调用：解除快捷指令抑制并清空无线历史缓冲
void MainWindow::resetWirelessDisplay() {
    m_wirelessSuppress = false;
    m_wirelessBuf.clear();
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
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    int band = uiWireless->comboBand->currentIndex();
    int channel = uiWireless->spinChannel->value() - 1;  // UI显示1开头, 实际下发从0开始
    int hop = uiWireless->comboHop->currentIndex();
    QByteArray values;
    values.append(static_cast<char>(band));
    values.append(static_cast<char>(channel));
    values.append(static_cast<char>(hop));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_CHANNEL_HOP, values);
    previewWirelessCmd(WIRELESS_DATA_TYPE_SET_CHANNEL_HOP, values, QStringLiteral("Set Freq"));
    logSend(QString("SetFreq band=%1 ch=%2 hop=%3").arg(band).arg(channel).arg(hop), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetFreq() {
    TRACE;
    if (checkSerialConnected("getFreq")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_CHANNEL_HOP, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_CHANNEL_HOP, QByteArray(), QStringLiteral("Get Freq"));
    logSend("GetFreq", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetStatus() {
    TRACE;
    if (checkSerialConnected("getStatus")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_STATUS, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_STATUS, QByteArray(), QStringLiteral("Get Status Gnd"));
    logSend("GetStatus(Gnd)", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetBbPwr() {
    TRACE;
    if (checkSerialConnected("getBbPwr")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_BB_PWR, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_BB_PWR, QByteArray(), QStringLiteral("Get BB Pwr"));
    logSend("GetBbPwr", packet);
    m_comm->sendData(packet);
}

void MainWindow::onSetBbPwr() {
    TRACE;
    if (checkSerialConnected("setBbPwr")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    int idx = uiWireless->comboBbPwr->currentData().toInt();
    QByteArray values;
    values.append(static_cast<char>(idx));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_BB_PWR, values);
    previewWirelessCmd(WIRELESS_DATA_TYPE_SET_BB_PWR, values, QStringLiteral("Set BB Pwr"));
    logSend(QString("SetBbPwr idx=%1").arg(idx), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetStatusSky() {
    TRACE;
    if (checkSerialConnected("getStatusSky")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_STATUS_SKY, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_STATUS_SKY, QByteArray(), QStringLiteral("Get Status Sky"));
    logSend("GetStatus(Sky)", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetDistance() {
    TRACE;
    if (checkSerialConnected("getDistance")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_DISTANCE, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_DISTANCE, QByteArray(), QStringLiteral("Get Distance"));
    logSend("GetDistance", packet);
    m_comm->sendData(packet);
}

// =====================================================================
// 中继(Relay)无线参数槽函数  (0xA0-0xA7)
// =====================================================================
void MainWindow::onSetRelayFreq() {
    TRACE;
    if (checkSerialConnected("setRelayFreq")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    int band = uiWireless->comboRelayBand->currentIndex();
    int channel = uiWireless->spinRelayChannel->value() - 1;  // UI显示1开头, 实际下发从0开始
    int hop = uiWireless->comboRelayHop->currentIndex();
    QByteArray values;
    values.append(static_cast<char>(band));
    values.append(static_cast<char>(channel));
    values.append(static_cast<char>(hop));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_RELAY_CHANNEL_HOP, values);
    previewWirelessCmd(WIRELESS_DATA_TYPE_SET_RELAY_CHANNEL_HOP, values, QStringLiteral("Set Relay Freq"));
    logSend(QString("SetRelayFreq band=%1 ch=%2 hop=%3").arg(band).arg(channel).arg(hop), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetRelayFreq() {
    TRACE;
    if (checkSerialConnected("getRelayFreq")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_RELAY_CHANNEL_HOP, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_RELAY_CHANNEL_HOP, QByteArray(), QStringLiteral("Get Relay Freq"));
    logSend("GetRelayFreq", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetRelayStatus() {
    TRACE;
    if (checkSerialConnected("getRelayStatus")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_RELAY_STATUS, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_RELAY_STATUS, QByteArray(), QStringLiteral("Get Relay Status"));
    logSend("GetRelayStatus", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetRelayBbPwr() {
    TRACE;
    if (checkSerialConnected("getRelayBbPwr")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_RELAY_BB_PWR, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_RELAY_BB_PWR, QByteArray(), QStringLiteral("Get Relay BB Pwr"));
    logSend("GetRelayBbPwr", packet);
    m_comm->sendData(packet);
}

void MainWindow::onSetRelayBbPwr() {
    TRACE;
    if (checkSerialConnected("setRelayBbPwr")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    int idx = uiWireless->comboRelayBbPwr->currentData().toInt();
    QByteArray values;
    values.append(static_cast<char>(idx));
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_SET_RELAY_BB_PWR, values);
    previewWirelessCmd(WIRELESS_DATA_TYPE_SET_RELAY_BB_PWR, values, QStringLiteral("Set Relay BB Pwr"));
    logSend(QString("SetRelayBbPwr idx=%1").arg(idx), packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetRelayGndDist() {
    TRACE;
    if (checkSerialConnected("getRelayGndDist")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_RELAY_GND_DISTANCE, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_RELAY_GND_DISTANCE, QByteArray(), QStringLiteral("Get Relay Gnd Dist"));
    logSend("GetRelayGndDist", packet);
    m_comm->sendData(packet);
}

void MainWindow::onGetRelaySkyDist() {
    TRACE;
    if (checkSerialConnected("getRelaySkyDist")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray packet = m_protocol->createWirelessCommand(WIRELESS_DATA_TYPE_GET_RELAY_SKY_DISTANCE, QByteArray());
    previewWirelessCmd(WIRELESS_DATA_TYPE_GET_RELAY_SKY_DISTANCE, QByteArray(), QStringLiteral("Get Relay Sky Dist"));
    logSend("GetRelaySkyDist", packet);
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
}

// =====================================================================
// 无线参数页 0x57 OSD 获取（输出到无线页底部打印框 textStatus）
// =====================================================================
void MainWindow::onGetOsdDataWireless() {
    TRACE;
    if (checkSerialConnected("getOsdDataWireless")) return;
    resetWirelessDisplay();   // user wireless op: clear suppress + buf
    QByteArray values;
    QByteArray packet = m_protocol->createWirelessCommand(
        static_cast<wireless_data_type_t>(OSD_CMD_TYPE), values);  // OSD_CMD_TYPE = 0x57
    previewWirelessCmd(static_cast<wireless_data_type_t>(OSD_CMD_TYPE), values, QStringLiteral("Get OSD Data (0x57)"));
    logSend("GetOsdDataWireless", packet);
    m_comm->sendData(packet);

    // ---- 在无线页底部打印框显示协议格式解析 ----
    QString fmt;
    fmt += "===== 获取OSD数据请求协议包 (0x57) =====\n";
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
    uiWireless->textStatus->append(fmt);
    QTextCursor c = uiWireless->textStatus->textCursor();
    c.movePosition(QTextCursor::End);
    uiWireless->textStatus->setTextCursor(c);
}

void MainWindow::onClearOsdDataWireless() {
    TRACE;
    m_wirelessBuf.clear();
    uiWireless->textStatus->clear();
}

void MainWindow::onClearOsd() {
    TRACE;
    memset(m_osdCharMap, 0, sizeof(m_osdCharMap));
    m_osdGrid->clear();
    uiOsd->textOsdInfo->clear();
    m_osdReassemblyBuf.clear();
    m_osdExpectedLen = 0;
}

void MainWindow::onAutoPollToggled(bool checked) {
    TRACE << "checked=" << checked;
    if (checked) {
        // 未连接时启动定时器但提示一次，不弹对话框刷屏
        if (!m_comm->isConnected()) {
            uiOsd->textOsdInfo->append("⚠ 串口未连接，轮询将在连接后生效");
        }
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
    case 0: // Auto — 负载均衡：按 Betaflight 处理（最通用）
        loaded = loadOsdFont(QString("font_bf_%1.png").arg(fontPixel), fontPixel);
        if (!loaded) {
            // 回退：尝试所有字体名
            QStringList fallbacks = {
                "font_bf_%1.png", "font_inav_%1.png", "font_ardu_%1.png",
                "font_FTTC%1.png", "font_kiss_%1.png", "WS_QUIC_%1.png"
            };
            for (const QString &fb : fallbacks) {
                QString name = fb.arg(fontPixel);
                uiOsd->textOsdInfo->append(QString("Auto 尝试: %1").arg(name));
                if (loadOsdFont(name, fontPixel)) {
                    loaded = true;
                    break;
                }
            }
        }
        break;
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
        loaded = m_osdGrid->loadCharImages(":/osdchars", 512);
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
        m_osdGrid->loadCharImages(":/osdchars", 512);
    }
}

void MainWindow::onOsdResolutionChanged(int /*index*/) {
    // 分辨率改变 → 重新加载当前飞控字体
    onFcTypeChanged(uiOsd->cboFcType->currentIndex());
}

void MainWindow::onPollTimer() {
    if (m_comm->isConnected()) {
        onGetOsdData();
    }
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
    const int expectedMapSize = 20 * 53 * 2; // 2120

    if (osdPayloadSize >= expectedMapSize) {
        // 解析 character_map
        const quint8 *data = reinterpret_cast<const quint8*>(osdPayload.constData());
        for (int row = 0; row < 20; ++row) {
            for (int col = 0; col < 53; ++col) {
                int idx = row * 53 + col;
                // 小端 unsigned short
                quint16 val = data[idx * 2] | (data[idx * 2 + 1] << 8);
                m_osdCharMap[row][col] = val;
            }
        }
        m_osdGrid->setCharacterMap(m_osdCharMap);

        // ---- 在数据详情输出 OSD 20x53 矩阵（与快捷指令页 printOsdMatrix 一致）----
        uiOsd->textOsdInfo->append("--- OSD 20x53 矩阵 ---");
        for (int row = 0; row < 20; ++row) {
            QString line;
            bool hasContent = false;
            for (int col = 0; col < 53; ++col) {
                quint16 v = m_osdCharMap[row][col];
                quint8 ch = static_cast<quint8>(v & 0xFF);
                if (ch == 0) {
                    line += ' ';
                } else if (ch >= 0x20 && ch <= 0x7E) {
                    line += QChar(ch);
                    hasContent = true;
                } else {
                    line += QStringLiteral("[%1]").arg(ch, 2, 16, QChar('0'));
                    hasContent = true;
                }
            }
            if (hasContent)
                uiOsd->textOsdInfo->append(QStringLiteral("R %1: %2").arg(row).arg(line));
        }
        uiOsd->textOsdInfo->append("--- 矩阵结束 ---");

        // 统计非零字符（活跃字符数）
        int activeCount = 0;
        quint16 minVal = 0xFFFF, maxVal = 0;
        for (int i = 0; i < 20 * 53; ++i) {
            quint16 v = reinterpret_cast<const quint16*>(osdPayload.constData())[i];
            if (v != 0) {
                activeCount++;
                if (v < minVal) minVal = v;
                if (v > maxVal) maxVal = v;
            }
        }

        uiOsd->textOsdInfo->append(
            QString("[%1] OSD character_map: %2 bytes | 活跃字符: %3/%4 | 索引: %5~%6")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                .arg(osdPayloadSize)
                .arg(activeCount)
                .arg(20 * 53)
                .arg(minVal == 0xFFFF ? 0 : minVal)
                .arg(maxVal));
    } else if (osdPayloadSize < 100) {
        // 可能是简短的回复（查询确认），显示到信息框
        QString hexStr = QString::fromLatin1(osdPayload.toHex(' ').toUpper());
        uiOsd->textOsdInfo->append(
            QString("[%1] OSD 回复 (%2 bytes): %3")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                .arg(osdPayloadSize)
                .arg(hexStr));
    } else {
        // 数据大小不对（既不是 character_map 也不是简短回复）
        // 此时 visualizeRawOsdData 已经通过重组缓冲区解析了 MSP 数据
        // 这里不再重复显示，避免干扰 OSD 信息区
        qDebug() << "[OSD] onOsdDataReceived: 跳过非 characteristic_map 数据, payloadSize=" << osdPayloadSize;
    }
}

void MainWindow::previewWirelessCmd(wireless_data_type_t type, const QByteArray &values, const QString &name) {
    TRACE;
    QByteArray packet = m_protocol->createWirelessCommand(type, values);
    m_previewType = static_cast<int>(type);
    uiWireless->editCmdPreview->setText(
        QStringLiteral("%1 : %2").arg(name).arg(QString::fromLatin1(packet.toHex(' ').toUpper())));
}

void MainWindow::onUpdatePreviewFromParams() {
    TRACE;
    if (m_previewType < 0) {
        uiWireless->editCmdPreview->clear();
        return;
    }
    int band     = uiWireless->comboBand->currentIndex();
    int channel  = uiWireless->spinChannel->value() - 1;     // UI显示1开头, 实际下发从0开始
    int hop      = uiWireless->comboHop->currentIndex();
    int bbPwr    = uiWireless->comboBbPwr->currentText().split(' ').at(0).toInt();
    int rBand    = uiWireless->comboRelayBand->currentIndex();
    int rChannel = uiWireless->spinRelayChannel->value() - 1; // UI显示1开头, 实际下发从0开始
    int rHop     = uiWireless->comboRelayHop->currentIndex();
    int rBbPwr   = uiWireless->comboRelayBbPwr->currentText().split(' ').at(0).toInt();

    QByteArray v;
    QString name;
    switch (m_previewType) {
    case WIRELESS_DATA_TYPE_SET_CHANNEL_HOP:
        v.append(static_cast<char>(band)); v.append(static_cast<char>(channel)); v.append(static_cast<char>(hop));
        name = QStringLiteral("Set Freq");
        break;
    case WIRELESS_DATA_TYPE_SET_BB_PWR:
        v.append(static_cast<char>(bbPwr));
        name = QStringLiteral("Set BB Pwr");
        break;
    case WIRELESS_DATA_TYPE_SET_RELAY_CHANNEL_HOP:
        v.append(static_cast<char>(rBand)); v.append(static_cast<char>(rChannel)); v.append(static_cast<char>(rHop));
        name = QStringLiteral("Set Relay Freq");
        break;
    case WIRELESS_DATA_TYPE_SET_RELAY_BB_PWR:
        v.append(static_cast<char>(rBbPwr));
        name = QStringLiteral("Set Relay BB Pwr");
        break;
    default:
        // 无参数命令：参数变化不改变包内容
        return;
    }
    previewWirelessCmd(static_cast<wireless_data_type_t>(m_previewType), v, name);
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

    // 把收到的原始数据追加到重组缓冲区
    m_osdReassemblyBuf.append(data);

    QString hexStr = QString::fromLatin1(data.toHex(' ').toUpper());

    // 调试：打印原始数据到 qDebug
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    qDebug().noquote() << QString("[%1] ===== 收到原始数据 %2 bytes =====")
                              .arg(timestamp)
                              .arg(data.size());
    qDebug().noquote() << QString("HEX: %1").arg(hexStr);
    qDebug().noquote() << QString("累计: %1 bytes").arg(m_osdReassemblyBuf.size());

    // 打印到 OSD 信息框
    uiOsd->textOsdInfo->append(QString("[%1] RX: %2 bytes (累计 %3)")
                                   .arg(QDateTime::currentDateTime().toString("HH:mm:ss.zzz"))
                                   .arg(data.size())
                                   .arg(m_osdReassemblyBuf.size()));

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

    // ======== MSP 分包重组（容错版） ========
    // 扫描缓冲区，提取 OSD 数据包
    // 格式: FE EF cmd(1) lenH(1) lenL(1) [data_content...] [csH csL] [0D 0A]
    // 关键：不能死等 0D 0A，有时设备不发尾部；只要 data_content 到齐就应该解析
    m_osdRawData = m_osdReassemblyBuf; // 同步全部缓存数据供 visualizeRawOsdData

    while (m_osdReassemblyBuf.size() >= 7) {
        // 找 FE EF 头
        int headerPos = -1;
        for (int i = 0; i < m_osdReassemblyBuf.size() - 1; ++i) {
            if ((quint8)m_osdReassemblyBuf.at(i) == 0xFE && (quint8)m_osdReassemblyBuf.at(i + 1) == 0xEF) {
                headerPos = i;
                break;
            }
        }
        if (headerPos < 0) {
            m_osdReassemblyBuf.clear();
            break;
        }
        if (headerPos > 0) {
            m_osdReassemblyBuf.remove(0, headerPos);
        }

        const quint8 *raw = reinterpret_cast<const quint8*>(m_osdReassemblyBuf.constData());
        if (m_osdReassemblyBuf.size() < 9) break;

        if (raw[2] != 0xA2) {
            m_osdReassemblyBuf.remove(0, 5);
            continue;
        }
        int declaredLen = (raw[3] << 8) | raw[4];
        if (declaredLen <= 0 || declaredLen > 2048) {
            m_osdReassemblyBuf.remove(0, 5);
            continue;
        }

        // 最少需要的字节：5 (header) + declaredLen (data_content) = data_content 到齐即可
        int minPktSize = 5 + declaredLen;
        // 完整包尺寸（含 checksum + footer 2+2）
        int fullPktSize = minPktSize + 2 + 2;

        // data_content 是否到齐
        bool contentReady = (m_osdReassemblyBuf.size() >= minPktSize);
        // 是不是 OSD 包（data_content[0] == 0x57）
        bool isOsdPkt = (raw[5] == 0x57);

        if (!contentReady) {
            if (isOsdPkt) {
                qDebug().noquote() << QString("  ⏳ OSD 数据等待: data_content %1/%2")
                                      .arg(m_osdReassemblyBuf.size() - 5).arg(declaredLen);
            }
            break; // data_content 没到齐，下次再说
        }

        // data_content 已到齐，确认是不是 OSD
        if (!isOsdPkt) {
            // 非 OSD 包，直接移除
            int removeSize = qMin(fullPktSize, m_osdReassemblyBuf.size());
            m_osdReassemblyBuf.remove(0, removeSize);
            continue;
        }

        // ===== OSD 包！data_content 已到齐，立即解析 =====

        // 校验 checksum
        // 注意：设备端 OSD 包可能不包含 data_content[0] (cmd_type=0x57)
        // 所以先尝试全算，不匹配则跳过 cmd_type 再试
        bool csValid = false;
        bool footerValid = false;
        if (m_osdReassemblyBuf.size() >= fullPktSize) {

            // 有完整的包（含 checksum + 0D 0A），可以做校验
            quint16 calcCs = 0;
            for (int i = 0; i < declaredLen; i++) {
                calcCs += raw[5 + i];
            }
            quint16 pktCs = (raw[minPktSize] << 8) | raw[minPktSize + 1];
            csValid = (calcCs == pktCs);
            if (!csValid && declaredLen >= 2) {
                // 兼容：跳过 data_content[0] (cmd_type) 再算一次
                calcCs = 0;
                for (int i = 1; i < declaredLen; i++) {
                    calcCs += raw[5 + i];
                }
                csValid = (calcCs == pktCs);
            }

            footerValid = (raw[minPktSize + 2] == 0x0D && raw[minPktSize + 3] == 0x0A);
            qDebug().noquote() << QString("  checksum: calc=0x%1 pkt=0x%2 %3, footer 0D 0A: %4")
                                  .arg(calcCs, 4, 16, QChar('0'))
                                  .arg(pktCs, 4, 16, QChar('0'))
                                  .arg(csValid ? "✓" : "✗")
                                  .arg(footerValid ? "✓" : "✗");
        } else {
            // 只有 data_content 到齐但尾部还没到，继续等待完整包
            qDebug().noquote() << QString("  ⏳ 等待尾部: %1/%2")
                                  .arg(m_osdReassemblyBuf.size()).arg(fullPktSize);
            break;
        }

        if (!csValid) {
            qDebug().noquote() << "  ⚠ checksum 不匹配，丢弃";
            int removeSize = qMin(fullPktSize, m_osdReassemblyBuf.size());
            m_osdReassemblyBuf.remove(0, removeSize);
            continue;
        }

        if (!footerValid) {
            qDebug().noquote() << "  ⚠ 尾部 0D 0A 缺失，丢弃";
            int removeSize = qMin(fullPktSize, m_osdReassemblyBuf.size());
            m_osdReassemblyBuf.remove(0, removeSize);
            continue;
        }

        qDebug().noquote() << QString("✅ OSD 包校验通过: declaredLen=%1, m_osdReassemblyBuf=%2")
                              .arg(declaredLen).arg(m_osdReassemblyBuf.size());

        // 用完整的 FE EF A2... 来解析：从 buffer 取 minPktSize 字节
        QByteArray osdPkt = m_osdReassemblyBuf.mid(0, minPktSize);
        m_osdRawData = osdPkt;

        // 从重组缓冲区移除已消费的数据
        m_osdReassemblyBuf.remove(0, fullPktSize);

        visualizeRawOsdData();
        break;
    }

    // 解析数据包，如果是无线查询/设置响应则显示到 textStatus
    // 注意：串口数据可能被拆成多段 readyRead，之前用单次 data 解析，
    // 分包时首包不完整会直接 return，导致“第一次点击没反应、再点一次才有”。
    // 修复：用 m_wirelessBuf 累积，再从缓冲中提取完整包解析。
    m_wirelessBuf.append(data);

    QVariantMap parsed;
    quint8 cmd = 0;
    // 从累积缓冲中尝试提取一个完整包（FE EF cmd lenH lenL ... csH csL 0D 0A）
    while (m_wirelessBuf.size() >= 9) {
        // 定位 FE EF 头
        int headerPos = -1;
        for (int i = 0; i < m_wirelessBuf.size() - 1; ++i) {
            if ((quint8)m_wirelessBuf.at(i) == 0xFE && (quint8)m_wirelessBuf.at(i + 1) == 0xEF) {
                headerPos = i;
                break;
            }
        }
        if (headerPos < 0) { m_wirelessBuf.clear(); break; }
        if (headerPos > 0) m_wirelessBuf.remove(0, headerPos);

        const quint8 *raw = reinterpret_cast<const quint8*>(m_wirelessBuf.constData());
        if (m_wirelessBuf.size() < 9) break;

        int dataLen = (raw[3] << 8) | raw[4];
        if (dataLen <= 0 || dataLen > 2048) { m_wirelessBuf.remove(0, 5); continue; }

        int fullSize = 5 + dataLen + 2 + 2; // 头5 + data_content + checksum2 + footer2
        if (m_wirelessBuf.size() < fullSize) break; // 等完整包

        // 取完整包解析
        QByteArray full = m_wirelessBuf.mid(0, fullSize);
        m_wirelessBuf.remove(0, fullSize);

        parsed = m_protocol->parsePacket(full);
        if (parsed.isEmpty()) continue; // 包损坏，继续找下一个

        cmd = static_cast<quint8>(parsed.value("command").toUInt());
        // 无线命令响应的 cmd 为 0xA2(查询) 或 0x22(设置)
        if (cmd != 0xA2 && cmd != 0x22) continue;
        break; // 找到有效的无线响应
    }

    QByteArray content = parsed.value("data_content").toByteArray();
    if (content.isEmpty()) return;

    quint8 subCmd = static_cast<quint8>(content.at(0));

    // ---- 显示无线参数响应到 textStatus ----
    QString displayText;
    displayText += QString("命令: 0x%1 响应\n").arg(subCmd, 2, 16, QChar('0'));
    displayText += QString("原始数据: %1\n").arg(QString::fromLatin1(content.toHex(' ').toUpper()));

    switch (subCmd) {
    case 0x50: // 设置频点响应 (error_payload: cmd_type error_code error_param_index error_param_value reserved[4])
        {
            int errCode = (content.size() >= 2) ? static_cast<qint8>(content.at(1)) : -1;
            if (errCode == 0)
                displayText += QStringLiteral("频点设置完成\n");
            else
                displayText += QString("频点设置失败, 错误码=%1\n").arg(errCode);
        }
        break;
    case 0x51: // 获取频点响应
        if (content.size() >= 4) {
            int band    = static_cast<quint8>(content.at(1));
            int channel = static_cast<quint8>(content.at(2));
            int hop     = static_cast<quint8>(content.at(3));
            // 回填频点参数到界面 (UI显示1开头, 实际值0开头)
            uiWireless->comboBand->setCurrentIndex(band);
            uiWireless->spinChannel->setValue(channel + 1);
            uiWireless->comboHop->setCurrentIndex(hop);
            displayText += QString("频段: %1 (%2)\n").arg(band).arg(bandToLetter(band));
            displayText += QString("通道: %1\n").arg(channel + 1);
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
            for (int i = 0; i < 20; ++i) {
                if (bitmap & (1U << i)) {
                    combo->addItem(QString::fromUtf8(bbPwrNames[i]), i);
                    settable << QString::fromUtf8(bbPwrNames[i]);
                }
            }
            combo->blockSignals(false);
            // 默认选中当前功率索引
            int curIdx = combo->findData(pwrIdx);
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
                // OSD 可视化已由 visualizeRawOsdData 处理，不再重复调用 onOsdDataReceived
            }
        }
        break;

    // ===== 中继(Relay)命令响应 0xA0-0xA7 =====
    case 0xA0: // 设置中继频点响应 (error_payload: cmd_type error_code error_param_index error_param_value reserved[4])
        {
            int errCode = (content.size() >= 2) ? static_cast<qint8>(content.at(1)) : -1;
            displayText += QStringLiteral("中继频点设置");
            if (errCode == 0)
                displayText += QStringLiteral("完成\n");
            else
                displayText += QString("失败, 错误码=%1\n").arg(errCode);
        }
        break;
    case 0xA1: // 获取中继频点响应
        if (content.size() >= 4) {
            int band     = static_cast<quint8>(content.at(1));
            int channel  = static_cast<quint8>(content.at(2));
            int hop      = static_cast<quint8>(content.at(3));
            // 回填中继频点参数到界面 (UI显示1开头, 实际值0开头)
            uiWireless->comboRelayBand->setCurrentIndex(band);
            uiWireless->spinRelayChannel->setValue(channel + 1);
            uiWireless->comboRelayHop->setCurrentIndex(hop);
            displayText += QString("频段: %1 (%2)\n").arg(band).arg(bandToLetter(band));
            displayText += QString("通道: %1\n").arg(channel + 1);
            displayText += QString("跳频模式: %1\n").arg(hop ? QStringLiteral("跳频") : QStringLiteral("定频"));
        }
        break;
    case 0xA2: // 获取中继状态响应
        // 结构: cmd(0xA2) relay_gnd_rssi[2] relay_sky_rssi[2] relay_ap_bb_connect relay_dev_bb_connect reserved
        if (content.size() >= 7) {
            int gndRssi1 = static_cast<signed char>(content.at(1));
            int gndRssi2 = static_cast<signed char>(content.at(2));
            int skyRssi1 = static_cast<signed char>(content.at(3));
            int skyRssi2 = static_cast<signed char>(content.at(4));
            int apConn   = static_cast<quint8>(content.at(5));
            int devConn  = static_cast<quint8>(content.at(6));
            displayText += QString("地面链路 RSSI1: %1\n").arg(gndRssi1);
            displayText += QString("地面链路 RSSI2: %1\n").arg(gndRssi2);
            displayText += QString("天空链路 RSSI1: %1\n").arg(skyRssi1);
            displayText += QString("天空链路 RSSI2: %1\n").arg(skyRssi2);
            displayText += QString("AP 基带连接: %1\n").arg(apConn ? QStringLiteral("已连接") : QStringLiteral("未连接"));
            displayText += QString("设备基带连接: %1\n").arg(devConn ? QStringLiteral("已连接") : QStringLiteral("未连接"));
        }
        break;
    case 0xA3: // 获取中继BB功率响应
        if (content.size() >= 5) {
            int pwrIdx = static_cast<quint8>(content.at(1));
            quint32 bitmap = (static_cast<quint8>(content.at(2))) |
                             (static_cast<quint8>(content.at(3)) << 8) |
                             (static_cast<quint8>(content.at(4)) << 16) |
                             (static_cast<quint8>(content.at(5)) << 24);
            displayText += QString("中继当前功率索引: %1\n").arg(pwrIdx);
            displayText += QString("可设置位图: 0x%1\n").arg(bitmap, 8, 16, QChar('0'));

            // 更新中继功率索引下拉框
            QComboBox *combo = uiWireless->comboRelayBbPwr;
            combo->blockSignals(true);
            combo->clear();
            QStringList settable;
            for (int i = 0; i < 20; ++i) {
                if (bitmap & (1U << i)) {
                    combo->addItem(QString::fromUtf8(bbPwrNames[i]), i);
                    settable << QString::fromUtf8(bbPwrNames[i]);
                }
            }
            combo->blockSignals(false);
            // 默认选中当前功率索引
            int curIdx = combo->findData(pwrIdx);
            if (curIdx >= 0) combo->setCurrentIndex(curIdx);

            if (!settable.isEmpty())
                displayText += QString("可设置功率: %1\n").arg(settable.join(", "));
        }
        break;
    case 0xA4: // 设置中继BB功率响应 (error_payload 格式)
        {
            int errCode = (content.size() >= 2) ? static_cast<qint8>(content.at(1)) : -1;
            displayText += QStringLiteral("中继BB功率设置");
            if (errCode == 0)
                displayText += QStringLiteral("完成\n");
            else
                displayText += QString("失败, 错误码=%1\n").arg(errCode);
        }
        break;
    case 0xA5: // 获取中继-地面距离响应 (小端32位)
        if (content.size() >= 5) {
            qint32 dist = static_cast<qint8>(content.at(1)) |
                          (static_cast<quint8>(content.at(2)) << 8) |
                          (static_cast<quint8>(content.at(3)) << 16) |
                          (static_cast<quint8>(content.at(4)) << 24);
            displayText += QString("中继-地面距离: %1 m\n").arg(dist);
        }
        break;
    case 0xA6: // 获取中继-天空距离响应 (小端32位)
        if (content.size() >= 5) {
            qint32 dist = static_cast<qint8>(content.at(1)) |
                          (static_cast<quint8>(content.at(2)) << 8) |
                          (static_cast<quint8>(content.at(3)) << 16) |
                          (static_cast<quint8>(content.at(4)) << 24);
            displayText += QString("中继-天空距离: %1 m\n").arg(dist);
        }
        break;
    case 0xA7: // 中继OSD数据响应
        {
            int osdPayloadSize = content.size() - 1;
            displayText += QString("中继OSD数据: %1 bytes\n").arg(osdPayloadSize);
            if (osdPayloadSize > 0) {
                displayText += QString("首字节: 0x%1\n")
                               .arg(static_cast<quint8>(content.at(1)), 2, 16, QChar('0'));
            }
        }
        break;
    default:
        displayText += QStringLiteral("未知命令类型\n");
        break;
    }

    if (!m_wirelessSuppress) {
        uiWireless->textStatus->append(displayText);
        // 自动滚动到最新
        QTextCursor c = uiWireless->textStatus->textCursor();
        c.movePosition(QTextCursor::End);
        uiWireless->textStatus->setTextCursor(c);
    }
    // 快捷指令(F0)接收期间(m_wirelessSuppress=true)不打印到无线 textStatus，
    // 但仍完成了组合框更新（0x51频点/0x53功率等），满足"快捷指令数据更新到频率/功率设置"。
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
    uiWireless->groupRelay->setEnabled(connected);
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
    if (m_comm->isConnected()) {
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
        // MSP V2 over-the-air 格式:
        //   $X > [flags(1)] [cmd(2) LE] [len(2) LE] [payload...] [crc(1)]
        //   $X M [flags(1)] [cmd(2) LE] [len(2) LE] [payload...]
        // cmd=0x00B6=182 = MSP_CMD_DISPLAYPORT
        int dataStart = 0;
        bool hasMspV2Crc = false;
        int mspV2PayloadLen = 0;
        if (osdSize >= 9 && osd[0] == 0x24 && osd[1] == 0x58) {
            // $X 头部: 2(sig) + 1(dir) + 1(flags) + 2(cmd_LE) + 2(len_LE) = 8
            dataStart = 8;
            int mspCmd = osd[3] | (osd[4] << 8);   // cmd LE
            mspV2PayloadLen = osd[5] | (osd[6] << 8); // len LE
            // MSP V2 inbound (0x3E='>') 末尾有 1 byte CRC
            if (osd[2] == 0x3E) {
                hasMspV2Crc = true;
            }
            qDebug().noquote() << QString("  跳过 MSP V2 头: cmd=0x%1 len=%2 crc=%3")
                                  .arg(mspCmd, 4, 16, QChar('0'))
                                  .arg(mspV2PayloadLen)
                                  .arg(hasMspV2Crc ? "yes" : "no");
        }

        // 如果 MSP V2 包末尾有 CRC 字节，则截掉它以免被误解析为子命令
        int maxParseEnd = osdSize;
        if (hasMspV2Crc && mspV2PayloadLen > 0) {
            int supposedEnd = dataStart + mspV2PayloadLen;
            if (supposedEnd < maxParseEnd) {
                maxParseEnd = supposedEnd;
            }
        }

        // 解析 MSP DISPLAYPORT 格式的 OSD 数据
        // 参考 SDK fcmsp_proc.cpp:
        //   0x35 [seq] [rsv] -- 后续 entries: [len][row][col][attr][chars...]
        //   0x05 [row] [col] [attr] [chars...]  -- MSP_DISPLAY_TEXT (不带 len!)
        //   0x09 DRAW_SCREEN
        //   0x0A CLEAR_SCREEN
        //
        // attrib mask: SDK 使用 & 0x03 (低位2bit 映射到 font page)
        //             必须是 (rawAttr & 0x03) << 8，而不是 0x0f
        int parsedCount = 0;
        int p = dataStart;

        while (p < maxParseEnd) {
            quint8 subcmd = osd[p];

            if (subcmd == 0x35) {
                // MSP_DISPLAY_DRAW_NORMAL_FULL_PKT
                if (p + 7 > maxParseEnd) break;
                int entryStart = p + 3; // 跳过 0x35 [seq] [rsv]
                int entryPos = entryStart;
                while (entryPos + 4 <= maxParseEnd) {
                    int entryLen = osd[entryPos];
                    if (entryLen < 4) break;
                    int textLen = entryLen - 4;
                    if (entryPos + entryLen > maxParseEnd) break;
                    int row = osd[entryPos + 1];
                    int col = osd[entryPos + 2];
                    int rawAttr = osd[entryPos + 3];
                    unsigned short attrs = (rawAttr & 0x03) << 8;  // SDK: & 0x03
                    if (row >= 0 && row < 20 && col >= 0 && col < 53) {
                        for (int i = 0; i < textLen && (col + i) < 53; ++i) {
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
                // SDK 处理: attrs = ((msg->payload[3] & 0x03) << 8)
                if (p + 4 > maxParseEnd) break;
                int row = osd[p + 1];
                int col = osd[p + 2];
                int rawAttr = osd[p + 3];
                unsigned short attrs = (rawAttr & 0x03) << 8;  // SDK: & 0x03
                // 文本从 pos+4 开始，直到 maxParseEnd 或下一个子命令开头
                int textStart = p + 4;
                int textEnd;
                for (textEnd = textStart; textEnd < maxParseEnd; textEnd++) {
                    quint8 b = osd[textEnd];
                    if (b == 0x00 || b == 0x05 || b == 0x09 || b == 0x0A || b == 0x35 || b >= 0x80)
                        break; // 遇到下一个子命令或非文本字节
                }
                int textLen = textEnd - textStart;
                if (row >= 0 && row < 20) {
                    for (int i = 0; i < textLen && (col + i) < 53; ++i) {
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
                if (p + entryLen <= maxParseEnd) {
                    int textLen = entryLen - 4;
                    if (textLen > 0 && textLen < 60) {
                        int row = osd[p + 1];
                        int col = osd[p + 2];
                        int rawAttr = osd[p + 3];
                        unsigned short attrs = (rawAttr & 0x03) << 8;  // SDK: & 0x03
                        if (row >= 0 && row < 20 && col >= 0 && col < 53) {
                            for (int i = 0; i < textLen && (col + i) < 53; ++i) {
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
            for (int row = 0; row < 20; ++row) {
                QString line;
                for (int col = 0; col < 53; ++col) {
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

            // ---- 在数据详情打印 OSD 20x53 矩阵（快捷指令页 printOsdMatrix 同款格式）----
            uiOsd->textOsdInfo->append("--- OSD 20x53 矩阵 ---");
            for (int row = 0; row < 20; ++row) {
                QString line;
                bool hasContent = false;
                for (int col = 0; col < 53; ++col) {
                    quint16 v = m_osdCharMap[row][col];
                    quint8 ch = static_cast<quint8>(v & 0xFF);
                    if (ch == 0) {
                        line += ' ';
                    } else if (ch >= 0x20 && ch <= 0x7E) {
                        line += QChar(ch);
                        hasContent = true;
                    } else {
                        line += QStringLiteral("[%1]").arg(ch, 2, 16, QChar('0'));
                        hasContent = true;
                    }
                }
                if (hasContent)
                    uiOsd->textOsdInfo->append(QStringLiteral("R %1: %2").arg(row).arg(line));
            }
            uiOsd->textOsdInfo->append("--- 矩阵结束 ---");

            uiOsd->textOsdInfo->append(
                QString("✅ OSD: %1 条目 (%2 bytes)").arg(parsedCount).arg(osdSize));

            // 清理已处理的原始数据，避免下次重复解析
            // 注意：只有 parsedCount > 0 而且 osdSize 是完整的 payload 时才清理
            // 如果消费量明显小于总包大小，说明可能不是完整包，不清理
            int consumed = payloadStart + osdSize;
            if (consumed >= m_osdRawData.size()) {
                m_osdRawData.clear();
            } else {
                // 残余字节多半是 checksum + footer，保留也无害，直接清掉
                m_osdRawData.clear();
            }
            return;
        }
    }

    qDebug().noquote() << "未找到 OSD 数据包，当前" << m_osdRawData.size() << "bytes";
}

