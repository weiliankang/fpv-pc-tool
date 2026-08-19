/********************************************************************************
** Form generated from reading UI file 'page_serial_mavlinktest.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_MAVLINKTEST_H
#define UI_PAGE_SERIAL_MAVLINKTEST_H

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

class Ui_PageSerialMavlinkTest
{
public:
    QVBoxLayout *mavMainLayout;
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
    QSpinBox *spinRepeatCount;
    QSpinBox *spinRepeatInterval;
    QPushButton *btnSend;
    QHBoxLayout *templateLayout;
    QPushButton *btnTplHeartbeat;
    QPushButton *btnTplCustom;
    QSpacerItem *spTpl;
    QGroupBox *groupRecv;
    QVBoxLayout *recvLayout;
    QTextEdit *textRecv;
    QHBoxLayout *recvCtrlLayout;
    QCheckBox *checkAutoScroll;
    QSpacerItem *spRecv;
    QPushButton *btnClearRecvData;
    QPushButton *btnSaveRecv;
    QGroupBox *groupStats;
    QHBoxLayout *statsLayout;
    QLabel *lbPktCount;
    QPushButton *btnResetStats;
    QSpinBox *spinStopAt;
    QPushButton *btnStopAtEnable;
    QSpacerItem *spStats;

    void setupUi(QWidget *PageSerialMavlinkTest)
    {
        if (PageSerialMavlinkTest->objectName().isEmpty())
            PageSerialMavlinkTest->setObjectName(QString::fromUtf8("PageSerialMavlinkTest"));
        mavMainLayout = new QVBoxLayout(PageSerialMavlinkTest);
        mavMainLayout->setObjectName(QString::fromUtf8("mavMainLayout"));
        groupSetup = new QGroupBox(PageSerialMavlinkTest);
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


        mavMainLayout->addWidget(groupSetup);

        splitterMain = new QWidget(PageSerialMavlinkTest);
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
        btnTplHeartbeat = new QPushButton(groupSend);
        btnTplHeartbeat->setObjectName(QString::fromUtf8("btnTplHeartbeat"));

        templateLayout->addWidget(btnTplHeartbeat);

        btnTplCustom = new QPushButton(groupSend);
        btnTplCustom->setObjectName(QString::fromUtf8("btnTplCustom"));

        templateLayout->addWidget(btnTplCustom);

        spTpl = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        templateLayout->addItem(spTpl);


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

        spRecv = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        recvCtrlLayout->addItem(spRecv);

        btnClearRecvData = new QPushButton(groupRecv);
        btnClearRecvData->setObjectName(QString::fromUtf8("btnClearRecvData"));

        recvCtrlLayout->addWidget(btnClearRecvData);

        btnSaveRecv = new QPushButton(groupRecv);
        btnSaveRecv->setObjectName(QString::fromUtf8("btnSaveRecv"));

        recvCtrlLayout->addWidget(btnSaveRecv);


        recvLayout->addLayout(recvCtrlLayout);


        splitterLayout->addWidget(groupRecv);


        mavMainLayout->addWidget(splitterMain);

        groupStats = new QGroupBox(PageSerialMavlinkTest);
        groupStats->setObjectName(QString::fromUtf8("groupStats"));
        statsLayout = new QHBoxLayout(groupStats);
        statsLayout->setObjectName(QString::fromUtf8("statsLayout"));
        lbPktCount = new QLabel(groupStats);
        lbPktCount->setObjectName(QString::fromUtf8("lbPktCount"));
        lbPktCount->setStyleSheet(QString::fromUtf8("font-weight:bold;font-size:11pt;"));

        statsLayout->addWidget(lbPktCount);

        btnResetStats = new QPushButton(groupStats);
        btnResetStats->setObjectName(QString::fromUtf8("btnResetStats"));

        statsLayout->addWidget(btnResetStats);

        spinStopAt = new QSpinBox(groupStats);
        spinStopAt->setObjectName(QString::fromUtf8("spinStopAt"));
        spinStopAt->setMinimumSize(QSize(100, 0));
        spinStopAt->setMinimum(1);
        spinStopAt->setMaximum(999999);
        spinStopAt->setValue(100);

        statsLayout->addWidget(spinStopAt);

        btnStopAtEnable = new QPushButton(groupStats);
        btnStopAtEnable->setObjectName(QString::fromUtf8("btnStopAtEnable"));
        btnStopAtEnable->setCheckable(true);

        statsLayout->addWidget(btnStopAtEnable);

        spStats = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        statsLayout->addItem(spStats);


        mavMainLayout->addWidget(groupStats);


        retranslateUi(PageSerialMavlinkTest);

        QMetaObject::connectSlotsByName(PageSerialMavlinkTest);
    } // setupUi

    void retranslateUi(QWidget *PageSerialMavlinkTest)
    {
        groupSetup->setTitle(QCoreApplication::translate("PageSerialMavlinkTest", "MAVLink \344\270\262\345\217\243\350\277\236\346\216\245\350\256\276\347\275\256", nullptr));
        lbPort->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\347\253\257\345\217\243:", nullptr));
        btnRefresh->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\345\210\267\346\226\260", nullptr));
        lbBaud->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\346\263\242\347\211\271\347\216\207:", nullptr));
        btnConnect->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\350\277\236\346\216\245", nullptr));
        labelStatus->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\346\234\252\350\277\236\346\216\245", nullptr));
        btnClearRecv->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\346\270\205\347\251\272", nullptr));
        groupSend->setTitle(QCoreApplication::translate("PageSerialMavlinkTest", "\345\217\221\351\200\201\346\225\260\346\215\256 (MAVLink v1/v2 \345\270\247)", nullptr));
        textSend->setPlaceholderText(QCoreApplication::translate("PageSerialMavlinkTest", "\350\276\223\345\205\245\350\246\201\345\217\221\351\200\201\347\232\204\345\215\201\345\205\255\350\277\233\345\210\266\346\225\260\346\215\256 (\347\251\272\346\240\274\345\210\206\351\232\224)\357\274\214\345\246\202: FE 09 00 01 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00", nullptr));
        spinRepeatCount->setPrefix(QCoreApplication::translate("PageSerialMavlinkTest", "\351\207\215\345\244\215: ", nullptr));
        spinRepeatInterval->setPrefix(QCoreApplication::translate("PageSerialMavlinkTest", "\351\227\264\351\232\224: ", nullptr));
        spinRepeatInterval->setSuffix(QCoreApplication::translate("PageSerialMavlinkTest", " ms", nullptr));
        btnSend->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\345\217\221\351\200\201", nullptr));
        btnTplHeartbeat->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\345\277\203\350\267\263\345\214\205(v1)", nullptr));
        btnTplCustom->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\347\251\272\345\270\247(v2)", nullptr));
        groupRecv->setTitle(QCoreApplication::translate("PageSerialMavlinkTest", "\346\216\245\346\224\266\346\225\260\346\215\256 (\345\270\246\346\227\266\351\227\264\346\210\263)", nullptr));
        checkAutoScroll->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\350\207\252\345\212\250\346\273\232\345\212\250", nullptr));
        btnClearRecvData->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        btnSaveRecv->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\344\277\235\345\255\230\346\225\260\346\215\256", nullptr));
        groupStats->setTitle(QCoreApplication::translate("PageSerialMavlinkTest", "MAVLink \347\273\237\350\256\241", nullptr));
        lbPktCount->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\346\234\211\346\225\210\345\270\247: 0", nullptr));
        btnResetStats->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\351\207\215\347\275\256\347\273\237\350\256\241", nullptr));
        spinStopAt->setPrefix(QCoreApplication::translate("PageSerialMavlinkTest", "\345\270\247\346\225\260 ", nullptr));
        btnStopAtEnable->setText(QCoreApplication::translate("PageSerialMavlinkTest", "\345\274\200\345\220\257", nullptr));
        (void)PageSerialMavlinkTest;
    } // retranslateUi

};

namespace Ui {
    class PageSerialMavlinkTest: public Ui_PageSerialMavlinkTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_MAVLINKTEST_H
