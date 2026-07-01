#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QMutex>
#include <QByteArray>
#include <QTimer>

class SerialReaderThread;

class SerialCommunicator : public QObject
{
    Q_OBJECT
public:
    explicit SerialCommunicator(QObject *parent = nullptr);
    ~SerialCommunicator();

    bool connectToPort(const QString &portName, qint32 baudRate = 115200);
    void disconnect();
    bool sendData(const QByteArray &data);
    bool isConnected() const;

    // Mock 模式：当串口未连接时，可以模拟设备回复
    void setMockEnabled(bool enabled) { m_mockEnabled = enabled; }
    bool isMockEnabled() const { return m_mockEnabled; }

    // 触发模拟 OSD 回复（由 🎨 按钮调用，绕过实际串口）
    void triggerMockOsdReply();

signals:
    void dataReceived(const QByteArray &data);
    void statusChanged(const QString &message);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);
    void onMockReply();

private:
    void buildMockOsdReply();

    QSerialPort *m_serialPort = nullptr;
    bool m_isConnected = false;
    bool m_mockEnabled = false;
    QTimer *m_mockTimer = nullptr;
    QByteArray m_lastSentData;  // 最近一次发送的数据（供 mock 用）
};

#endif // SERIALCOMMUNICATOR_H
