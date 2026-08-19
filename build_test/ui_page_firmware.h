/********************************************************************************
** Form generated from reading UI file 'page_firmware.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_FIRMWARE_H
#define UI_PAGE_FIRMWARE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageFirmware
{
public:
    QVBoxLayout *fwMainLayout;
    QGroupBox *groupFile;
    QGridLayout *gridFile;
    QLabel *lbSky;
    QLineEdit *editSkyPath;
    QPushButton *btnSkyBrowse;
    QPushButton *btnSkyParse;
    QLabel *lbGround;
    QLineEdit *editGroundPath;
    QPushButton *btnGroundBrowse;
    QPushButton *btnGroundParse;
    QGroupBox *groupResult;
    QVBoxLayout *resultLayout;
    QTextEdit *textResult;

    void setupUi(QWidget *PageFirmware)
    {
        if (PageFirmware->objectName().isEmpty())
            PageFirmware->setObjectName(QString::fromUtf8("PageFirmware"));
        PageFirmware->resize(1035, 647);
        fwMainLayout = new QVBoxLayout(PageFirmware);
        fwMainLayout->setObjectName(QString::fromUtf8("fwMainLayout"));
        groupFile = new QGroupBox(PageFirmware);
        groupFile->setObjectName(QString::fromUtf8("groupFile"));
        gridFile = new QGridLayout(groupFile);
        gridFile->setObjectName(QString::fromUtf8("gridFile"));
        lbSky = new QLabel(groupFile);
        lbSky->setObjectName(QString::fromUtf8("lbSky"));

        gridFile->addWidget(lbSky, 0, 0, 1, 1);

        editSkyPath = new QLineEdit(groupFile);
        editSkyPath->setObjectName(QString::fromUtf8("editSkyPath"));

        gridFile->addWidget(editSkyPath, 0, 1, 1, 1);

        btnSkyBrowse = new QPushButton(groupFile);
        btnSkyBrowse->setObjectName(QString::fromUtf8("btnSkyBrowse"));

        gridFile->addWidget(btnSkyBrowse, 0, 2, 1, 1);

        btnSkyParse = new QPushButton(groupFile);
        btnSkyParse->setObjectName(QString::fromUtf8("btnSkyParse"));

        gridFile->addWidget(btnSkyParse, 0, 3, 1, 1);

        lbGround = new QLabel(groupFile);
        lbGround->setObjectName(QString::fromUtf8("lbGround"));

        gridFile->addWidget(lbGround, 1, 0, 1, 1);

        editGroundPath = new QLineEdit(groupFile);
        editGroundPath->setObjectName(QString::fromUtf8("editGroundPath"));

        gridFile->addWidget(editGroundPath, 1, 1, 1, 1);

        btnGroundBrowse = new QPushButton(groupFile);
        btnGroundBrowse->setObjectName(QString::fromUtf8("btnGroundBrowse"));

        gridFile->addWidget(btnGroundBrowse, 1, 2, 1, 1);

        btnGroundParse = new QPushButton(groupFile);
        btnGroundParse->setObjectName(QString::fromUtf8("btnGroundParse"));

        gridFile->addWidget(btnGroundParse, 1, 3, 1, 1);


        fwMainLayout->addWidget(groupFile);

        groupResult = new QGroupBox(PageFirmware);
        groupResult->setObjectName(QString::fromUtf8("groupResult"));
        resultLayout = new QVBoxLayout(groupResult);
        resultLayout->setObjectName(QString::fromUtf8("resultLayout"));
        textResult = new QTextEdit(groupResult);
        textResult->setObjectName(QString::fromUtf8("textResult"));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(10);
        textResult->setFont(font);
        textResult->setReadOnly(true);

        resultLayout->addWidget(textResult);


        fwMainLayout->addWidget(groupResult);


        retranslateUi(PageFirmware);

        QMetaObject::connectSlotsByName(PageFirmware);
    } // setupUi

    void retranslateUi(QWidget *PageFirmware)
    {
        groupFile->setTitle(QCoreApplication::translate("PageFirmware", "Firmware Files", nullptr));
        lbSky->setText(QCoreApplication::translate("PageFirmware", "\345\244\251\347\251\272/\344\270\255\347\273\247\345\233\272\344\273\266:", nullptr));
        btnSkyBrowse->setText(QCoreApplication::translate("PageFirmware", "\346\265\217\350\247\210", nullptr));
        btnSkyParse->setText(QCoreApplication::translate("PageFirmware", "\350\247\243\346\236\220", nullptr));
        lbGround->setText(QCoreApplication::translate("PageFirmware", "\345\234\260\351\235\242\347\253\257\345\233\272\344\273\266:", nullptr));
        btnGroundBrowse->setText(QCoreApplication::translate("PageFirmware", "\346\265\217\350\247\210", nullptr));
        btnGroundParse->setText(QCoreApplication::translate("PageFirmware", "\350\247\243\346\236\220", nullptr));
        groupResult->setTitle(QCoreApplication::translate("PageFirmware", "Result", nullptr));
        (void)PageFirmware;
    } // retranslateUi

};

namespace Ui {
    class PageFirmware: public Ui_PageFirmware {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_FIRMWARE_H
