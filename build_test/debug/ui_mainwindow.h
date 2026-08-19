/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QHBoxLayout *centralLayout;
    QListWidget *listSidebar;
    QStackedWidget *stackContent;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1300, 800);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        centralLayout = new QHBoxLayout(centralwidget);
        centralLayout->setSpacing(0);
        centralLayout->setObjectName(QString::fromUtf8("centralLayout"));
        centralLayout->setContentsMargins(0, 0, 0, 0);
        listSidebar = new QListWidget(centralwidget);
        new QListWidgetItem(listSidebar);
        new QListWidgetItem(listSidebar);
        new QListWidgetItem(listSidebar);
        listSidebar->setObjectName(QString::fromUtf8("listSidebar"));
        listSidebar->setMinimumSize(QSize(200, 0));
        listSidebar->setMaximumSize(QSize(200, 16777215));
        listSidebar->setStyleSheet(QString::fromUtf8("QListWidget {\n"
"    background-color: #2c3e50;\n"
"    border: none;\n"
"    outline: none;\n"
"    font-size: 11pt;\n"
"}\n"
"QListWidget::item {\n"
"    color: #ecf0f1;\n"
"    padding: 14px 20px;\n"
"    border: none;\n"
"    border-bottom: 1px solid #34495e;\n"
"}\n"
"QListWidget::item:hover {\n"
"    background-color: #34495e;\n"
"    color: #ffffff;\n"
"    padding-left: 24px;\n"
"}\n"
"QListWidget::item:selected {\n"
"    background-color: #2980b9;\n"
"    color: #ffffff;\n"
"    font-weight: bold;\n"
"    border-left: 4px solid #1abc9c;\n"
"    padding-left: 16px;\n"
"}"));

        centralLayout->addWidget(listSidebar);

        stackContent = new QStackedWidget(centralwidget);
        stackContent->setObjectName(QString::fromUtf8("stackContent"));

        centralLayout->addWidget(stackContent);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "FPV Debug Tool v1.0", nullptr));

        const bool __sortingEnabled = listSidebar->isSortingEnabled();
        listSidebar->setSortingEnabled(false);
        QListWidgetItem *___qlistwidgetitem = listSidebar->item(0);
        ___qlistwidgetitem->setText(QCoreApplication::translate("MainWindow", "Firmware", nullptr));
        QListWidgetItem *___qlistwidgetitem1 = listSidebar->item(1);
        ___qlistwidgetitem1->setText(QCoreApplication::translate("MainWindow", "Serial", nullptr));
        QListWidgetItem *___qlistwidgetitem2 = listSidebar->item(2);
        ___qlistwidgetitem2->setText(QCoreApplication::translate("MainWindow", "Settings", nullptr));
        listSidebar->setSortingEnabled(__sortingEnabled);

    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
