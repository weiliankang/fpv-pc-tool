/********************************************************************************
** Form generated from reading UI file 'page_serial_keycontrol.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_KEYCONTROL_H
#define UI_PAGE_SERIAL_KEYCONTROL_H

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
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSerialKeyControl
{
public:
    QHBoxLayout *keyMainLayout;
    QGroupBox *groupDirection;
    QVBoxLayout *dirLayout;
    QGridLayout *dirGrid;
    QPushButton *btnUp;
    QPushButton *btnLeft;
    QPushButton *btnOk;
    QPushButton *btnRight;
    QPushButton *btnDown;
    QSpacerItem *dirSpacerTop;
    QGroupBox *groupFunction;
    QVBoxLayout *funcLayout;
    QPushButton *btnBack;
    QPushButton *btnPair;
    QPushButton *btnUpdate;
    QPushButton *btnRecord;
    QPushButton *btnForce720p;
    QPushButton *btnDebug3;
    QSpacerItem *funcSpacerTop;
    QGroupBox *groupCustomKey;
    QVBoxLayout *ckLayout;
    QLabel *lbKeyName;
    QComboBox *comboKey;
    QLabel *lbPressType;
    QComboBox *comboPressType;
    QPushButton *btnSendCustomKey;
    QLabel *lbPreview;
    QLineEdit *editKeyPreview;
    QSpacerItem *ckSpacerTop;

    void setupUi(QWidget *PageSerialKeyControl)
    {
        if (PageSerialKeyControl->objectName().isEmpty())
            PageSerialKeyControl->setObjectName(QString::fromUtf8("PageSerialKeyControl"));
        PageSerialKeyControl->resize(769, 349);
        keyMainLayout = new QHBoxLayout(PageSerialKeyControl);
        keyMainLayout->setObjectName(QString::fromUtf8("keyMainLayout"));
        groupDirection = new QGroupBox(PageSerialKeyControl);
        groupDirection->setObjectName(QString::fromUtf8("groupDirection"));
        dirLayout = new QVBoxLayout(groupDirection);
        dirLayout->setObjectName(QString::fromUtf8("dirLayout"));
        dirGrid = new QGridLayout();
        dirGrid->setObjectName(QString::fromUtf8("dirGrid"));
        dirGrid->setHorizontalSpacing(12);
        dirGrid->setVerticalSpacing(12);
        btnUp = new QPushButton(groupDirection);
        btnUp->setObjectName(QString::fromUtf8("btnUp"));
        btnUp->setMinimumSize(QSize(135, 48));
        btnUp->setMaximumSize(QSize(135, 48));

        dirGrid->addWidget(btnUp, 0, 1, 1, 1);

        btnLeft = new QPushButton(groupDirection);
        btnLeft->setObjectName(QString::fromUtf8("btnLeft"));
        btnLeft->setMinimumSize(QSize(135, 48));
        btnLeft->setMaximumSize(QSize(135, 48));

        dirGrid->addWidget(btnLeft, 1, 0, 1, 1);

        btnOk = new QPushButton(groupDirection);
        btnOk->setObjectName(QString::fromUtf8("btnOk"));
        btnOk->setMinimumSize(QSize(135, 48));
        btnOk->setMaximumSize(QSize(135, 48));

        dirGrid->addWidget(btnOk, 1, 1, 1, 1);

        btnRight = new QPushButton(groupDirection);
        btnRight->setObjectName(QString::fromUtf8("btnRight"));
        btnRight->setMinimumSize(QSize(135, 48));
        btnRight->setMaximumSize(QSize(135, 48));

        dirGrid->addWidget(btnRight, 1, 2, 1, 1);

        btnDown = new QPushButton(groupDirection);
        btnDown->setObjectName(QString::fromUtf8("btnDown"));
        btnDown->setMinimumSize(QSize(135, 48));
        btnDown->setMaximumSize(QSize(135, 48));

        dirGrid->addWidget(btnDown, 2, 1, 1, 1);


        dirLayout->addLayout(dirGrid);

        dirSpacerTop = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        dirLayout->addItem(dirSpacerTop);


        keyMainLayout->addWidget(groupDirection);

        groupFunction = new QGroupBox(PageSerialKeyControl);
        groupFunction->setObjectName(QString::fromUtf8("groupFunction"));
        funcLayout = new QVBoxLayout(groupFunction);
        funcLayout->setObjectName(QString::fromUtf8("funcLayout"));
        btnBack = new QPushButton(groupFunction);
        btnBack->setObjectName(QString::fromUtf8("btnBack"));
        btnBack->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnBack);

        btnPair = new QPushButton(groupFunction);
        btnPair->setObjectName(QString::fromUtf8("btnPair"));
        btnPair->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnPair);

        btnUpdate = new QPushButton(groupFunction);
        btnUpdate->setObjectName(QString::fromUtf8("btnUpdate"));
        btnUpdate->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnUpdate);

        btnRecord = new QPushButton(groupFunction);
        btnRecord->setObjectName(QString::fromUtf8("btnRecord"));
        btnRecord->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnRecord);

        btnForce720p = new QPushButton(groupFunction);
        btnForce720p->setObjectName(QString::fromUtf8("btnForce720p"));
        btnForce720p->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnForce720p);

        btnDebug3 = new QPushButton(groupFunction);
        btnDebug3->setObjectName(QString::fromUtf8("btnDebug3"));
        btnDebug3->setMinimumSize(QSize(0, 44));

        funcLayout->addWidget(btnDebug3);

        funcSpacerTop = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        funcLayout->addItem(funcSpacerTop);


        keyMainLayout->addWidget(groupFunction);

        groupCustomKey = new QGroupBox(PageSerialKeyControl);
        groupCustomKey->setObjectName(QString::fromUtf8("groupCustomKey"));
        ckLayout = new QVBoxLayout(groupCustomKey);
        ckLayout->setObjectName(QString::fromUtf8("ckLayout"));
        lbKeyName = new QLabel(groupCustomKey);
        lbKeyName->setObjectName(QString::fromUtf8("lbKeyName"));

        ckLayout->addWidget(lbKeyName);

        comboKey = new QComboBox(groupCustomKey);
        comboKey->setObjectName(QString::fromUtf8("comboKey"));
        comboKey->setMinimumSize(QSize(100, 28));

        ckLayout->addWidget(comboKey);

        lbPressType = new QLabel(groupCustomKey);
        lbPressType->setObjectName(QString::fromUtf8("lbPressType"));

        ckLayout->addWidget(lbPressType);

        comboPressType = new QComboBox(groupCustomKey);
        comboPressType->addItem(QString());
        comboPressType->addItem(QString());
        comboPressType->setObjectName(QString::fromUtf8("comboPressType"));
        comboPressType->setMinimumSize(QSize(100, 28));

        ckLayout->addWidget(comboPressType);

        btnSendCustomKey = new QPushButton(groupCustomKey);
        btnSendCustomKey->setObjectName(QString::fromUtf8("btnSendCustomKey"));
        btnSendCustomKey->setMinimumSize(QSize(100, 40));
        btnSendCustomKey->setMaximumSize(QSize(100, 40));
        btnSendCustomKey->setStyleSheet(QString::fromUtf8("QPushButton{background-color:#2196F3;color:white;font-weight:bold;}"));

        ckLayout->addWidget(btnSendCustomKey);

        lbPreview = new QLabel(groupCustomKey);
        lbPreview->setObjectName(QString::fromUtf8("lbPreview"));

        ckLayout->addWidget(lbPreview);

        editKeyPreview = new QLineEdit(groupCustomKey);
        editKeyPreview->setObjectName(QString::fromUtf8("editKeyPreview"));
        editKeyPreview->setMinimumSize(QSize(100, 28));
        editKeyPreview->setReadOnly(true);

        ckLayout->addWidget(editKeyPreview);

        ckSpacerTop = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);

        ckLayout->addItem(ckSpacerTop);


        keyMainLayout->addWidget(groupCustomKey);


        retranslateUi(PageSerialKeyControl);

        QMetaObject::connectSlotsByName(PageSerialKeyControl);
    } // setupUi

    void retranslateUi(QWidget *PageSerialKeyControl)
    {
        groupDirection->setTitle(QCoreApplication::translate("PageSerialKeyControl", "Direction", nullptr));
        btnUp->setText(QCoreApplication::translate("PageSerialKeyControl", "UP", nullptr));
        btnLeft->setText(QCoreApplication::translate("PageSerialKeyControl", "LEFT", nullptr));
        btnOk->setText(QCoreApplication::translate("PageSerialKeyControl", "OK", nullptr));
        btnRight->setText(QCoreApplication::translate("PageSerialKeyControl", "RIGHT", nullptr));
        btnDown->setText(QCoreApplication::translate("PageSerialKeyControl", "DOWN", nullptr));
        groupFunction->setTitle(QCoreApplication::translate("PageSerialKeyControl", "Functions", nullptr));
        btnBack->setText(QCoreApplication::translate("PageSerialKeyControl", "BACK", nullptr));
        btnPair->setText(QCoreApplication::translate("PageSerialKeyControl", "Pair", nullptr));
        btnUpdate->setText(QCoreApplication::translate("PageSerialKeyControl", "Update", nullptr));
        btnRecord->setText(QCoreApplication::translate("PageSerialKeyControl", "Record", nullptr));
        btnForce720p->setText(QCoreApplication::translate("PageSerialKeyControl", "720p60", nullptr));
        btnDebug3->setText(QCoreApplication::translate("PageSerialKeyControl", "Debug3", nullptr));
        groupCustomKey->setTitle(QCoreApplication::translate("PageSerialKeyControl", "Custom Key", nullptr));
        lbKeyName->setText(QCoreApplication::translate("PageSerialKeyControl", "Key:", nullptr));
        lbPressType->setText(QCoreApplication::translate("PageSerialKeyControl", "Press Type:", nullptr));
        comboPressType->setItemText(0, QCoreApplication::translate("PageSerialKeyControl", "Click (0)", nullptr));
        comboPressType->setItemText(1, QCoreApplication::translate("PageSerialKeyControl", "Long (1)", nullptr));

        btnSendCustomKey->setText(QCoreApplication::translate("PageSerialKeyControl", "Send Custom Key", nullptr));
        lbPreview->setText(QCoreApplication::translate("PageSerialKeyControl", "Preview:", nullptr));
        (void)PageSerialKeyControl;
    } // retranslateUi

};

namespace Ui {
    class PageSerialKeyControl: public Ui_PageSerialKeyControl {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_KEYCONTROL_H
