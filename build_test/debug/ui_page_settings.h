/********************************************************************************
** Form generated from reading UI file 'page_settings.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SETTINGS_H
#define UI_PAGE_SETTINGS_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSettings
{
public:
    QVBoxLayout *settingsLayout;
    QGroupBox *groupLanguage;
    QVBoxLayout *langLayout;
    QHBoxLayout *langSelectLayout;
    QLabel *lbLang;
    QComboBox *comboLang;
    QSpacerItem *spLang;
    QLabel *labelLangNote;
    QGroupBox *groupAbout;
    QVBoxLayout *aboutLayout;
    QLabel *labelAboutVer;
    QLabel *labelAboutDesc;
    QSpacerItem *spBottom;

    void setupUi(QWidget *PageSettings)
    {
        if (PageSettings->objectName().isEmpty())
            PageSettings->setObjectName(QString::fromUtf8("PageSettings"));
        settingsLayout = new QVBoxLayout(PageSettings);
        settingsLayout->setObjectName(QString::fromUtf8("settingsLayout"));
        groupLanguage = new QGroupBox(PageSettings);
        groupLanguage->setObjectName(QString::fromUtf8("groupLanguage"));
        langLayout = new QVBoxLayout(groupLanguage);
        langLayout->setObjectName(QString::fromUtf8("langLayout"));
        langSelectLayout = new QHBoxLayout();
        langSelectLayout->setObjectName(QString::fromUtf8("langSelectLayout"));
        lbLang = new QLabel(groupLanguage);
        lbLang->setObjectName(QString::fromUtf8("lbLang"));

        langSelectLayout->addWidget(lbLang);

        comboLang = new QComboBox(groupLanguage);
        comboLang->addItem(QString());
        comboLang->addItem(QString());
        comboLang->setObjectName(QString::fromUtf8("comboLang"));
        comboLang->setMinimumSize(QSize(200, 30));

        langSelectLayout->addWidget(comboLang);

        spLang = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        langSelectLayout->addItem(spLang);


        langLayout->addLayout(langSelectLayout);

        labelLangNote = new QLabel(groupLanguage);
        labelLangNote->setObjectName(QString::fromUtf8("labelLangNote"));
        labelLangNote->setStyleSheet(QString::fromUtf8("color: #666; font-style: italic;"));

        langLayout->addWidget(labelLangNote);


        settingsLayout->addWidget(groupLanguage);

        groupAbout = new QGroupBox(PageSettings);
        groupAbout->setObjectName(QString::fromUtf8("groupAbout"));
        aboutLayout = new QVBoxLayout(groupAbout);
        aboutLayout->setObjectName(QString::fromUtf8("aboutLayout"));
        labelAboutVer = new QLabel(groupAbout);
        labelAboutVer->setObjectName(QString::fromUtf8("labelAboutVer"));

        aboutLayout->addWidget(labelAboutVer);

        labelAboutDesc = new QLabel(groupAbout);
        labelAboutDesc->setObjectName(QString::fromUtf8("labelAboutDesc"));
        labelAboutDesc->setWordWrap(true);

        aboutLayout->addWidget(labelAboutDesc);


        settingsLayout->addWidget(groupAbout);

        spBottom = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        settingsLayout->addItem(spBottom);


        retranslateUi(PageSettings);

        QMetaObject::connectSlotsByName(PageSettings);
    } // setupUi

    void retranslateUi(QWidget *PageSettings)
    {
        groupLanguage->setTitle(QCoreApplication::translate("PageSettings", "Language / \350\257\255\350\250\200", nullptr));
        lbLang->setText(QCoreApplication::translate("PageSettings", "\350\257\255\350\250\200 / Language:", nullptr));
        comboLang->setItemText(0, QCoreApplication::translate("PageSettings", "\344\270\255\346\226\207 (zh_CN)", nullptr));
        comboLang->setItemText(1, QCoreApplication::translate("PageSettings", "English (en)", nullptr));

        labelLangNote->setText(QCoreApplication::translate("PageSettings", "Restart may be required for language change to fully apply.", nullptr));
        groupAbout->setTitle(QCoreApplication::translate("PageSettings", "About", nullptr));
        labelAboutVer->setText(QCoreApplication::translate("PageSettings", "Version: 1.0.0", nullptr));
        labelAboutDesc->setText(QCoreApplication::translate("PageSettings", "FPV Debug Tool for wireless system testing and firmware analysis.", nullptr));
        (void)PageSettings;
    } // retranslateUi

};

namespace Ui {
    class PageSettings: public Ui_PageSettings {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SETTINGS_H
