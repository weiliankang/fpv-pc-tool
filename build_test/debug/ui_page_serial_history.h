/********************************************************************************
** Form generated from reading UI file 'page_serial_history.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PAGE_SERIAL_HISTORY_H
#define UI_PAGE_SERIAL_HISTORY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PageSerialHistory
{
public:
    QVBoxLayout *histMainLayout;
    QGroupBox *groupSend;
    QVBoxLayout *sendHistLayout;
    QHBoxLayout *sendBtnLayout;
    QPushButton *btnClearSend;
    QPushButton *btnExportSend;
    QSpacerItem *spSend;
    QTextEdit *textSendHistory;
    QGroupBox *groupRecv;
    QVBoxLayout *recvHistLayout;
    QHBoxLayout *recvHistBtn;
    QPushButton *btnClearRecv;
    QPushButton *btnExportRecv;
    QSpacerItem *spRecvHist;
    QTextEdit *textRecvHistory;

    void setupUi(QWidget *PageSerialHistory)
    {
        if (PageSerialHistory->objectName().isEmpty())
            PageSerialHistory->setObjectName(QString::fromUtf8("PageSerialHistory"));
        histMainLayout = new QVBoxLayout(PageSerialHistory);
        histMainLayout->setObjectName(QString::fromUtf8("histMainLayout"));
        groupSend = new QGroupBox(PageSerialHistory);
        groupSend->setObjectName(QString::fromUtf8("groupSend"));
        sendHistLayout = new QVBoxLayout(groupSend);
        sendHistLayout->setObjectName(QString::fromUtf8("sendHistLayout"));
        sendBtnLayout = new QHBoxLayout();
        sendBtnLayout->setObjectName(QString::fromUtf8("sendBtnLayout"));
        btnClearSend = new QPushButton(groupSend);
        btnClearSend->setObjectName(QString::fromUtf8("btnClearSend"));

        sendBtnLayout->addWidget(btnClearSend);

        btnExportSend = new QPushButton(groupSend);
        btnExportSend->setObjectName(QString::fromUtf8("btnExportSend"));

        sendBtnLayout->addWidget(btnExportSend);

        spSend = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        sendBtnLayout->addItem(spSend);


        sendHistLayout->addLayout(sendBtnLayout);

        textSendHistory = new QTextEdit(groupSend);
        textSendHistory->setObjectName(QString::fromUtf8("textSendHistory"));
        textSendHistory->setReadOnly(true);
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        font.setPointSize(9);
        textSendHistory->setFont(font);

        sendHistLayout->addWidget(textSendHistory);


        histMainLayout->addWidget(groupSend);

        groupRecv = new QGroupBox(PageSerialHistory);
        groupRecv->setObjectName(QString::fromUtf8("groupRecv"));
        recvHistLayout = new QVBoxLayout(groupRecv);
        recvHistLayout->setObjectName(QString::fromUtf8("recvHistLayout"));
        recvHistBtn = new QHBoxLayout();
        recvHistBtn->setObjectName(QString::fromUtf8("recvHistBtn"));
        btnClearRecv = new QPushButton(groupRecv);
        btnClearRecv->setObjectName(QString::fromUtf8("btnClearRecv"));

        recvHistBtn->addWidget(btnClearRecv);

        btnExportRecv = new QPushButton(groupRecv);
        btnExportRecv->setObjectName(QString::fromUtf8("btnExportRecv"));

        recvHistBtn->addWidget(btnExportRecv);

        spRecvHist = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        recvHistBtn->addItem(spRecvHist);


        recvHistLayout->addLayout(recvHistBtn);

        textRecvHistory = new QTextEdit(groupRecv);
        textRecvHistory->setObjectName(QString::fromUtf8("textRecvHistory"));
        textRecvHistory->setReadOnly(true);
        textRecvHistory->setFont(font);

        recvHistLayout->addWidget(textRecvHistory);


        histMainLayout->addWidget(groupRecv);


        retranslateUi(PageSerialHistory);

        QMetaObject::connectSlotsByName(PageSerialHistory);
    } // setupUi

    void retranslateUi(QWidget *PageSerialHistory)
    {
        groupSend->setTitle(QCoreApplication::translate("PageSerialHistory", "Send History", nullptr));
        btnClearSend->setText(QCoreApplication::translate("PageSerialHistory", "Clear", nullptr));
        btnExportSend->setText(QCoreApplication::translate("PageSerialHistory", "Export", nullptr));
        groupRecv->setTitle(QCoreApplication::translate("PageSerialHistory", "Receive History", nullptr));
        btnClearRecv->setText(QCoreApplication::translate("PageSerialHistory", "Clear", nullptr));
        btnExportRecv->setText(QCoreApplication::translate("PageSerialHistory", "Export", nullptr));
        (void)PageSerialHistory;
    } // retranslateUi

};

namespace Ui {
    class PageSerialHistory: public Ui_PageSerialHistory {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PAGE_SERIAL_HISTORY_H
