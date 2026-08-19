/********************************************************************************
** Form generated from reading UI file 'page_serial_osd.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_OSD_H
#define UI_PAGE_SERIAL_OSD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <fillplaintextedit.h>

QT_BEGIN_NAMESPACE

class Ui_PageSerialOsd
{
public:
    QVBoxLayout *osdMainLayout;
    QGroupBox *groupOsdControl;
    QVBoxLayout *osdCtrlLayout;
    QHBoxLayout *osdBtnLayout;
    QPushButton *btnGetOsdData;
    QPushButton *btnClearOsd;
    QCheckBox *checkAutoPoll;
    QSpinBox *spinPollInterval;
    QLabel *labelFcType;
    QComboBox *cboFcType;
    QSpinBox *spinCellSize;
    QLabel *labelResolution;
    QComboBox *cboOsdResolution;
    QSplitter *splitterOsd;
    QGroupBox *groupOsdGrid;
    QVBoxLayout *osdGridLayout;
    QWidget *widgetOsdCanvas;
    QGroupBox *groupOsdInfo;
    QVBoxLayout *osdInfoLayout;
    FillPlainTextEdit *textOsdInfo;

    void setupUi(QWidget *PageSerialOsd)
    {
        if (PageSerialOsd->objectName().isEmpty())
            PageSerialOsd->setObjectName(QString::fromUtf8("PageSerialOsd"));
        osdMainLayout = new QVBoxLayout(PageSerialOsd);
        osdMainLayout->setObjectName(QString::fromUtf8("osdMainLayout"));
        groupOsdControl = new QGroupBox(PageSerialOsd);
        groupOsdControl->setObjectName(QString::fromUtf8("groupOsdControl"));
        osdCtrlLayout = new QVBoxLayout(groupOsdControl);
        osdCtrlLayout->setObjectName(QString::fromUtf8("osdCtrlLayout"));
        osdBtnLayout = new QHBoxLayout();
        osdBtnLayout->setObjectName(QString::fromUtf8("osdBtnLayout"));
        btnGetOsdData = new QPushButton(groupOsdControl);
        btnGetOsdData->setObjectName(QString::fromUtf8("btnGetOsdData"));
        btnGetOsdData->setMinimumSize(QSize(0, 40));

        osdBtnLayout->addWidget(btnGetOsdData);

        btnClearOsd = new QPushButton(groupOsdControl);
        btnClearOsd->setObjectName(QString::fromUtf8("btnClearOsd"));
        btnClearOsd->setMinimumSize(QSize(0, 40));

        osdBtnLayout->addWidget(btnClearOsd);

        checkAutoPoll = new QCheckBox(groupOsdControl);
        checkAutoPoll->setObjectName(QString::fromUtf8("checkAutoPoll"));

        osdBtnLayout->addWidget(checkAutoPoll);

        spinPollInterval = new QSpinBox(groupOsdControl);
        spinPollInterval->setObjectName(QString::fromUtf8("spinPollInterval"));
        spinPollInterval->setMinimum(100);
        spinPollInterval->setMaximum(10000);
        spinPollInterval->setValue(100);
        spinPollInterval->setSingleStep(100);

        osdBtnLayout->addWidget(spinPollInterval);

        labelFcType = new QLabel(groupOsdControl);
        labelFcType->setObjectName(QString::fromUtf8("labelFcType"));

        osdBtnLayout->addWidget(labelFcType);

        cboFcType = new QComboBox(groupOsdControl);
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->addItem(QString());
        cboFcType->setObjectName(QString::fromUtf8("cboFcType"));
        cboFcType->setMinimumSize(QSize(140, 0));

        osdBtnLayout->addWidget(cboFcType);

        spinCellSize = new QSpinBox(groupOsdControl);
        spinCellSize->setObjectName(QString::fromUtf8("spinCellSize"));
        spinCellSize->setMinimum(8);
        spinCellSize->setMaximum(48);
        spinCellSize->setValue(18);

        osdBtnLayout->addWidget(spinCellSize);

        labelResolution = new QLabel(groupOsdControl);
        labelResolution->setObjectName(QString::fromUtf8("labelResolution"));

        osdBtnLayout->addWidget(labelResolution);

        cboOsdResolution = new QComboBox(groupOsdControl);
        cboOsdResolution->addItem(QString());
        cboOsdResolution->addItem(QString());
        cboOsdResolution->setObjectName(QString::fromUtf8("cboOsdResolution"));
        cboOsdResolution->setMinimumSize(QSize(80, 0));

        osdBtnLayout->addWidget(cboOsdResolution);


        osdCtrlLayout->addLayout(osdBtnLayout);


        osdMainLayout->addWidget(groupOsdControl);

        splitterOsd = new QSplitter(PageSerialOsd);
        splitterOsd->setObjectName(QString::fromUtf8("splitterOsd"));
        splitterOsd->setOrientation(Qt::Vertical);
        groupOsdGrid = new QGroupBox(splitterOsd);
        groupOsdGrid->setObjectName(QString::fromUtf8("groupOsdGrid"));
        osdGridLayout = new QVBoxLayout(groupOsdGrid);
        osdGridLayout->setObjectName(QString::fromUtf8("osdGridLayout"));
        widgetOsdCanvas = new QWidget(groupOsdGrid);
        widgetOsdCanvas->setObjectName(QString::fromUtf8("widgetOsdCanvas"));
        widgetOsdCanvas->setMinimumSize(QSize(0, 0));

        osdGridLayout->addWidget(widgetOsdCanvas);

        splitterOsd->addWidget(groupOsdGrid);
        groupOsdInfo = new QGroupBox(splitterOsd);
        groupOsdInfo->setObjectName(QString::fromUtf8("groupOsdInfo"));
        osdInfoLayout = new QVBoxLayout(groupOsdInfo);
        osdInfoLayout->setObjectName(QString::fromUtf8("osdInfoLayout"));
        textOsdInfo = new FillPlainTextEdit(groupOsdInfo);
        textOsdInfo->setObjectName(QString::fromUtf8("textOsdInfo"));
        textOsdInfo->setReadOnly(true);
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        font.setPointSize(9);
        textOsdInfo->setFont(font);

        osdInfoLayout->addWidget(textOsdInfo);

        splitterOsd->addWidget(groupOsdInfo);

        osdMainLayout->addWidget(splitterOsd);


        retranslateUi(PageSerialOsd);

        QMetaObject::connectSlotsByName(PageSerialOsd);
    } // setupUi

    void retranslateUi(QWidget *PageSerialOsd)
    {
        groupOsdControl->setTitle(QCoreApplication::translate("PageSerialOsd", "OSD \346\225\260\346\215\256\346\216\247\345\210\266", nullptr));
        btnGetOsdData->setText(QCoreApplication::translate("PageSerialOsd", "\350\216\267\345\217\226 OSD \346\225\260\346\215\256 (0x57)", nullptr));
        btnClearOsd->setText(QCoreApplication::translate("PageSerialOsd", "\346\270\205\347\251\272", nullptr));
        checkAutoPoll->setText(QCoreApplication::translate("PageSerialOsd", "\350\207\252\345\212\250\350\275\256\350\257\242", nullptr));
        spinPollInterval->setSuffix(QCoreApplication::translate("PageSerialOsd", " ms", nullptr));
        labelFcType->setText(QCoreApplication::translate("PageSerialOsd", "\351\243\236\346\216\247\347\261\273\345\236\213:", nullptr));
        cboFcType->setItemText(0, QCoreApplication::translate("PageSerialOsd", "Auto", nullptr));
        cboFcType->setItemText(1, QCoreApplication::translate("PageSerialOsd", "Betaflight", nullptr));
        cboFcType->setItemText(2, QCoreApplication::translate("PageSerialOsd", "INAV", nullptr));
        cboFcType->setItemText(3, QCoreApplication::translate("PageSerialOsd", "ArduPilot", nullptr));
        cboFcType->setItemText(4, QCoreApplication::translate("PageSerialOsd", "Fettec", nullptr));
        cboFcType->setItemText(5, QCoreApplication::translate("PageSerialOsd", "KISS", nullptr));
        cboFcType->setItemText(6, QCoreApplication::translate("PageSerialOsd", "QUIC", nullptr));
        cboFcType->setItemText(7, QCoreApplication::translate("PageSerialOsd", "Custom", nullptr));

        spinCellSize->setPrefix(QCoreApplication::translate("PageSerialOsd", "\345\255\227\347\254\246\345\244\247\345\260\217: ", nullptr));
        labelResolution->setText(QCoreApplication::translate("PageSerialOsd", "\345\210\206\350\276\250\347\216\207:", nullptr));
        cboOsdResolution->setItemText(0, QCoreApplication::translate("PageSerialOsd", "720p", nullptr));
        cboOsdResolution->setItemText(1, QCoreApplication::translate("PageSerialOsd", "1080p", nullptr));

        groupOsdGrid->setTitle(QCoreApplication::translate("PageSerialOsd", "OSD \345\217\257\350\247\206\345\214\226\350\247\206\345\233\276 (20\350\241\214 x 53\345\210\227)", nullptr));
        groupOsdInfo->setTitle(QCoreApplication::translate("PageSerialOsd", "\346\225\260\346\215\256\350\257\246\346\203\205", nullptr));
        (void)PageSerialOsd;
    } // retranslateUi

};

namespace Ui {
    class PageSerialOsd: public Ui_PageSerialOsd {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_OSD_H
