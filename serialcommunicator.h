#ifndef SERIALCOMMUNICATOR_H
#define SERIALCOMMUNICATOR_H

#include <QObject>
#include <QSerialPort>
#include <QThread>
#include <QMutex>
#include <QByteArray>

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

signals:
    void dataReceived(const QByteArray &data);
    void statusChanged(const QString &message);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort = nullptr;
    bool m_isConnected = false;
};

#endif // SERIALCOMMUNICATOR_H
