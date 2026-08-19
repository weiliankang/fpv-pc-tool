#ifndef CRSFTESTPAGE_H
#define CRSFTESTPAGE_H

#include <QWidget>
#include <QSerialPort>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QQueue>
#include <QTableWidget>
#include <QLabel>
#include <QGroupBox>
#include <QSplitter>
#include <QScrollBar>
#include <QProgressBar>
#include <QHeaderView>
#include <QTextBlock>
#include <QTextDocument>

#include "crsfprotocolparser.h"
#include "crsfchannelwidget.h"

class SerialCommunicator;

namespace Ui {
class PageSerialCrsfTest;
}

class CrsfTestPage : public QWidget
{
    Q_OBJECT
public:
    explicit CrsfTestPage(SerialCommunicator *comm, QWidget *parent = nullptr);
    ~CrsfTestPage();

signals:
    void serialStatusChanged(const QString &msg);

private slots:
    void onRefreshPorts();
    void onToggleConnection();
    void onSendData();
    void onClearRecv();
    void onSaveRecv();
    void onResetStats();
    void onTplRcChannels();
    void onTplLinkStats();
    void onTplHeartbeat();
    void onTplCustom();
    void onDataReceived(const QByteArray &data);
    void onRxFlushTimeout();
    void onSerialStatus(const QString &msg);
    void onLogFlushTimeout();
    void onStopAtChanged(int value);
    void onStopAtToggle(bool checked);

private:
    // ============================================================
    // 异步日志记录（将所有CRSF数据写入文件）
    // ============================================================
    void initLogFile();
    void closeLogFile();
    void enqueueLogLine(const QString &line);
    void enqueueRawFrame(const QByteArray &frame);
    void flushLogBuffer();

    QFile *m_logFile = nullptr;
    QString m_logFilePath;
    QQueue<QByteArray> m_logQueue;       // 待写入队列（原始二进制数据）
    QTimer *m_logFlushTimer = nullptr;    // 定时flusher
    qint64 m_logBytesWritten = 0;
    bool m_logEnabled = false;

    void updateConnectionState();
    void appendRecvData(const QByteArray &data, const QString &direction, int matchSeq = -1, qint64 matchLatency = -1);
    void computeLatency(const QByteArray &responseData);
    int binForLatency(qint64 latency) const;
    void processRxData();

    QByteArray m_rxBuffer;

    Ui::PageSerialCrsfTest *ui;
    QWidget *m_pageWidget;
    SerialCommunicator *m_comm;

    struct SendRecord { qint64 timestamp; QByteArray data; };
    int m_sendSeq = 0;
    QMap<int, SendRecord> m_sendRecords;
    static constexpr int MAX_SEND_RECORDS = 100;

    QVector<qint64> m_latencySamples;
    qint64 m_minLatency = 0;
    qint64 m_maxLatency = 0;
    qint64 m_totalLatency = 0;
    int m_packetCount = 0;

    QVector<int> m_distributionBins;
    QVector<int> m_distCount;
    void updateDistDisplay();
    void updateIntervalDisplay();
    QString buildLatencyDistStr() const;
    QString buildIntervalDistStr() const;

    qint64 m_lastRxTime = 0;
    QVector<qint64> m_rxIntervalSamples;
    QVector<int> m_rxIntervalBins;
    QVector<int> m_rxIntervalCount;

    int m_lastSendLineSeq = -1;
    QTimer *m_rxFlushTimer;

    // 数据统计
    qint64 m_rxTotalBytes = 0;       // 从串口收到的总字节数
    qint64 m_crcPassBytes = 0;       // CRC 校验通过帧的总字节数
    int m_crcPassFrames = 0;         // CRC 校验通过的帧数
    int m_crcFailFrames = 0;         // CRC 校验失败的帧数

    // 接收帧数停止
    int m_stopAtFrameCount = 0;
    bool m_stoppedByLimit = false;

    // 大延迟诊断
    QVector<QString> m_diagRing;
    QString m_lastDiagReport;
    void addDiagEvent(const QString &event);
    void diagnoseLargeLatency(int matchedSeq, qint64 latency, const QByteArray &frame);
    void diagnoseLargeInterval(qint64 interval, const QByteArray &frame, int bufSize);

    // 诊断统计分析
    int m_diagBufAccumCount = 0;     // buf>30 的持续次数
    int m_diagIntervalAnomaly = 0;   // 间隔异常但 buf 小的连续次数

    // 诊断窗口
    void openDiagDialog();
    void addDiagLine(const QString &line);
    void showDiagStats();

    QDialog *m_diagDialog;
    QTextEdit *m_diagText;
    QPushButton *m_btnClearDiag;

    // ============================================================
    // CRSF Protocol Analyzer
    // ============================================================
private slots:
    void onClearAnalyzer();
    void onResetIntervalStats();

private:
    void initAnalyzerUI();
    void appendAnalyzerPacket(int seq, const QByteArray &rawFrame, qint64 intervalMs);
    void updateChannelWidget();
    void updateLinkStatsUI();

    // Parser
    CrsfProtocolParser *m_parser = nullptr;

    // Analyzer widgets (created in initAnalyzerUI)
    QTextEdit *m_analyzerText = nullptr;
    QLabel *m_lblIntervalPattern = nullptr;
    QLabel *m_lblIntervalMin = nullptr;
    QLabel *m_lblIntervalMax = nullptr;
    QLabel *m_lblIntervalAvg = nullptr;
    QLabel *m_lblAnalyzerPktCount = nullptr;

    // RC Channel widgets
    CrsfChannelWidget *m_channelWidget = nullptr;
    QTableWidget *m_channelTable = nullptr;

    // Link Statistics widgets
    QLabel *m_lblLinkRSSI1 = nullptr;
    QLabel *m_lblLinkRSSI2 = nullptr;
    QLabel *m_lblLinkLQ = nullptr;
    QLabel *m_lblLinkSNR = nullptr;
    QLabel *m_lblLinkRFMode = nullptr;
    QLabel *m_lblLinkTxPower = nullptr;
    QLabel *m_lblLinkDLRSSI = nullptr;
    QLabel *m_lblLinkDLLQ = nullptr;
    QLabel *m_lblLinkDLSNR = nullptr;
    QProgressBar *m_barRSSI1 = nullptr;
    QProgressBar *m_barRSSI2 = nullptr;
    QProgressBar *m_barLQ = nullptr;

    // Analyzer state
    int m_analyzerPacketCount = 0;
    qint64 m_analyzerLastRxTime = 0;

    // Group boxes for new UI panels
    QGroupBox *m_groupAnalyzer = nullptr;
    QGroupBox *m_groupChannels = nullptr;
    QGroupBox *m_groupLinkStats = nullptr;

    // Current channel values for waveform
    uint16_t m_currentChannels[16] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500,
                                      1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};
    bool m_hasRcData = false;

    // Current link statistics
    CrsfLinkStatistics m_currentLinkStats;
    bool m_hasLinkData = false;
};

#endif // CRSFTESTPAGE_H
