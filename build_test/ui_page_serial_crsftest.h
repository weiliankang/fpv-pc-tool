/********************************************************************************
** Form generated from reading UI file 'page_serial_crsftest.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_CRSFTEST_H
#define UI_PAGE_SERIAL_CRSFTEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSerialCrsfTest
{
public:
    QVBoxLayout *crsfMainLayout;
    QGroupBox *groupSetup;
    QGridLayout *gridSetup;
    QLabel *lbPort;
    QComboBox *comboPort;
    QPushButton *btnRefresh;
    QLabel *lbBaud;
    QComboBox *comboBaud;
    QPushButton *btnConnect;
    QLabel *labelStatus;
    QPushButton *btnClearRecv;
    QWidget *splitterMain;
    QHBoxLayout *splitterLayout;
    QGroupBox *groupSend;
    QVBoxLayout *sendLayout;
    QTextEdit *textSend;
    QHBoxLayout *sendBtnLayout;
    QComboBox *comboCrsfType;
    QSpinBox *spinRepeatCount;
    QSpinBox *spinRepeatInterval;
    QPushButton *btnSend;
    QHBoxLayout *templateLayout;
    QPushButton *btnTplRcChannels;
    QPushButton *btnTplLinkStats;
    QPushButton *btnTplHeartbeat;
    QPushButton *btnTplCustom;
    QGroupBox *groupRecv;
    QVBoxLayout *recvLayout;
    QTextEdit *textRecv;
    QHBoxLayout *recvCtrlLayout;
    QCheckBox *checkAutoScroll;
    QCheckBox *checkShowTimestamp;
    QCheckBox *checkHexOnly;
    QSpacerItem *spRecv;
    QPushButton *btnClearRecvData;
    QPushButton *btnSaveRecv;
    QPushButton *btnDiagWindow;
    QGroupBox *groupLatency;
    QHBoxLayout *latencyLayout;
    QLabel *lbLastLatency;
    QLabel *lbAvgLatency;
    QLabel *lbMinLatency;
    QLabel *lbMaxLatency;
    QLabel *lbPktCount;
    QPushButton *btnResetStats;
    QSpinBox *spinStopAt;
    QPushButton *btnStopAtEnable;
    QGroupBox *groupDist;
    QVBoxLayout *distLayout;
    QTextEdit *textDist;

    void setupUi(QWidget *PageSerialCrsfTest)
    {
        if (PageSerialCrsfTest->objectName().isEmpty())
            PageSerialCrsfTest->setObjectName(QString::fromUtf8("PageSerialCrsfTest"));
        crsfMainLayout = new QVBoxLayout(PageSerialCrsfTest);
        crsfMainLayout->setObjectName(QString::fromUtf8("crsfMainLayout"));
        groupSetup = new QGroupBox(PageSerialCrsfTest);
        groupSetup->setObjectName(QString::fromUtf8("groupSetup"));
        gridSetup = new QGridLayout(groupSetup);
        gridSetup->setObjectName(QString::fromUtf8("gridSetup"));
        lbPort = new QLabel(groupSetup);
        lbPort->setObjectName(QString::fromUtf8("lbPort"));

        gridSetup->addWidget(lbPort, 0, 0, 1, 1);

        comboPort = new QComboBox(groupSetup);
        comboPort->setObjectName(QString::fromUtf8("comboPort"));

        gridSetup->addWidget(comboPort, 0, 1, 1, 1);

        btnRefresh = new QPushButton(groupSetup);
        btnRefresh->setObjectName(QString::fromUtf8("btnRefresh"));

        gridSetup->addWidget(btnRefresh, 0, 2, 1, 1);

        lbBaud = new QLabel(groupSetup);
        lbBaud->setObjectName(QString::fromUtf8("lbBaud"));

        gridSetup->addWidget(lbBaud, 1, 0, 1, 1);

        comboBaud = new QComboBox(groupSetup);
        comboBaud->setObjectName(QString::fromUtf8("comboBaud"));

        gridSetup->addWidget(comboBaud, 1, 1, 1, 1);

        btnConnect = new QPushButton(groupSetup);
        btnConnect->setObjectName(QString::fromUtf8("btnConnect"));
        btnConnect->setMinimumSize(QSize(0, 40));

        gridSetup->addWidget(btnConnect, 1, 2, 1, 1);

        labelStatus = new QLabel(groupSetup);
        labelStatus->setObjectName(QString::fromUtf8("labelStatus"));
        labelStatus->setStyleSheet(QString::fromUtf8("color:red;font-weight:bold;font-size:12pt;"));

        gridSetup->addWidget(labelStatus, 2, 0, 1, 2);

        btnClearRecv = new QPushButton(groupSetup);
        btnClearRecv->setObjectName(QString::fromUtf8("btnClearRecv"));

        gridSetup->addWidget(btnClearRecv, 2, 2, 1, 1);


        crsfMainLayout->addWidget(groupSetup);

        splitterMain = new QWidget(PageSerialCrsfTest);
        splitterMain->setObjectName(QString::fromUtf8("splitterMain"));
        splitterLayout = new QHBoxLayout(splitterMain);
        splitterLayout->setObjectName(QString::fromUtf8("splitterLayout"));
        groupSend = new QGroupBox(splitterMain);
        groupSend->setObjectName(QString::fromUtf8("groupSend"));
        sendLayout = new QVBoxLayout(groupSend);
        sendLayout->setObjectName(QString::fromUtf8("sendLayout"));
        textSend = new QTextEdit(groupSend);
        textSend->setObjectName(QString::fromUtf8("textSend"));
        textSend->setMinimumSize(QSize(0, 120));
        textSend->setMaximumSize(QSize(16777215, 150));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(10);
        textSend->setFont(font);

        sendLayout->addWidget(textSend);

        sendBtnLayout = new QHBoxLayout();
        sendBtnLayout->setObjectName(QString::fromUtf8("sendBtnLayout"));
        comboCrsfType = new QComboBox(groupSend);
        comboCrsfType->setObjectName(QString::fromUtf8("comboCrsfType"));
        comboCrsfType->setMinimumSize(QSize(120, 0));

        sendBtnLayout->addWidget(comboCrsfType);

        spinRepeatCount = new QSpinBox(groupSend);
        spinRepeatCount->setObjectName(QString::fromUtf8("spinRepeatCount"));
        spinRepeatCount->setMinimum(1);
        spinRepeatCount->setMaximum(9999);
        spinRepeatCount->setValue(1);

        sendBtnLayout->addWidget(spinRepeatCount);

        spinRepeatInterval = new QSpinBox(groupSend);
        spinRepeatInterval->setObjectName(QString::fromUtf8("spinRepeatInterval"));
        spinRepeatInterval->setMinimum(1);
        spinRepeatInterval->setMaximum(60000);
        spinRepeatInterval->setValue(100);

        sendBtnLayout->addWidget(spinRepeatInterval);

        btnSend = new QPushButton(groupSend);
        btnSend->setObjectName(QString::fromUtf8("btnSend"));
        btnSend->setMinimumSize(QSize(80, 36));

        sendBtnLayout->addWidget(btnSend);


        sendLayout->addLayout(sendBtnLayout);

        templateLayout = new QHBoxLayout();
        templateLayout->setObjectName(QString::fromUtf8("templateLayout"));
        btnTplRcChannels = new QPushButton(groupSend);
        btnTplRcChannels->setObjectName(QString::fromUtf8("btnTplRcChannels"));

        templateLayout->addWidget(btnTplRcChannels);

        btnTplLinkStats = new QPushButton(groupSend);
        btnTplLinkStats->setObjectName(QString::fromUtf8("btnTplLinkStats"));

        templateLayout->addWidget(btnTplLinkStats);

        btnTplHeartbeat = new QPushButton(groupSend);
        btnTplHeartbeat->setObjectName(QString::fromUtf8("btnTplHeartbeat"));

        templateLayout->addWidget(btnTplHeartbeat);

        btnTplCustom = new QPushButton(groupSend);
        btnTplCustom->setObjectName(QString::fromUtf8("btnTplCustom"));

        templateLayout->addWidget(btnTplCustom);


        sendLayout->addLayout(templateLayout);


        splitterLayout->addWidget(groupSend);

        groupRecv = new QGroupBox(splitterMain);
        groupRecv->setObjectName(QString::fromUtf8("groupRecv"));
        recvLayout = new QVBoxLayout(groupRecv);
        recvLayout->setObjectName(QString::fromUtf8("recvLayout"));
        textRecv = new QTextEdit(groupRecv);
        textRecv->setObjectName(QString::fromUtf8("textRecv"));
        textRecv->setFont(font);
        textRecv->setReadOnly(true);

        recvLayout->addWidget(textRecv);

        recvCtrlLayout = new QHBoxLayout();
        recvCtrlLayout->setObjectName(QString::fromUtf8("recvCtrlLayout"));
        checkAutoScroll = new QCheckBox(groupRecv);
        checkAutoScroll->setObjectName(QString::fromUtf8("checkAutoScroll"));
        checkAutoScroll->setChecked(true);

        recvCtrlLayout->addWidget(checkAutoScroll);

        checkShowTimestamp = new QCheckBox(groupRecv);
        checkShowTimestamp->setObjectName(QString::fromUtf8("checkShowTimestamp"));
        checkShowTimestamp->setChecked(true);

        recvCtrlLayout->addWidget(checkShowTimestamp);

        checkHexOnly = new QCheckBox(groupRecv);
        checkHexOnly->setObjectName(QString::fromUtf8("checkHexOnly"));

        recvCtrlLayout->addWidget(checkHexOnly);

        spRecv = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        recvCtrlLayout->addItem(spRecv);

        btnClearRecvData = new QPushButton(groupRecv);
        btnClearRecvData->setObjectName(QString::fromUtf8("btnClearRecvData"));

        recvCtrlLayout->addWidget(btnClearRecvData);

        btnSaveRecv = new QPushButton(groupRecv);
        btnSaveRecv->setObjectName(QString::fromUtf8("btnSaveRecv"));

        recvCtrlLayout->addWidget(btnSaveRecv);

        btnDiagWindow = new QPushButton(groupRecv);
        btnDiagWindow->setObjectName(QString::fromUtf8("btnDiagWindow"));

        recvCtrlLayout->addWidget(btnDiagWindow);


        recvLayout->addLayout(recvCtrlLayout);


        splitterLayout->addWidget(groupRecv);


        crsfMainLayout->addWidget(splitterMain);

        groupLatency = new QGroupBox(PageSerialCrsfTest);
        groupLatency->setObjectName(QString::fromUtf8("groupLatency"));
        latencyLayout = new QHBoxLayout(groupLatency);
        latencyLayout->setObjectName(QString::fromUtf8("latencyLayout"));
        lbLastLatency = new QLabel(groupLatency);
        lbLastLatency->setObjectName(QString::fromUtf8("lbLastLatency"));
        lbLastLatency->setStyleSheet(QString::fromUtf8("font-weight:bold;font-size:11pt;"));

        latencyLayout->addWidget(lbLastLatency);

        lbAvgLatency = new QLabel(groupLatency);
        lbAvgLatency->setObjectName(QString::fromUtf8("lbAvgLatency"));
        lbAvgLatency->setStyleSheet(QString::fromUtf8("font-size:10pt;color:#555;"));

        latencyLayout->addWidget(lbAvgLatency);

        lbMinLatency = new QLabel(groupLatency);
        lbMinLatency->setObjectName(QString::fromUtf8("lbMinLatency"));

        latencyLayout->addWidget(lbMinLatency);

        lbMaxLatency = new QLabel(groupLatency);
        lbMaxLatency->setObjectName(QString::fromUtf8("lbMaxLatency"));

        latencyLayout->addWidget(lbMaxLatency);

        lbPktCount = new QLabel(groupLatency);
        lbPktCount->setObjectName(QString::fromUtf8("lbPktCount"));

        latencyLayout->addWidget(lbPktCount);

        btnResetStats = new QPushButton(groupLatency);
        btnResetStats->setObjectName(QString::fromUtf8("btnResetStats"));

        latencyLayout->addWidget(btnResetStats);

        spinStopAt = new QSpinBox(groupLatency);
        spinStopAt->setObjectName(QString::fromUtf8("spinStopAt"));
        spinStopAt->setMinimumSize(QSize(100, 0));
        spinStopAt->setMinimum(1);
        spinStopAt->setMaximum(999999);
        spinStopAt->setValue(100);

        latencyLayout->addWidget(spinStopAt);

        btnStopAtEnable = new QPushButton(groupLatency);
        btnStopAtEnable->setObjectName(QString::fromUtf8("btnStopAtEnable"));
        btnStopAtEnable->setCheckable(true);
        btnStopAtEnable->setMinimumSize(QSize(90, 0));

        latencyLayout->addWidget(btnStopAtEnable);


        crsfMainLayout->addWidget(groupLatency);

        groupDist = new QGroupBox(PageSerialCrsfTest);
        groupDist->setObjectName(QString::fromUtf8("groupDist"));
        distLayout = new QVBoxLayout(groupDist);
        distLayout->setObjectName(QString::fromUtf8("distLayout"));
        textDist = new QTextEdit(groupDist);
        textDist->setObjectName(QString::fromUtf8("textDist"));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Courier New"));
        font1.setPointSize(9);
        textDist->setFont(font1);
        textDist->setReadOnly(true);

        distLayout->addWidget(textDist);


        crsfMainLayout->addWidget(groupDist);


        retranslateUi(PageSerialCrsfTest);

        QMetaObject::connectSlotsByName(PageSerialCrsfTest);
    } // setupUi

    void retranslateUi(QWidget *PageSerialCrsfTest)
    {
        groupSetup->setTitle(QCoreApplication::translate("PageSerialCrsfTest", "CRSF \344\270\262\345\217\243\350\277\236\346\216\245\350\256\276\347\275\256", nullptr));
        lbPort->setText(QCoreApplication::translate("PageSerialCrsfTest", "\347\253\257\345\217\243:", nullptr));
        btnRefresh->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\210\267\346\226\260", nullptr));
        lbBaud->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\263\242\347\211\271\347\216\207:", nullptr));
        btnConnect->setText(QCoreApplication::translate("PageSerialCrsfTest", "\350\277\236\346\216\245", nullptr));
        labelStatus->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\234\252\350\277\236\346\216\245", nullptr));
        btnClearRecv->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\270\205\347\251\272", nullptr));
        groupSend->setTitle(QCoreApplication::translate("PageSerialCrsfTest", "\345\217\221\351\200\201\346\225\260\346\215\256", nullptr));
        textSend->setPlaceholderText(QCoreApplication::translate("PageSerialCrsfTest", "\350\276\223\345\205\245\350\246\201\345\217\221\351\200\201\347\232\204\345\215\201\345\205\255\350\277\233\345\210\266\346\225\260\346\215\256 (\347\251\272\346\240\274\345\210\206\351\232\224)\357\274\214\345\246\202: C8 14 00 00 00 00", nullptr));
        spinRepeatCount->setPrefix(QCoreApplication::translate("PageSerialCrsfTest", "\351\207\215\345\244\215: ", nullptr));
        spinRepeatInterval->setPrefix(QCoreApplication::translate("PageSerialCrsfTest", "\351\227\264\351\232\224: ", nullptr));
        spinRepeatInterval->setSuffix(QCoreApplication::translate("PageSerialCrsfTest", " ms", nullptr));
        btnSend->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\217\221\351\200\201", nullptr));
        btnTplRcChannels->setText(QCoreApplication::translate("PageSerialCrsfTest", "RC\351\200\232\351\201\223", nullptr));
        btnTplLinkStats->setText(QCoreApplication::translate("PageSerialCrsfTest", "\351\223\276\350\267\257\347\273\237\350\256\241", nullptr));
        btnTplHeartbeat->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\277\203\350\267\263\345\214\205", nullptr));
        btnTplCustom->setText(QCoreApplication::translate("PageSerialCrsfTest", "\350\207\252\345\256\232\344\271\211Hex", nullptr));
        groupRecv->setTitle(QCoreApplication::translate("PageSerialCrsfTest", "\346\216\245\346\224\266\346\225\260\346\215\256 (\345\270\246\346\227\266\351\227\264\346\210\263)", nullptr));
        checkAutoScroll->setText(QCoreApplication::translate("PageSerialCrsfTest", "\350\207\252\345\212\250\346\273\232\345\212\250", nullptr));
        checkShowTimestamp->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\230\276\347\244\272\346\227\266\351\227\264\346\210\263", nullptr));
        checkHexOnly->setText(QCoreApplication::translate("PageSerialCrsfTest", "\347\272\257Hex", nullptr));
        btnClearRecvData->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        btnSaveRecv->setText(QCoreApplication::translate("PageSerialCrsfTest", "\344\277\235\345\255\230\346\225\260\346\215\256", nullptr));
        btnDiagWindow->setText(QCoreApplication::translate("PageSerialCrsfTest", "\350\257\212\346\226\255\347\252\227\345\217\243", nullptr));
        groupLatency->setTitle(QCoreApplication::translate("PageSerialCrsfTest", "\345\276\200\350\277\224\345\273\266\350\277\237\347\273\237\350\256\241", nullptr));
        lbLastLatency->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\234\200\345\220\216\345\273\266\350\277\237: --", nullptr));
        lbAvgLatency->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\271\263\345\235\207\345\273\266\350\277\237: --", nullptr));
        lbMinLatency->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\234\200\345\260\217: --", nullptr));
        lbMaxLatency->setText(QCoreApplication::translate("PageSerialCrsfTest", "\346\234\200\345\244\247: --", nullptr));
        lbPktCount->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\267\262\346\224\266\345\214\205: 0", nullptr));
        btnResetStats->setText(QCoreApplication::translate("PageSerialCrsfTest", "\351\207\215\347\275\256\347\273\237\350\256\241", nullptr));
        spinStopAt->setPrefix(QCoreApplication::translate("PageSerialCrsfTest", "\345\270\247\346\225\260 ", nullptr));
        spinStopAt->setSuffix(QString());
        btnStopAtEnable->setText(QCoreApplication::translate("PageSerialCrsfTest", "\345\274\200\345\220\257", nullptr));
        groupDist->setTitle(QCoreApplication::translate("PageSerialCrsfTest", "\345\273\266\350\277\237\345\210\206\345\270\203\347\273\237\350\256\241", nullptr));
        (void)PageSerialCrsfTest;
    } // retranslateUi

};

namespace Ui {
    class PageSerialCrsfTest: public Ui_PageSerialCrsfTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_CRSFTEST_H
