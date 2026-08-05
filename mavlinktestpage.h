#ifndef MAVLINKTESTPAGE_H
#define MAVLINKTESTPAGE_H

#include <QWidget>
#include <QSerialPort>
#include <QTimer>
#include <QDateTime>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QQueue>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>
#include <QListWidget>

#include "mavlinkprotocolparser.h"

class SerialCommunicator;

namespace Ui {
class PageSerialMavlinkTest;
}

class MavlinkTestPage : public QWidget
{
    Q_OBJECT
public:
    explicit MavlinkTestPage(SerialCommunicator *comm, QWidget *parent = nullptr);
    ~MavlinkTestPage();

signals:
    void serialStatusChanged(const QString &msg);

private slots:
    void onRefreshPorts();
    void onToggleConnection();
    void onSendData();
    void onClearRecv();
    void onSaveRecv();
    void onResetStats();
    void onTplHeartbeat();
    void onTplCustom();
    void onDataReceived(const QByteArray &data);
    void onSerialStatus(const QString &msg);
    void onClearAnalyzer();
    void onStopAtChanged(int value);
    void onStopAtToggle(bool checked);

private:
    void initLogFile();
    void closeLogFile();
    void enqueueRawFrame(const QByteArray &frame);
    void flushLogBuffer();

    QFile *m_logFile = nullptr;
    QString m_logFilePath;
    QQueue<QByteArray> m_logQueue;
    QTimer *m_logFlushTimer = nullptr;
    qint64 m_logBytesWritten = 0;
    bool m_logEnabled = false;

    void updateConnectionState();
    void processRxData();
    void appendAnalyzerPacket(int seq, const ParsedMavlinkFrame &parsed, qint64 intervalMs);
    void updateMessageCounts(uint32_t msgid);
    void initAnalyzerUI();

    QByteArray m_rxBuffer;
    Ui::PageSerialMavlinkTest *ui;
    QWidget *m_pageWidget;
    SerialCommunicator *m_comm;

    qint64 m_rxTotalBytes = 0;
    int m_validFrames = 0;
    int m_crcOkFrames = 0;
    int m_crcBadFrames = 0;
    int m_packetCount = 0;
    qint64 m_lastRxTime = 0;
    qint64 m_intervalSum = 0;
    qint64 m_intervalMin = 999999;
    qint64 m_intervalMax = 0;

    int m_stopAtFrameCount = 0;
    bool m_stoppedByLimit = false;

    // Analyzer UI
    QTextEdit *m_analyzerText = nullptr;
    QLabel *m_lblAnalyzerPktCount = nullptr;
    QLabel *m_lblIntervalAvg = nullptr;
    QLabel *m_lblCrcStats = nullptr;
    QTableWidget *m_msgTable = nullptr;
    QGroupBox *m_groupAnalyzer = nullptr;
    QGroupBox *m_groupMsgs = nullptr;

    // Message type counters
    QMap<uint32_t, int> m_msgCounts;
};

#endif // MAVLINKTESTPAGE_H
