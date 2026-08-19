/********************************************************************************
** Form generated from reading UI file 'page_serial_sbustest.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_SBUSTEST_H
#define UI_PAGE_SERIAL_SBUSTEST_H

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

class Ui_PageSerialSbusTest
{
public:
    QVBoxLayout *sbusMainLayout;
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
    QPushButton *btnTplChannels;
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

    void setupUi(QWidget *PageSerialSbusTest)
    {
        if (PageSerialSbusTest->objectName().isEmpty())
            PageSerialSbusTest->setObjectName(QString::fromUtf8("PageSerialSbusTest"));
        sbusMainLayout = new QVBoxLayout(PageSerialSbusTest);
        sbusMainLayout->setObjectName(QString::fromUtf8("sbusMainLayout"));
        groupSetup = new QGroupBox(PageSerialSbusTest);
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


        sbusMainLayout->addWidget(groupSetup);

        splitterMain = new QWidget(PageSerialSbusTest);
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
        spinRepeatInterval->setValue(14);

        sendBtnLayout->addWidget(spinRepeatInterval);

        btnSend = new QPushButton(groupSend);
        btnSend->setObjectName(QString::fromUtf8("btnSend"));
        btnSend->setMinimumSize(QSize(80, 36));

        sendBtnLayout->addWidget(btnSend);


        sendLayout->addLayout(sendBtnLayout);

        templateLayout = new QHBoxLayout();
        templateLayout->setObjectName(QString::fromUtf8("templateLayout"));
        btnTplChannels = new QPushButton(groupSend);
        btnTplChannels->setObjectName(QString::fromUtf8("btnTplChannels"));

        templateLayout->addWidget(btnTplChannels);

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


        sbusMainLayout->addWidget(splitterMain);

        groupStats = new QGroupBox(PageSerialSbusTest);
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


        sbusMainLayout->addWidget(groupStats);


        retranslateUi(PageSerialSbusTest);

        QMetaObject::connectSlotsByName(PageSerialSbusTest);
    } // setupUi

    void retranslateUi(QWidget *PageSerialSbusTest)
    {
        groupSetup->setTitle(QCoreApplication::translate("PageSerialSbusTest", "SBUS \344\270\262\345\217\243\350\277\236\346\216\245\350\256\276\347\275\256", nullptr));
        lbPort->setText(QCoreApplication::translate("PageSerialSbusTest", "\347\253\257\345\217\243:", nullptr));
        btnRefresh->setText(QCoreApplication::translate("PageSerialSbusTest", "\345\210\267\346\226\260", nullptr));
        lbBaud->setText(QCoreApplication::translate("PageSerialSbusTest", "\346\263\242\347\211\271\347\216\207:", nullptr));
        btnConnect->setText(QCoreApplication::translate("PageSerialSbusTest", "\350\277\236\346\216\245", nullptr));
        labelStatus->setText(QCoreApplication::translate("PageSerialSbusTest", "\346\234\252\350\277\236\346\216\245", nullptr));
        btnClearRecv->setText(QCoreApplication::translate("PageSerialSbusTest", "\346\270\205\347\251\272", nullptr));
        groupSend->setTitle(QCoreApplication::translate("PageSerialSbusTest", "\345\217\221\351\200\201\346\225\260\346\215\256 (25\345\255\227\350\212\202SBUS\345\270\247)", nullptr));
        textSend->setPlaceholderText(QCoreApplication::translate("PageSerialSbusTest", "\350\276\223\345\205\245\350\246\201\345\217\221\351\200\201\347\232\204\345\215\201\345\205\255\350\277\233\345\210\266\346\225\260\346\215\256 (\347\251\272\346\240\274\345\210\206\351\232\224)\357\274\214\345\246\202: 0F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00", nullptr));
        spinRepeatCount->setPrefix(QCoreApplication::translate("PageSerialSbusTest", "\351\207\215\345\244\215: ", nullptr));
        spinRepeatInterval->setPrefix(QCoreApplication::translate("PageSerialSbusTest", "\351\227\264\351\232\224: ", nullptr));
        spinRepeatInterval->setSuffix(QCoreApplication::translate("PageSerialSbusTest", " ms", nullptr));
        btnSend->setText(QCoreApplication::translate("PageSerialSbusTest", "\345\217\221\351\200\201", nullptr));
        btnTplChannels->setText(QCoreApplication::translate("PageSerialSbusTest", "\344\270\255\347\253\213\351\200\232\351\201\223\345\270\247", nullptr));
        btnTplCustom->setText(QCoreApplication::translate("PageSerialSbusTest", "\347\244\272\344\276\213\351\200\232\351\201\223\345\270\247", nullptr));
        groupRecv->setTitle(QCoreApplication::translate("PageSerialSbusTest", "\346\216\245\346\224\266\346\225\260\346\215\256 (\345\270\246\346\227\266\351\227\264\346\210\263)", nullptr));
        checkAutoScroll->setText(QCoreApplication::translate("PageSerialSbusTest", "\350\207\252\345\212\250\346\273\232\345\212\250", nullptr));
        btnClearRecvData->setText(QCoreApplication::translate("PageSerialSbusTest", "\346\270\205\347\251\272\346\216\245\346\224\266", nullptr));
        btnSaveRecv->setText(QCoreApplication::translate("PageSerialSbusTest", "\344\277\235\345\255\230\346\225\260\346\215\256", nullptr));
        groupStats->setTitle(QCoreApplication::translate("PageSerialSbusTest", "SBUS \347\273\237\350\256\241", nullptr));
        lbPktCount->setText(QCoreApplication::translate("PageSerialSbusTest", "\346\234\211\346\225\210\345\270\247: 0", nullptr));
        btnResetStats->setText(QCoreApplication::translate("PageSerialSbusTest", "\351\207\215\347\275\256\347\273\237\350\256\241", nullptr));
        spinStopAt->setPrefix(QCoreApplication::translate("PageSerialSbusTest", "\345\270\247\346\225\260 ", nullptr));
        btnStopAtEnable->setText(QCoreApplication::translate("PageSerialSbusTest", "\345\274\200\345\220\257", nullptr));
        (void)PageSerialSbusTest;
    } // retranslateUi

};

namespace Ui {
    class PageSerialSbusTest: public Ui_PageSerialSbusTest {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_SBUSTEST_H
