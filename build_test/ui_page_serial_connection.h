/********************************************************************************
** Form generated from reading UI file 'page_serial_connection.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_CONNECTION_H
#define UI_PAGE_SERIAL_CONNECTION_H

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
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSerialConnection
{
public:
    QVBoxLayout *connMainLayout;
    QGroupBox *groupSetup;
    QGridLayout *gridSetup;
    QLabel *lbPort;
    QComboBox *comboPort;
    QPushButton *btnRefresh;
    QLabel *lbBaud;
    QComboBox *comboBaud;
    QPushButton *btnConnect;
    QLabel *labelStatus;
    QGroupBox *groupRecv;
    QVBoxLayout *recvLayout;
    QTextEdit *textRecv;
    QHBoxLayout *recvBtnLayout;
    QPushButton *btnClearRecv;
    QSpacerItem *spRecv;
    QCheckBox *checkAutoScroll;

    void setupUi(QWidget *PageSerialConnection)
    {
        if (PageSerialConnection->objectName().isEmpty())
            PageSerialConnection->setObjectName(QString::fromUtf8("PageSerialConnection"));
        connMainLayout = new QVBoxLayout(PageSerialConnection);
        connMainLayout->setObjectName(QString::fromUtf8("connMainLayout"));
        groupSetup = new QGroupBox(PageSerialConnection);
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

        gridSetup->addWidget(labelStatus, 2, 1, 1, 2);


        connMainLayout->addWidget(groupSetup);

        groupRecv = new QGroupBox(PageSerialConnection);
        groupRecv->setObjectName(QString::fromUtf8("groupRecv"));
        recvLayout = new QVBoxLayout(groupRecv);
        recvLayout->setObjectName(QString::fromUtf8("recvLayout"));
        textRecv = new QTextEdit(groupRecv);
        textRecv->setObjectName(QString::fromUtf8("textRecv"));
        textRecv->setReadOnly(true);
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(10);
        textRecv->setFont(font);

        recvLayout->addWidget(textRecv);

        recvBtnLayout = new QHBoxLayout();
        recvBtnLayout->setObjectName(QString::fromUtf8("recvBtnLayout"));
        btnClearRecv = new QPushButton(groupRecv);
        btnClearRecv->setObjectName(QString::fromUtf8("btnClearRecv"));

        recvBtnLayout->addWidget(btnClearRecv);

        spRecv = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        recvBtnLayout->addItem(spRecv);

        checkAutoScroll = new QCheckBox(groupRecv);
        checkAutoScroll->setObjectName(QString::fromUtf8("checkAutoScroll"));
        checkAutoScroll->setChecked(true);

        recvBtnLayout->addWidget(checkAutoScroll);


        recvLayout->addLayout(recvBtnLayout);


        connMainLayout->addWidget(groupRecv);


        retranslateUi(PageSerialConnection);

        QMetaObject::connectSlotsByName(PageSerialConnection);
    } // setupUi

    void retranslateUi(QWidget *PageSerialConnection)
    {
        groupSetup->setTitle(QCoreApplication::translate("PageSerialConnection", "Serial Port Setup", nullptr));
        lbPort->setText(QCoreApplication::translate("PageSerialConnection", "Port:", nullptr));
        btnRefresh->setText(QCoreApplication::translate("PageSerialConnection", "Refresh", nullptr));
        lbBaud->setText(QCoreApplication::translate("PageSerialConnection", "Baud:", nullptr));
        btnConnect->setText(QCoreApplication::translate("PageSerialConnection", "Connect", nullptr));
        labelStatus->setText(QCoreApplication::translate("PageSerialConnection", "Disconnected", nullptr));
        groupRecv->setTitle(QCoreApplication::translate("PageSerialConnection", "Live Data", nullptr));
        btnClearRecv->setText(QCoreApplication::translate("PageSerialConnection", "Clear", nullptr));
        checkAutoScroll->setText(QCoreApplication::translate("PageSerialConnection", "Auto Scroll", nullptr));
        (void)PageSerialConnection;
    } // retranslateUi

};

namespace Ui {
    class PageSerialConnection: public Ui_PageSerialConnection {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_CONNECTION_H
