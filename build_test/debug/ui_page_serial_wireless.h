/********************************************************************************
** Form generated from reading UI file 'page_serial_wireless.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_WIRELESS_H
#define UI_PAGE_SERIAL_WIRELESS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSerialWireless
{
public:
    QVBoxLayout *wlMainLayout;
    QGroupBox *groupOsdData;
    QHBoxLayout *osdDataLayout;
    QPushButton *btnGetOsdDataWireless;
    QSpacerItem *spacer_osdDataLayout;
    QVBoxLayout *middleLayout;
    QGroupBox *groupFreq;
    QVBoxLayout *freqMainLayout;
    QHBoxLayout *gridFreq;
    QLabel *lbBand;
    QComboBox *comboBand;
    QLabel *lbChan;
    QSpinBox *spinChannel;
    QLabel *lbHop;
    QComboBox *comboHop;
    QSpacerItem *horizontalSpacer_7;
    QHBoxLayout *freqBtnCol;
    QPushButton *btnGetFreq;
    QPushButton *btnSetFreq;
    QSpacerItem *horizontalSpacer_6;
    QSpacerItem *spacer_freqMainLayout;
    QGroupBox *groupStatus;
    QGridLayout *gridStatus;
    QPushButton *btnGetStatusSky;
    QSpacerItem *spacer_gridStatus;
    QPushButton *btnGetStatus;
    QPushButton *btnGetDistance;
    QSpacerItem *horizontalSpacer_5;
    QGroupBox *groupBbPwr;
    QGridLayout *groupBbPwr_grid;
    QLabel *lbBbPwr;
    QHBoxLayout *pwrBtnCol;
    QPushButton *btnGetBbPwr;
    QPushButton *btnSetBbPwr;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *spacer_groupBbPwr_grid;
    QComboBox *comboBbPwr;
    QSpacerItem *horizontalSpacer_4;
    QGroupBox *groupRelay;
    QVBoxLayout *relayMainLayout;
    QGroupBox *relaySubFreq;
    QVBoxLayout *relaySubFreqV;
    QHBoxLayout *relaySubFreqRow;
    QLabel *lbRelayBand;
    QComboBox *comboRelayBand;
    QLabel *lbRelayChan;
    QSpinBox *spinRelayChannel;
    QLabel *lbRelayHop;
    QComboBox *comboRelayHop;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *relayFreqBtnCol;
    QPushButton *btnGetRelayFreq;
    QPushButton *btnSetRelayFreq;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *spacer_relaySubFreqV;
    QGroupBox *relaySubStatus;
    QHBoxLayout *relaySubStatusH;
    QPushButton *btnGetRelayStatus;
    QPushButton *btnGetRelayGndDist;
    QPushButton *btnGetRelaySkyDist;
    QSpacerItem *spacer_relaySubStatusH;
    QGroupBox *relaySubBbPwr;
    QGridLayout *relaySubBbPwr_grid;
    QSpacerItem *spacer_relaySubBbPwr_grid;
    QLabel *lbRelayBbPwr;
    QHBoxLayout *relayPwrCol;
    QPushButton *btnGetRelayBbPwr;
    QPushButton *btnSetRelayBbPwr;
    QSpacerItem *horizontalSpacer_8;
    QComboBox *comboRelayBbPwr;
    QSpacerItem *horizontalSpacer_9;
    QGroupBox *groupPreview;
    QHBoxLayout *previewLayout;
    QLabel *lbPreviewHint;
    QLineEdit *editCmdPreview;
    QGroupBox *groupRelayData;
    QHBoxLayout *relayDataLayout;
    QTextEdit *textStatus;
    QPushButton *btnClearOsdDataWireless;

    void setupUi(QWidget *PageSerialWireless)
    {
        if (PageSerialWireless->objectName().isEmpty())
            PageSerialWireless->setObjectName(QString::fromUtf8("PageSerialWireless"));
        PageSerialWireless->resize(682, 1464);
        wlMainLayout = new QVBoxLayout(PageSerialWireless);
        wlMainLayout->setSpacing(10);
        wlMainLayout->setContentsMargins(8, 8, 8, 8);
        wlMainLayout->setObjectName(QString::fromUtf8("wlMainLayout"));
        groupOsdData = new QGroupBox(PageSerialWireless);
        groupOsdData->setObjectName(QString::fromUtf8("groupOsdData"));
        osdDataLayout = new QHBoxLayout(groupOsdData);
        osdDataLayout->setObjectName(QString::fromUtf8("osdDataLayout"));
        btnGetOsdDataWireless = new QPushButton(groupOsdData);
        btnGetOsdDataWireless->setObjectName(QString::fromUtf8("btnGetOsdDataWireless"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(btnGetOsdDataWireless->sizePolicy().hasHeightForWidth());
        btnGetOsdDataWireless->setSizePolicy(sizePolicy);
        btnGetOsdDataWireless->setMinimumSize(QSize(220, 36));
        btnGetOsdDataWireless->setMaximumSize(QSize(220, 36));

        osdDataLayout->addWidget(btnGetOsdDataWireless, 0, Qt::AlignVCenter);

        spacer_osdDataLayout = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        osdDataLayout->addItem(spacer_osdDataLayout);


        wlMainLayout->addWidget(groupOsdData);

        middleLayout = new QVBoxLayout();
        middleLayout->setSpacing(10);
        middleLayout->setObjectName(QString::fromUtf8("middleLayout"));
        groupFreq = new QGroupBox(PageSerialWireless);
        groupFreq->setObjectName(QString::fromUtf8("groupFreq"));
        freqMainLayout = new QVBoxLayout(groupFreq);
        freqMainLayout->setSpacing(12);
        freqMainLayout->setObjectName(QString::fromUtf8("freqMainLayout"));
        gridFreq = new QHBoxLayout();
        gridFreq->setSpacing(10);
        gridFreq->setObjectName(QString::fromUtf8("gridFreq"));
        lbBand = new QLabel(groupFreq);
        lbBand->setObjectName(QString::fromUtf8("lbBand"));
        lbBand->setMinimumSize(QSize(64, 28));
        lbBand->setMaximumSize(QSize(64, 28));
        lbBand->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridFreq->addWidget(lbBand, 0, Qt::AlignVCenter);

        comboBand = new QComboBox(groupFreq);
        comboBand->addItem(QString());
        comboBand->addItem(QString());
        comboBand->addItem(QString());
        comboBand->addItem(QString());
        comboBand->addItem(QString());
        comboBand->setObjectName(QString::fromUtf8("comboBand"));
        comboBand->setMinimumSize(QSize(110, 30));
        comboBand->setMaximumSize(QSize(110, 30));

        gridFreq->addWidget(comboBand, 0, Qt::AlignVCenter);

        lbChan = new QLabel(groupFreq);
        lbChan->setObjectName(QString::fromUtf8("lbChan"));
        lbChan->setMinimumSize(QSize(64, 28));
        lbChan->setMaximumSize(QSize(64, 28));
        lbChan->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridFreq->addWidget(lbChan, 0, Qt::AlignVCenter);

        spinChannel = new QSpinBox(groupFreq);
        spinChannel->setObjectName(QString::fromUtf8("spinChannel"));
        spinChannel->setMinimumSize(QSize(110, 30));
        spinChannel->setMaximumSize(QSize(110, 30));
        spinChannel->setMaximum(48);

        gridFreq->addWidget(spinChannel, 0, Qt::AlignVCenter);

        lbHop = new QLabel(groupFreq);
        lbHop->setObjectName(QString::fromUtf8("lbHop"));
        lbHop->setMinimumSize(QSize(64, 28));
        lbHop->setMaximumSize(QSize(64, 28));
        lbHop->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        gridFreq->addWidget(lbHop, 0, Qt::AlignVCenter);

        comboHop = new QComboBox(groupFreq);
        comboHop->addItem(QString());
        comboHop->addItem(QString());
        comboHop->setObjectName(QString::fromUtf8("comboHop"));
        comboHop->setMinimumSize(QSize(110, 30));
        comboHop->setMaximumSize(QSize(110, 30));

        gridFreq->addWidget(comboHop, 0, Qt::AlignVCenter);

        horizontalSpacer_7 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridFreq->addItem(horizontalSpacer_7);


        freqMainLayout->addLayout(gridFreq);

        freqBtnCol = new QHBoxLayout();
        freqBtnCol->setSpacing(10);
        freqBtnCol->setObjectName(QString::fromUtf8("freqBtnCol"));
        btnGetFreq = new QPushButton(groupFreq);
        btnGetFreq->setObjectName(QString::fromUtf8("btnGetFreq"));
        sizePolicy.setHeightForWidth(btnGetFreq->sizePolicy().hasHeightForWidth());
        btnGetFreq->setSizePolicy(sizePolicy);
        btnGetFreq->setMinimumSize(QSize(182, 36));
        btnGetFreq->setMaximumSize(QSize(182, 36));

        freqBtnCol->addWidget(btnGetFreq, 0, Qt::AlignVCenter);

        btnSetFreq = new QPushButton(groupFreq);
        btnSetFreq->setObjectName(QString::fromUtf8("btnSetFreq"));
        sizePolicy.setHeightForWidth(btnSetFreq->sizePolicy().hasHeightForWidth());
        btnSetFreq->setSizePolicy(sizePolicy);
        btnSetFreq->setMinimumSize(QSize(182, 36));
        btnSetFreq->setMaximumSize(QSize(182, 36));

        freqBtnCol->addWidget(btnSetFreq, 0, Qt::AlignVCenter);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        freqBtnCol->addItem(horizontalSpacer_6);


        freqMainLayout->addLayout(freqBtnCol);

        spacer_freqMainLayout = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        freqMainLayout->addItem(spacer_freqMainLayout);


        middleLayout->addWidget(groupFreq);

        groupStatus = new QGroupBox(PageSerialWireless);
        groupStatus->setObjectName(QString::fromUtf8("groupStatus"));
        gridStatus = new QGridLayout(groupStatus);
        gridStatus->setSpacing(8);
        gridStatus->setObjectName(QString::fromUtf8("gridStatus"));
        btnGetStatusSky = new QPushButton(groupStatus);
        btnGetStatusSky->setObjectName(QString::fromUtf8("btnGetStatusSky"));
        sizePolicy.setHeightForWidth(btnGetStatusSky->sizePolicy().hasHeightForWidth());
        btnGetStatusSky->setSizePolicy(sizePolicy);
        btnGetStatusSky->setMinimumSize(QSize(182, 36));
        btnGetStatusSky->setMaximumSize(QSize(182, 36));

        gridStatus->addWidget(btnGetStatusSky, 2, 1, 1, 1, Qt::AlignVCenter);

        spacer_gridStatus = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridStatus->addItem(spacer_gridStatus, 3, 0, 1, 1);

        btnGetStatus = new QPushButton(groupStatus);
        btnGetStatus->setObjectName(QString::fromUtf8("btnGetStatus"));
        sizePolicy.setHeightForWidth(btnGetStatus->sizePolicy().hasHeightForWidth());
        btnGetStatus->setSizePolicy(sizePolicy);
        btnGetStatus->setMinimumSize(QSize(182, 36));
        btnGetStatus->setMaximumSize(QSize(182, 36));

        gridStatus->addWidget(btnGetStatus, 2, 0, 1, 1, Qt::AlignVCenter);

        btnGetDistance = new QPushButton(groupStatus);
        btnGetDistance->setObjectName(QString::fromUtf8("btnGetDistance"));
        sizePolicy.setHeightForWidth(btnGetDistance->sizePolicy().hasHeightForWidth());
        btnGetDistance->setSizePolicy(sizePolicy);
        btnGetDistance->setMinimumSize(QSize(182, 36));
        btnGetDistance->setMaximumSize(QSize(182, 36));

        gridStatus->addWidget(btnGetDistance, 2, 2, 1, 1, Qt::AlignVCenter);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridStatus->addItem(horizontalSpacer_5, 2, 3, 1, 1);


        middleLayout->addWidget(groupStatus);

        groupBbPwr = new QGroupBox(PageSerialWireless);
        groupBbPwr->setObjectName(QString::fromUtf8("groupBbPwr"));
        groupBbPwr_grid = new QGridLayout(groupBbPwr);
        groupBbPwr_grid->setObjectName(QString::fromUtf8("groupBbPwr_grid"));
        lbBbPwr = new QLabel(groupBbPwr);
        lbBbPwr->setObjectName(QString::fromUtf8("lbBbPwr"));
        lbBbPwr->setMinimumSize(QSize(64, 28));
        lbBbPwr->setMaximumSize(QSize(64, 28));
        lbBbPwr->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        groupBbPwr_grid->addWidget(lbBbPwr, 0, 0, 1, 1, Qt::AlignVCenter);

        pwrBtnCol = new QHBoxLayout();
        pwrBtnCol->setSpacing(10);
        pwrBtnCol->setObjectName(QString::fromUtf8("pwrBtnCol"));
        btnGetBbPwr = new QPushButton(groupBbPwr);
        btnGetBbPwr->setObjectName(QString::fromUtf8("btnGetBbPwr"));
        sizePolicy.setHeightForWidth(btnGetBbPwr->sizePolicy().hasHeightForWidth());
        btnGetBbPwr->setSizePolicy(sizePolicy);
        btnGetBbPwr->setMinimumSize(QSize(182, 36));
        btnGetBbPwr->setMaximumSize(QSize(182, 36));

        pwrBtnCol->addWidget(btnGetBbPwr, 0, Qt::AlignVCenter);

        btnSetBbPwr = new QPushButton(groupBbPwr);
        btnSetBbPwr->setObjectName(QString::fromUtf8("btnSetBbPwr"));
        sizePolicy.setHeightForWidth(btnSetBbPwr->sizePolicy().hasHeightForWidth());
        btnSetBbPwr->setSizePolicy(sizePolicy);
        btnSetBbPwr->setMinimumSize(QSize(182, 36));
        btnSetBbPwr->setMaximumSize(QSize(182, 36));

        pwrBtnCol->addWidget(btnSetBbPwr, 0, Qt::AlignVCenter);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        pwrBtnCol->addItem(horizontalSpacer_3);


        groupBbPwr_grid->addLayout(pwrBtnCol, 1, 0, 1, 2);

        spacer_groupBbPwr_grid = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        groupBbPwr_grid->addItem(spacer_groupBbPwr_grid, 2, 0, 1, 1);

        comboBbPwr = new QComboBox(groupBbPwr);
        comboBbPwr->setObjectName(QString::fromUtf8("comboBbPwr"));
        comboBbPwr->setMinimumSize(QSize(150, 30));
        comboBbPwr->setMaximumSize(QSize(150, 30));

        groupBbPwr_grid->addWidget(comboBbPwr, 0, 1, 1, 1, Qt::AlignVCenter);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        groupBbPwr_grid->addItem(horizontalSpacer_4, 0, 2, 1, 1);


        middleLayout->addWidget(groupBbPwr);

        groupRelay = new QGroupBox(PageSerialWireless);
        groupRelay->setObjectName(QString::fromUtf8("groupRelay"));
        relayMainLayout = new QVBoxLayout(groupRelay);
        relayMainLayout->setSpacing(10);
        relayMainLayout->setObjectName(QString::fromUtf8("relayMainLayout"));
        relaySubFreq = new QGroupBox(groupRelay);
        relaySubFreq->setObjectName(QString::fromUtf8("relaySubFreq"));
        relaySubFreqV = new QVBoxLayout(relaySubFreq);
        relaySubFreqV->setSpacing(10);
        relaySubFreqV->setObjectName(QString::fromUtf8("relaySubFreqV"));
        relaySubFreqRow = new QHBoxLayout();
        relaySubFreqRow->setSpacing(10);
        relaySubFreqRow->setObjectName(QString::fromUtf8("relaySubFreqRow"));
        lbRelayBand = new QLabel(relaySubFreq);
        lbRelayBand->setObjectName(QString::fromUtf8("lbRelayBand"));
        lbRelayBand->setMinimumSize(QSize(64, 28));
        lbRelayBand->setMaximumSize(QSize(64, 28));
        lbRelayBand->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        relaySubFreqRow->addWidget(lbRelayBand, 0, Qt::AlignVCenter);

        comboRelayBand = new QComboBox(relaySubFreq);
        comboRelayBand->addItem(QString());
        comboRelayBand->addItem(QString());
        comboRelayBand->addItem(QString());
        comboRelayBand->addItem(QString());
        comboRelayBand->addItem(QString());
        comboRelayBand->setObjectName(QString::fromUtf8("comboRelayBand"));
        comboRelayBand->setMinimumSize(QSize(110, 30));
        comboRelayBand->setMaximumSize(QSize(110, 30));

        relaySubFreqRow->addWidget(comboRelayBand, 0, Qt::AlignVCenter);

        lbRelayChan = new QLabel(relaySubFreq);
        lbRelayChan->setObjectName(QString::fromUtf8("lbRelayChan"));
        lbRelayChan->setMinimumSize(QSize(64, 28));
        lbRelayChan->setMaximumSize(QSize(64, 28));
        lbRelayChan->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        relaySubFreqRow->addWidget(lbRelayChan, 0, Qt::AlignVCenter);

        spinRelayChannel = new QSpinBox(relaySubFreq);
        spinRelayChannel->setObjectName(QString::fromUtf8("spinRelayChannel"));
        spinRelayChannel->setMinimumSize(QSize(110, 30));
        spinRelayChannel->setMaximumSize(QSize(110, 30));
        spinRelayChannel->setMaximum(48);

        relaySubFreqRow->addWidget(spinRelayChannel, 0, Qt::AlignVCenter);

        lbRelayHop = new QLabel(relaySubFreq);
        lbRelayHop->setObjectName(QString::fromUtf8("lbRelayHop"));
        lbRelayHop->setMinimumSize(QSize(64, 28));
        lbRelayHop->setMaximumSize(QSize(64, 28));
        lbRelayHop->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        relaySubFreqRow->addWidget(lbRelayHop, 0, Qt::AlignVCenter);

        comboRelayHop = new QComboBox(relaySubFreq);
        comboRelayHop->addItem(QString());
        comboRelayHop->addItem(QString());
        comboRelayHop->setObjectName(QString::fromUtf8("comboRelayHop"));
        comboRelayHop->setMinimumSize(QSize(110, 30));
        comboRelayHop->setMaximumSize(QSize(110, 30));

        relaySubFreqRow->addWidget(comboRelayHop, 0, Qt::AlignVCenter);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        relaySubFreqRow->addItem(horizontalSpacer_2);


        relaySubFreqV->addLayout(relaySubFreqRow);

        relayFreqBtnCol = new QHBoxLayout();
        relayFreqBtnCol->setSpacing(10);
        relayFreqBtnCol->setObjectName(QString::fromUtf8("relayFreqBtnCol"));
        btnGetRelayFreq = new QPushButton(relaySubFreq);
        btnGetRelayFreq->setObjectName(QString::fromUtf8("btnGetRelayFreq"));
        sizePolicy.setHeightForWidth(btnGetRelayFreq->sizePolicy().hasHeightForWidth());
        btnGetRelayFreq->setSizePolicy(sizePolicy);
        btnGetRelayFreq->setMinimumSize(QSize(182, 36));
        btnGetRelayFreq->setMaximumSize(QSize(182, 36));

        relayFreqBtnCol->addWidget(btnGetRelayFreq, 0, Qt::AlignVCenter);

        btnSetRelayFreq = new QPushButton(relaySubFreq);
        btnSetRelayFreq->setObjectName(QString::fromUtf8("btnSetRelayFreq"));
        sizePolicy.setHeightForWidth(btnSetRelayFreq->sizePolicy().hasHeightForWidth());
        btnSetRelayFreq->setSizePolicy(sizePolicy);
        btnSetRelayFreq->setMinimumSize(QSize(182, 36));
        btnSetRelayFreq->setMaximumSize(QSize(182, 36));

        relayFreqBtnCol->addWidget(btnSetRelayFreq, 0, Qt::AlignVCenter);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        relayFreqBtnCol->addItem(horizontalSpacer);


        relaySubFreqV->addLayout(relayFreqBtnCol);

        spacer_relaySubFreqV = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        relaySubFreqV->addItem(spacer_relaySubFreqV);


        relayMainLayout->addWidget(relaySubFreq);

        relaySubStatus = new QGroupBox(groupRelay);
        relaySubStatus->setObjectName(QString::fromUtf8("relaySubStatus"));
        relaySubStatusH = new QHBoxLayout(relaySubStatus);
        relaySubStatusH->setSpacing(10);
        relaySubStatusH->setObjectName(QString::fromUtf8("relaySubStatusH"));
        btnGetRelayStatus = new QPushButton(relaySubStatus);
        btnGetRelayStatus->setObjectName(QString::fromUtf8("btnGetRelayStatus"));
        sizePolicy.setHeightForWidth(btnGetRelayStatus->sizePolicy().hasHeightForWidth());
        btnGetRelayStatus->setSizePolicy(sizePolicy);
        btnGetRelayStatus->setMinimumSize(QSize(182, 36));
        btnGetRelayStatus->setMaximumSize(QSize(182, 36));

        relaySubStatusH->addWidget(btnGetRelayStatus, 0, Qt::AlignVCenter);

        btnGetRelayGndDist = new QPushButton(relaySubStatus);
        btnGetRelayGndDist->setObjectName(QString::fromUtf8("btnGetRelayGndDist"));
        sizePolicy.setHeightForWidth(btnGetRelayGndDist->sizePolicy().hasHeightForWidth());
        btnGetRelayGndDist->setSizePolicy(sizePolicy);
        btnGetRelayGndDist->setMinimumSize(QSize(182, 36));
        btnGetRelayGndDist->setMaximumSize(QSize(182, 36));

        relaySubStatusH->addWidget(btnGetRelayGndDist, 0, Qt::AlignVCenter);

        btnGetRelaySkyDist = new QPushButton(relaySubStatus);
        btnGetRelaySkyDist->setObjectName(QString::fromUtf8("btnGetRelaySkyDist"));
        sizePolicy.setHeightForWidth(btnGetRelaySkyDist->sizePolicy().hasHeightForWidth());
        btnGetRelaySkyDist->setSizePolicy(sizePolicy);
        btnGetRelaySkyDist->setMinimumSize(QSize(182, 36));
        btnGetRelaySkyDist->setMaximumSize(QSize(182, 36));

        relaySubStatusH->addWidget(btnGetRelaySkyDist, 0, Qt::AlignVCenter);

        spacer_relaySubStatusH = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        relaySubStatusH->addItem(spacer_relaySubStatusH);


        relayMainLayout->addWidget(relaySubStatus);

        relaySubBbPwr = new QGroupBox(groupRelay);
        relaySubBbPwr->setObjectName(QString::fromUtf8("relaySubBbPwr"));
        relaySubBbPwr_grid = new QGridLayout(relaySubBbPwr);
        relaySubBbPwr_grid->setObjectName(QString::fromUtf8("relaySubBbPwr_grid"));
        spacer_relaySubBbPwr_grid = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        relaySubBbPwr_grid->addItem(spacer_relaySubBbPwr_grid, 2, 0, 1, 1);

        lbRelayBbPwr = new QLabel(relaySubBbPwr);
        lbRelayBbPwr->setObjectName(QString::fromUtf8("lbRelayBbPwr"));
        lbRelayBbPwr->setMinimumSize(QSize(64, 28));
        lbRelayBbPwr->setMaximumSize(QSize(64, 28));
        lbRelayBbPwr->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        relaySubBbPwr_grid->addWidget(lbRelayBbPwr, 0, 0, 1, 1, Qt::AlignVCenter);

        relayPwrCol = new QHBoxLayout();
        relayPwrCol->setSpacing(10);
        relayPwrCol->setObjectName(QString::fromUtf8("relayPwrCol"));
        btnGetRelayBbPwr = new QPushButton(relaySubBbPwr);
        btnGetRelayBbPwr->setObjectName(QString::fromUtf8("btnGetRelayBbPwr"));
        sizePolicy.setHeightForWidth(btnGetRelayBbPwr->sizePolicy().hasHeightForWidth());
        btnGetRelayBbPwr->setSizePolicy(sizePolicy);
        btnGetRelayBbPwr->setMinimumSize(QSize(182, 36));
        btnGetRelayBbPwr->setMaximumSize(QSize(182, 36));

        relayPwrCol->addWidget(btnGetRelayBbPwr, 0, Qt::AlignVCenter);

        btnSetRelayBbPwr = new QPushButton(relaySubBbPwr);
        btnSetRelayBbPwr->setObjectName(QString::fromUtf8("btnSetRelayBbPwr"));
        sizePolicy.setHeightForWidth(btnSetRelayBbPwr->sizePolicy().hasHeightForWidth());
        btnSetRelayBbPwr->setSizePolicy(sizePolicy);
        btnSetRelayBbPwr->setMinimumSize(QSize(182, 36));
        btnSetRelayBbPwr->setMaximumSize(QSize(182, 36));

        relayPwrCol->addWidget(btnSetRelayBbPwr, 0, Qt::AlignVCenter);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        relayPwrCol->addItem(horizontalSpacer_8);


        relaySubBbPwr_grid->addLayout(relayPwrCol, 1, 0, 1, 2);

        comboRelayBbPwr = new QComboBox(relaySubBbPwr);
        comboRelayBbPwr->setObjectName(QString::fromUtf8("comboRelayBbPwr"));
        comboRelayBbPwr->setMinimumSize(QSize(150, 30));
        comboRelayBbPwr->setMaximumSize(QSize(150, 30));

        relaySubBbPwr_grid->addWidget(comboRelayBbPwr, 0, 1, 1, 1, Qt::AlignVCenter);

        horizontalSpacer_9 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        relaySubBbPwr_grid->addItem(horizontalSpacer_9, 0, 2, 1, 1);


        relayMainLayout->addWidget(relaySubBbPwr);


        middleLayout->addWidget(groupRelay);


        wlMainLayout->addLayout(middleLayout);

        groupPreview = new QGroupBox(PageSerialWireless);
        groupPreview->setObjectName(QString::fromUtf8("groupPreview"));
        previewLayout = new QHBoxLayout(groupPreview);
        previewLayout->setSpacing(10);
        previewLayout->setObjectName(QString::fromUtf8("previewLayout"));
        lbPreviewHint = new QLabel(groupPreview);
        lbPreviewHint->setObjectName(QString::fromUtf8("lbPreviewHint"));
        lbPreviewHint->setMinimumSize(QSize(64, 30));
        lbPreviewHint->setMaximumSize(QSize(64, 30));
        lbPreviewHint->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        previewLayout->addWidget(lbPreviewHint, 0, Qt::AlignVCenter);

        editCmdPreview = new QLineEdit(groupPreview);
        editCmdPreview->setObjectName(QString::fromUtf8("editCmdPreview"));
        editCmdPreview->setMinimumSize(QSize(0, 30));
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        font.setPointSize(10);
        editCmdPreview->setFont(font);
        editCmdPreview->setReadOnly(true);

        previewLayout->addWidget(editCmdPreview);


        wlMainLayout->addWidget(groupPreview);

        groupRelayData = new QGroupBox(PageSerialWireless);
        groupRelayData->setObjectName(QString::fromUtf8("groupRelayData"));
        relayDataLayout = new QHBoxLayout(groupRelayData);
        relayDataLayout->setSpacing(10);
        relayDataLayout->setObjectName(QString::fromUtf8("relayDataLayout"));
        textStatus = new QTextEdit(groupRelayData);
        textStatus->setObjectName(QString::fromUtf8("textStatus"));
        textStatus->setFont(font);
        textStatus->setReadOnly(true);

        relayDataLayout->addWidget(textStatus);

        btnClearOsdDataWireless = new QPushButton(groupRelayData);
        btnClearOsdDataWireless->setObjectName(QString::fromUtf8("btnClearOsdDataWireless"));
        sizePolicy.setHeightForWidth(btnClearOsdDataWireless->sizePolicy().hasHeightForWidth());
        btnClearOsdDataWireless->setSizePolicy(sizePolicy);
        btnClearOsdDataWireless->setMinimumSize(QSize(80, 36));
        btnClearOsdDataWireless->setMaximumSize(QSize(80, 36));

        relayDataLayout->addWidget(btnClearOsdDataWireless, 0, Qt::AlignVCenter);


        wlMainLayout->addWidget(groupRelayData);


        retranslateUi(PageSerialWireless);

        comboRelayBand->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(PageSerialWireless);
    } // setupUi

    void retranslateUi(QWidget *PageSerialWireless)
    {
        groupOsdData->setTitle(QCoreApplication::translate("PageSerialWireless", "OSD Data", nullptr));
        btnGetOsdDataWireless->setText(QCoreApplication::translate("PageSerialWireless", "Get OSD Data (0x57)", nullptr));
        groupFreq->setTitle(QCoreApplication::translate("PageSerialWireless", "\351\242\221\347\202\271\350\256\276\347\275\256", nullptr));
        lbBand->setText(QCoreApplication::translate("PageSerialWireless", "Band:", nullptr));
        comboBand->setItemText(0, QCoreApplication::translate("PageSerialWireless", "0", nullptr));
        comboBand->setItemText(1, QCoreApplication::translate("PageSerialWireless", "1", nullptr));
        comboBand->setItemText(2, QCoreApplication::translate("PageSerialWireless", "2", nullptr));
        comboBand->setItemText(3, QCoreApplication::translate("PageSerialWireless", "3", nullptr));
        comboBand->setItemText(4, QCoreApplication::translate("PageSerialWireless", "4", nullptr));

        lbChan->setText(QCoreApplication::translate("PageSerialWireless", "Channel:", nullptr));
        lbHop->setText(QCoreApplication::translate("PageSerialWireless", "Mode:", nullptr));
        comboHop->setItemText(0, QCoreApplication::translate("PageSerialWireless", "Fixed (0)", nullptr));
        comboHop->setItemText(1, QCoreApplication::translate("PageSerialWireless", "Hopping (1)", nullptr));

        btnGetFreq->setText(QCoreApplication::translate("PageSerialWireless", "Get \351\242\221\347\202\271", nullptr));
        btnSetFreq->setText(QCoreApplication::translate("PageSerialWireless", "Set \351\242\221\347\202\271", nullptr));
        groupStatus->setTitle(QCoreApplication::translate("PageSerialWireless", "Status & Parameters", nullptr));
        btnGetStatusSky->setText(QCoreApplication::translate("PageSerialWireless", "Get Status (0x55)", nullptr));
        btnGetStatus->setText(QCoreApplication::translate("PageSerialWireless", "Get Status (0x52)", nullptr));
        btnGetDistance->setText(QCoreApplication::translate("PageSerialWireless", "Get Distance (0x56)", nullptr));
        groupBbPwr->setTitle(QCoreApplication::translate("PageSerialWireless", "\345\237\272\345\270\246\345\212\237\347\216\207", nullptr));
        lbBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "BB Pwr:", nullptr));
        btnGetBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "Get (0x53)", nullptr));
        btnSetBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "Set (0x54)", nullptr));
        groupRelay->setTitle(QCoreApplication::translate("PageSerialWireless", "Relay", nullptr));
        relaySubFreq->setTitle(QCoreApplication::translate("PageSerialWireless", "\351\242\221\347\202\271\350\256\276\347\275\256", nullptr));
        lbRelayBand->setText(QCoreApplication::translate("PageSerialWireless", "Band:", nullptr));
        comboRelayBand->setItemText(0, QCoreApplication::translate("PageSerialWireless", "0", nullptr));
        comboRelayBand->setItemText(1, QCoreApplication::translate("PageSerialWireless", "1", nullptr));
        comboRelayBand->setItemText(2, QCoreApplication::translate("PageSerialWireless", "2", nullptr));
        comboRelayBand->setItemText(3, QCoreApplication::translate("PageSerialWireless", "3", nullptr));
        comboRelayBand->setItemText(4, QCoreApplication::translate("PageSerialWireless", "4", nullptr));

        lbRelayChan->setText(QCoreApplication::translate("PageSerialWireless", "Channel:", nullptr));
        lbRelayHop->setText(QCoreApplication::translate("PageSerialWireless", "Mode:", nullptr));
        comboRelayHop->setItemText(0, QCoreApplication::translate("PageSerialWireless", "Fixed (0)", nullptr));
        comboRelayHop->setItemText(1, QCoreApplication::translate("PageSerialWireless", "Hopping (1)", nullptr));

        btnGetRelayFreq->setText(QCoreApplication::translate("PageSerialWireless", "Get (0xA1)", nullptr));
        btnSetRelayFreq->setText(QCoreApplication::translate("PageSerialWireless", "Set (0xA0)", nullptr));
        relaySubStatus->setTitle(QCoreApplication::translate("PageSerialWireless", "\347\212\266\346\200\201\344\270\216\345\217\202\346\225\260", nullptr));
        btnGetRelayStatus->setText(QCoreApplication::translate("PageSerialWireless", "Get Status (0xA2)", nullptr));
        btnGetRelayGndDist->setText(QCoreApplication::translate("PageSerialWireless", "Get Gnd Dist (0xA5)", nullptr));
        btnGetRelaySkyDist->setText(QCoreApplication::translate("PageSerialWireless", "Get Sky Dist (0xA6)", nullptr));
        relaySubBbPwr->setTitle(QCoreApplication::translate("PageSerialWireless", "\344\270\255\347\273\247\345\237\272\345\270\246\345\212\237\347\216\207", nullptr));
        lbRelayBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "BB Pwr:", nullptr));
        btnGetRelayBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "Get (0xA3)", nullptr));
        btnSetRelayBbPwr->setText(QCoreApplication::translate("PageSerialWireless", "Set (0xA4)", nullptr));
        groupPreview->setTitle(QCoreApplication::translate("PageSerialWireless", "Command Preview", nullptr));
        lbPreviewHint->setText(QCoreApplication::translate("PageSerialWireless", "Preview:", nullptr));
        groupRelayData->setTitle(QCoreApplication::translate("PageSerialWireless", "Relay Data", nullptr));
        btnClearOsdDataWireless->setText(QCoreApplication::translate("PageSerialWireless", "Clear", nullptr));
        (void)PageSerialWireless;
    } // retranslateUi

};

namespace Ui {
    class PageSerialWireless: public Ui_PageSerialWireless {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_WIRELESS_H
