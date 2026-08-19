/********************************************************************************
** Form generated from reading UI file 'page_serial_custom.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_CUSTOM_H
#define UI_PAGE_SERIAL_CUSTOM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
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

class Ui_PageSerialCustom
{
public:
    QVBoxLayout *custMainLayout;
    QGroupBox *groupTemplate;
    QGridLayout *gridTemplate;
    QGroupBox *groupInput;
    QVBoxLayout *inputLayout;
    QLabel *lbHex;
    QTextEdit *editCustomInput;
    QHBoxLayout *checkLayout;
    QPushButton *btnCheckFormat;
    QLabel *labelFormatStatus;
    QSpacerItem *spCheck;
    QGroupBox *groupSendCtrl;
    QGridLayout *gridSend;
    QLabel *lbCount;
    QSpinBox *spinSendCount;
    QLabel *lbInterval;
    QSpinBox *spinInterval;
    QCheckBox *checkRepeat;
    QPushButton *btnSendCustom;

    void setupUi(QWidget *PageSerialCustom)
    {
        if (PageSerialCustom->objectName().isEmpty())
            PageSerialCustom->setObjectName(QString::fromUtf8("PageSerialCustom"));
        custMainLayout = new QVBoxLayout(PageSerialCustom);
        custMainLayout->setObjectName(QString::fromUtf8("custMainLayout"));
        groupTemplate = new QGroupBox(PageSerialCustom);
        groupTemplate->setObjectName(QString::fromUtf8("groupTemplate"));
        gridTemplate = new QGridLayout(groupTemplate);
        gridTemplate->setObjectName(QString::fromUtf8("gridTemplate"));

        custMainLayout->addWidget(groupTemplate);

        groupInput = new QGroupBox(PageSerialCustom);
        groupInput->setObjectName(QString::fromUtf8("groupInput"));
        inputLayout = new QVBoxLayout(groupInput);
        inputLayout->setObjectName(QString::fromUtf8("inputLayout"));
        lbHex = new QLabel(groupInput);
        lbHex->setObjectName(QString::fromUtf8("lbHex"));

        inputLayout->addWidget(lbHex);

        editCustomInput = new QTextEdit(groupInput);
        editCustomInput->setObjectName(QString::fromUtf8("editCustomInput"));
        editCustomInput->setMaximumSize(QSize(16777215, 100));

        inputLayout->addWidget(editCustomInput);

        checkLayout = new QHBoxLayout();
        checkLayout->setObjectName(QString::fromUtf8("checkLayout"));
        btnCheckFormat = new QPushButton(groupInput);
        btnCheckFormat->setObjectName(QString::fromUtf8("btnCheckFormat"));

        checkLayout->addWidget(btnCheckFormat);

        labelFormatStatus = new QLabel(groupInput);
        labelFormatStatus->setObjectName(QString::fromUtf8("labelFormatStatus"));

        checkLayout->addWidget(labelFormatStatus);

        spCheck = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        checkLayout->addItem(spCheck);


        inputLayout->addLayout(checkLayout);


        custMainLayout->addWidget(groupInput);

        groupSendCtrl = new QGroupBox(PageSerialCustom);
        groupSendCtrl->setObjectName(QString::fromUtf8("groupSendCtrl"));
        gridSend = new QGridLayout(groupSendCtrl);
        gridSend->setObjectName(QString::fromUtf8("gridSend"));
        lbCount = new QLabel(groupSendCtrl);
        lbCount->setObjectName(QString::fromUtf8("lbCount"));

        gridSend->addWidget(lbCount, 0, 0, 1, 1);

        spinSendCount = new QSpinBox(groupSendCtrl);
        spinSendCount->setObjectName(QString::fromUtf8("spinSendCount"));
        spinSendCount->setMinimum(1);
        spinSendCount->setMaximum(100);
        spinSendCount->setValue(1);

        gridSend->addWidget(spinSendCount, 0, 1, 1, 1);

        lbInterval = new QLabel(groupSendCtrl);
        lbInterval->setObjectName(QString::fromUtf8("lbInterval"));

        gridSend->addWidget(lbInterval, 1, 0, 1, 1);

        spinInterval = new QSpinBox(groupSendCtrl);
        spinInterval->setObjectName(QString::fromUtf8("spinInterval"));
        spinInterval->setMinimum(10);
        spinInterval->setMaximum(10000);
        spinInterval->setValue(100);

        gridSend->addWidget(spinInterval, 1, 1, 1, 1);

        checkRepeat = new QCheckBox(groupSendCtrl);
        checkRepeat->setObjectName(QString::fromUtf8("checkRepeat"));

        gridSend->addWidget(checkRepeat, 2, 0, 1, 2);

        btnSendCustom = new QPushButton(groupSendCtrl);
        btnSendCustom->setObjectName(QString::fromUtf8("btnSendCustom"));
        btnSendCustom->setMinimumSize(QSize(0, 50));
        btnSendCustom->setStyleSheet(QString::fromUtf8("QPushButton{background-color:#FF5722;color:white;font-weight:bold;font-size:12pt;}"));

        gridSend->addWidget(btnSendCustom, 3, 0, 1, 2);


        custMainLayout->addWidget(groupSendCtrl);


        retranslateUi(PageSerialCustom);

        QMetaObject::connectSlotsByName(PageSerialCustom);
    } // setupUi

    void retranslateUi(QWidget *PageSerialCustom)
    {
        groupTemplate->setTitle(QCoreApplication::translate("PageSerialCustom", "Templates", nullptr));
        groupInput->setTitle(QCoreApplication::translate("PageSerialCustom", "Custom Command Input", nullptr));
        lbHex->setText(QCoreApplication::translate("PageSerialCustom", "Hex (space separated):", nullptr));
        btnCheckFormat->setText(QCoreApplication::translate("PageSerialCustom", "Check Format", nullptr));
        labelFormatStatus->setText(QString());
        groupSendCtrl->setTitle(QCoreApplication::translate("PageSerialCustom", "Send Control", nullptr));
        lbCount->setText(QCoreApplication::translate("PageSerialCustom", "Count:", nullptr));
        lbInterval->setText(QCoreApplication::translate("PageSerialCustom", "Interval(ms):", nullptr));
        checkRepeat->setText(QCoreApplication::translate("PageSerialCustom", "Repeat", nullptr));
        btnSendCustom->setText(QCoreApplication::translate("PageSerialCustom", "Send Command", nullptr));
        (void)PageSerialCustom;
    } // retranslateUi

};

namespace Ui {
    class PageSerialCustom: public Ui_PageSerialCustom {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_CUSTOM_H
