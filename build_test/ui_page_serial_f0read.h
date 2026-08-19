/********************************************************************************
** Form generated from reading UI file 'page_serial_f0read.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_F0READ_H
#define UI_PAGE_SERIAL_F0READ_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <fillplaintextedit.h>

QT_BEGIN_NAMESPACE

class Ui_PageSerialF0Read
{
public:
    QVBoxLayout *f0MainLayout;
    QGroupBox *groupControl;
    QHBoxLayout *ctlLayout;
    QPushButton *btnSendF0;
    QPushButton *btnAutoPoll;
    QLabel *lbInterval;
    QSpinBox *spinInterval;
    QPushButton *btnClear;
    QSpacerItem *hSpacerCtl;
    QPushButton *btnSaveRaw;
    QLabel *lbSavePath;
    QGroupBox *groupFile;
    QHBoxLayout *fileLayout;
    QLineEdit *editFilePath;
    QPushButton *btnBrowseFile;
    QPushButton *btnLoadFile;
    QGroupBox *groupPlayer;
    QVBoxLayout *playerLayout;
    QHBoxLayout *playerCtlLayout;
    QPushButton *btnPlay;
    QPushButton *btnPrevFrame;
    QPushButton *btnNextFrame;
    QLabel *lbFrameInterval;
    QSpinBox *spinFrameInterval;
    QLabel *lbFrameInfo;
    QSpacerItem *hSpacerPlayer;
    QLabel *lbFont;
    QComboBox *cboFont;
    QCheckBox *chkAttitudeBar;
    QWidget *widgetOsdPlayer;
    QGroupBox *groupStats;
    QHBoxLayout *statsLayout;
    QLabel *lbRxBytes;
    QLabel *lbFrames;
    QLabel *lbLastCmd;
    QLabel *lbLastInterval;
    QGroupBox *groupLog;
    QVBoxLayout *logLayout;
    FillPlainTextEdit *textLog;

    void setupUi(QWidget *PageSerialF0Read)
    {
        if (PageSerialF0Read->objectName().isEmpty())
            PageSerialF0Read->setObjectName(QString::fromUtf8("PageSerialF0Read"));
        f0MainLayout = new QVBoxLayout(PageSerialF0Read);
        f0MainLayout->setObjectName(QString::fromUtf8("f0MainLayout"));
        groupControl = new QGroupBox(PageSerialF0Read);
        groupControl->setObjectName(QString::fromUtf8("groupControl"));
        ctlLayout = new QHBoxLayout(groupControl);
        ctlLayout->setObjectName(QString::fromUtf8("ctlLayout"));
        btnSendF0 = new QPushButton(groupControl);
        btnSendF0->setObjectName(QString::fromUtf8("btnSendF0"));
        btnSendF0->setMinimumSize(QSize(0, 40));

        ctlLayout->addWidget(btnSendF0);

        btnAutoPoll = new QPushButton(groupControl);
        btnAutoPoll->setObjectName(QString::fromUtf8("btnAutoPoll"));
        btnAutoPoll->setCheckable(true);
        btnAutoPoll->setMinimumSize(QSize(0, 40));

        ctlLayout->addWidget(btnAutoPoll);

        lbInterval = new QLabel(groupControl);
        lbInterval->setObjectName(QString::fromUtf8("lbInterval"));

        ctlLayout->addWidget(lbInterval);

        spinInterval = new QSpinBox(groupControl);
        spinInterval->setObjectName(QString::fromUtf8("spinInterval"));
        spinInterval->setMinimum(50);
        spinInterval->setMaximum(5000);
        spinInterval->setValue(100);

        ctlLayout->addWidget(spinInterval);

        btnClear = new QPushButton(groupControl);
        btnClear->setObjectName(QString::fromUtf8("btnClear"));
        btnClear->setMinimumSize(QSize(0, 40));

        ctlLayout->addWidget(btnClear);

        hSpacerCtl = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        ctlLayout->addItem(hSpacerCtl);

        btnSaveRaw = new QPushButton(groupControl);
        btnSaveRaw->setObjectName(QString::fromUtf8("btnSaveRaw"));
        btnSaveRaw->setCheckable(true);
        btnSaveRaw->setMinimumSize(QSize(0, 40));

        ctlLayout->addWidget(btnSaveRaw);

        lbSavePath = new QLabel(groupControl);
        lbSavePath->setObjectName(QString::fromUtf8("lbSavePath"));

        ctlLayout->addWidget(lbSavePath);


        f0MainLayout->addWidget(groupControl);

        groupFile = new QGroupBox(PageSerialF0Read);
        groupFile->setObjectName(QString::fromUtf8("groupFile"));
        fileLayout = new QHBoxLayout(groupFile);
        fileLayout->setObjectName(QString::fromUtf8("fileLayout"));
        editFilePath = new QLineEdit(groupFile);
        editFilePath->setObjectName(QString::fromUtf8("editFilePath"));

        fileLayout->addWidget(editFilePath);

        btnBrowseFile = new QPushButton(groupFile);
        btnBrowseFile->setObjectName(QString::fromUtf8("btnBrowseFile"));
        btnBrowseFile->setMinimumSize(QSize(0, 36));

        fileLayout->addWidget(btnBrowseFile);

        btnLoadFile = new QPushButton(groupFile);
        btnLoadFile->setObjectName(QString::fromUtf8("btnLoadFile"));
        btnLoadFile->setMinimumSize(QSize(0, 36));

        fileLayout->addWidget(btnLoadFile);


        f0MainLayout->addWidget(groupFile);

        groupPlayer = new QGroupBox(PageSerialF0Read);
        groupPlayer->setObjectName(QString::fromUtf8("groupPlayer"));
        playerLayout = new QVBoxLayout(groupPlayer);
        playerLayout->setObjectName(QString::fromUtf8("playerLayout"));
        playerCtlLayout = new QHBoxLayout();
        playerCtlLayout->setObjectName(QString::fromUtf8("playerCtlLayout"));
        btnPlay = new QPushButton(groupPlayer);
        btnPlay->setObjectName(QString::fromUtf8("btnPlay"));
        btnPlay->setMinimumSize(QSize(0, 36));

        playerCtlLayout->addWidget(btnPlay);

        btnPrevFrame = new QPushButton(groupPlayer);
        btnPrevFrame->setObjectName(QString::fromUtf8("btnPrevFrame"));
        btnPrevFrame->setMinimumSize(QSize(0, 36));

        playerCtlLayout->addWidget(btnPrevFrame);

        btnNextFrame = new QPushButton(groupPlayer);
        btnNextFrame->setObjectName(QString::fromUtf8("btnNextFrame"));
        btnNextFrame->setMinimumSize(QSize(0, 36));

        playerCtlLayout->addWidget(btnNextFrame);

        lbFrameInterval = new QLabel(groupPlayer);
        lbFrameInterval->setObjectName(QString::fromUtf8("lbFrameInterval"));

        playerCtlLayout->addWidget(lbFrameInterval);

        spinFrameInterval = new QSpinBox(groupPlayer);
        spinFrameInterval->setObjectName(QString::fromUtf8("spinFrameInterval"));
        spinFrameInterval->setMinimum(20);
        spinFrameInterval->setMaximum(5000);
        spinFrameInterval->setValue(200);

        playerCtlLayout->addWidget(spinFrameInterval);

        lbFrameInfo = new QLabel(groupPlayer);
        lbFrameInfo->setObjectName(QString::fromUtf8("lbFrameInfo"));

        playerCtlLayout->addWidget(lbFrameInfo);

        hSpacerPlayer = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        playerCtlLayout->addItem(hSpacerPlayer);

        lbFont = new QLabel(groupPlayer);
        lbFont->setObjectName(QString::fromUtf8("lbFont"));

        playerCtlLayout->addWidget(lbFont);

        cboFont = new QComboBox(groupPlayer);
        cboFont->addItem(QString());
        cboFont->addItem(QString());
        cboFont->addItem(QString());
        cboFont->addItem(QString());
        cboFont->addItem(QString());
        cboFont->addItem(QString());
        cboFont->setObjectName(QString::fromUtf8("cboFont"));

        playerCtlLayout->addWidget(cboFont);

        chkAttitudeBar = new QCheckBox(groupPlayer);
        chkAttitudeBar->setObjectName(QString::fromUtf8("chkAttitudeBar"));
        chkAttitudeBar->setChecked(false);

        playerCtlLayout->addWidget(chkAttitudeBar);


        playerLayout->addLayout(playerCtlLayout);

        widgetOsdPlayer = new QWidget(groupPlayer);
        widgetOsdPlayer->setObjectName(QString::fromUtf8("widgetOsdPlayer"));
        widgetOsdPlayer->setMinimumSize(QSize(0, 0));

        playerLayout->addWidget(widgetOsdPlayer);


        f0MainLayout->addWidget(groupPlayer);

        groupStats = new QGroupBox(PageSerialF0Read);
        groupStats->setObjectName(QString::fromUtf8("groupStats"));
        statsLayout = new QHBoxLayout(groupStats);
        statsLayout->setObjectName(QString::fromUtf8("statsLayout"));
        lbRxBytes = new QLabel(groupStats);
        lbRxBytes->setObjectName(QString::fromUtf8("lbRxBytes"));

        statsLayout->addWidget(lbRxBytes);

        lbFrames = new QLabel(groupStats);
        lbFrames->setObjectName(QString::fromUtf8("lbFrames"));

        statsLayout->addWidget(lbFrames);

        lbLastCmd = new QLabel(groupStats);
        lbLastCmd->setObjectName(QString::fromUtf8("lbLastCmd"));

        statsLayout->addWidget(lbLastCmd);

        lbLastInterval = new QLabel(groupStats);
        lbLastInterval->setObjectName(QString::fromUtf8("lbLastInterval"));

        statsLayout->addWidget(lbLastInterval);


        f0MainLayout->addWidget(groupStats);

        groupLog = new QGroupBox(PageSerialF0Read);
        groupLog->setObjectName(QString::fromUtf8("groupLog"));
        logLayout = new QVBoxLayout(groupLog);
        logLayout->setObjectName(QString::fromUtf8("logLayout"));
        textLog = new FillPlainTextEdit(groupLog);
        textLog->setObjectName(QString::fromUtf8("textLog"));
        textLog->setReadOnly(true);
        textLog->setMaximumBlockCount(2000);

        logLayout->addWidget(textLog);


        f0MainLayout->addWidget(groupLog);


        retranslateUi(PageSerialF0Read);

        QMetaObject::connectSlotsByName(PageSerialF0Read);
    } // setupUi

    void retranslateUi(QWidget *PageSerialF0Read)
    {
        groupControl->setTitle(QCoreApplication::translate("PageSerialF0Read", "F0 \350\257\273\345\217\226\346\216\247\345\210\266", nullptr));
        btnSendF0->setText(QCoreApplication::translate("PageSerialF0Read", "\345\217\221\351\200\201 F0 \346\214\207\344\273\244", nullptr));
        btnAutoPoll->setText(QCoreApplication::translate("PageSerialF0Read", "\350\207\252\345\212\250\350\275\256\350\257\242", nullptr));
        lbInterval->setText(QCoreApplication::translate("PageSerialF0Read", "\351\227\264\351\232\224(ms):", nullptr));
        spinInterval->setSuffix(QCoreApplication::translate("PageSerialF0Read", " ms", nullptr));
        btnClear->setText(QCoreApplication::translate("PageSerialF0Read", "\346\270\205\347\251\272", nullptr));
        btnSaveRaw->setText(QCoreApplication::translate("PageSerialF0Read", "\360\237\223\201 \350\256\260\345\275\225\345\216\237\345\247\213\346\225\260\346\215\256", nullptr));
        lbSavePath->setText(QCoreApplication::translate("PageSerialF0Read", "\346\234\252\350\256\260\345\275\225", nullptr));
        groupFile->setTitle(QCoreApplication::translate("PageSerialF0Read", "\350\257\273\345\217\226 test.txt \347\246\273\347\272\277\350\247\243\346\236\220", nullptr));
        editFilePath->setPlaceholderText(QCoreApplication::translate("PageSerialF0Read", "\351\200\211\346\213\251\346\210\226\350\276\223\345\205\245 0x57 \346\227\245\345\277\227\346\226\207\344\273\266\350\267\257\345\276\204\357\274\210\346\257\217\350\241\214: [\346\227\266\351\227\264\346\210\263] 57 24 58 ... hex\357\274\211", nullptr));
        btnBrowseFile->setText(QCoreApplication::translate("PageSerialF0Read", "\346\265\217\350\247\210\342\200\246", nullptr));
        btnLoadFile->setText(QCoreApplication::translate("PageSerialF0Read", "\350\257\273\345\217\226\345\271\266\350\247\243\346\236\220", nullptr));
        groupPlayer->setTitle(QCoreApplication::translate("PageSerialF0Read", "OSD \351\200\220\345\270\247\346\222\255\346\224\276\357\274\210\345\203\217 OSD \345\217\257\350\247\206\345\214\226\351\202\243\346\240\267\344\270\200\345\270\247\345\270\247\346\230\276\347\244\272\357\274\211", nullptr));
        btnPlay->setText(QCoreApplication::translate("PageSerialF0Read", "\342\226\266 \346\222\255\346\224\276", nullptr));
        btnPrevFrame->setText(QCoreApplication::translate("PageSerialF0Read", "\342\227\200 \344\270\212\344\270\200\345\270\247", nullptr));
        btnNextFrame->setText(QCoreApplication::translate("PageSerialF0Read", "\344\270\213\344\270\200\345\270\247 \342\226\266", nullptr));
        lbFrameInterval->setText(QCoreApplication::translate("PageSerialF0Read", "\345\270\247\351\227\264\351\232\224(ms):", nullptr));
        spinFrameInterval->setSuffix(QCoreApplication::translate("PageSerialF0Read", " ms", nullptr));
        lbFrameInfo->setText(QCoreApplication::translate("PageSerialF0Read", "\345\270\247: -/-", nullptr));
        lbFont->setText(QCoreApplication::translate("PageSerialF0Read", "\345\255\227\344\275\223\346\240\267\345\274\217:", nullptr));
        cboFont->setItemText(0, QCoreApplication::translate("PageSerialF0Read", "Betaflight", nullptr));
        cboFont->setItemText(1, QCoreApplication::translate("PageSerialF0Read", "INAV", nullptr));
        cboFont->setItemText(2, QCoreApplication::translate("PageSerialF0Read", "ArduPilot", nullptr));
        cboFont->setItemText(3, QCoreApplication::translate("PageSerialF0Read", "Fettec", nullptr));
        cboFont->setItemText(4, QCoreApplication::translate("PageSerialF0Read", "KISS", nullptr));
        cboFont->setItemText(5, QCoreApplication::translate("PageSerialF0Read", "\345\206\205\347\275\256\345\255\227\347\254\246\351\233\206(osdChars)", nullptr));

        chkAttitudeBar->setText(QCoreApplication::translate("PageSerialF0Read", "\345\247\277\346\200\201\346\235\241\345\233\276\345\275\242\345\214\226", nullptr));
        groupStats->setTitle(QCoreApplication::translate("PageSerialF0Read", "\347\212\266\346\200\201\347\273\237\350\256\241", nullptr));
        lbRxBytes->setText(QCoreApplication::translate("PageSerialF0Read", "\346\216\245\346\224\266\345\255\227\350\212\202: 0", nullptr));
        lbFrames->setText(QCoreApplication::translate("PageSerialF0Read", "\345\256\214\346\225\264\345\270\247: 0", nullptr));
        lbLastCmd->setText(QCoreApplication::translate("PageSerialF0Read", "\346\234\200\345\220\216\345\221\275\344\273\244: -", nullptr));
        lbLastInterval->setText(QCoreApplication::translate("PageSerialF0Read", "\345\270\247\351\227\264\351\232\224: -", nullptr));
        groupLog->setTitle(QCoreApplication::translate("PageSerialF0Read", "\351\200\220\345\255\227\350\212\202\350\247\243\346\236\220\346\227\245\345\277\227\357\274\210\345\220\253\346\227\266\351\227\264\346\210\263\357\274\211", nullptr));
        textLog->setPlaceholderText(QCoreApplication::translate("PageSerialF0Read", "\347\202\271\345\207\273\"\345\217\221\351\200\201 F0 \346\214\207\344\273\244\"\345\274\200\345\247\213\351\200\220\345\255\227\350\212\202\350\247\243\346\236\220\342\200\246", nullptr));
        (void)PageSerialF0Read;
    } // retranslateUi

};

namespace Ui {
    class PageSerialF0Read: public Ui_PageSerialF0Read {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_F0READ_H
