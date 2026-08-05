#ifndef SBUSTESTPAGE_H
#define SBUSTESTPAGE_H

#include <QWidget>
#include <QSerialPort>
#include <QTimer>
#include <QDateTime>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QQueue>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QProgressBar>

#include "sbusprotocolparser.h"
#include "crsfchannelwidget.h"

class SerialCommunicator;

namespace Ui {
class PageSerialSbusTest;
}

class SbusTestPage : public QWidget
{
    Q_OBJECT
public:
    explicit SbusTestPage(SerialCommunicator *comm, QWidget *parent = nullptr);
    ~SbusTestPage();

signals:
    void serialStatusChanged(const QString &msg);

private slots:
    void onRefreshPorts();
    void onToggleConnection();
    void onSendData();
    void onClearRecv();
    void onSaveRecv();
    void onResetStats();
    void onTplChannels();
    void onTplCustom();
    void onDataReceived(const QByteArray &data);
    void onSerialStatus(const QString &msg);
    void onStopAtChanged(int value);
    void onStopAtToggle(bool checked);

private:
    // Async log (optional)
    void initLogFile();
    void closeLogFile();
    void enqueueLogLine(const QString &line);
    void enqueueRawFrame(const QByteArray &frame);
    void flushLogBuffer();

    QFile *m_logFile = nullptr;
    QString m_logFilePath;
    QQueue<QByteArray> m_logQueue;
    QTimer *m_logFlushTimer = nullptr;
    QTimer *m_processTimer = nullptr;
    qint64 m_logBytesWritten = 0;
    bool m_logEnabled = false;

    void updateConnectionState();
    void processRxData();
    void onProcessTimer();
    void appendAnalyzerPacket(int seq, const ParsedSbusFrame &parsed, qint64 intervalMs);
    void updateChannelWidget();
    void updateIntervalDisplay();
    void initAnalyzerUI();

    QByteArray m_rxBuffer;
    Ui::PageSerialSbusTest *ui;
    QWidget *m_pageWidget;
    SerialCommunicator *m_comm;

    // stats
    qint64 m_rxTotalBytes = 0;
    int m_validFrames = 0;
    int m_invalidFrames = 0;
    int m_packetCount = 0;
    qint64 m_lastRxTime = 0;
    qint64 m_intervalSum = 0;
    qint64 m_intervalMin = 999999;
    qint64 m_intervalMax = 0;

    int m_stopAtFrameCount = 0;
    bool m_stoppedByLimit = false;

    // Analyzer UI (created in ctor)
    QTextEdit *m_analyzerText = nullptr;
    QLabel *m_lblIntervalPattern = nullptr;
    QLabel *m_lblAnalyzerPktCount = nullptr;
    QLabel *m_lblIntervalMin = nullptr;
    QLabel *m_lblIntervalMax = nullptr;
    QLabel *m_lblIntervalAvg = nullptr;
    QLabel *m_lblFlags = nullptr;
    QLabel *m_lblRxBytes = nullptr;

    CrsfChannelWidget *m_channelWidget = nullptr;
    QTableWidget *m_channelTable = nullptr;
    QGroupBox *m_groupAnalyzer = nullptr;
    QGroupBox *m_groupChannels = nullptr;
    QGroupBox *m_groupFlags = nullptr;

    uint16_t m_currentChannels[16] = {1500,1500,1500,1500,1500,1500,1500,1500,
                                      1500,1500,1500,1500,1500,1500,1500,1500};
    bool m_hasRcData = false;
};

#endif // SBUSTESTPAGE_H
