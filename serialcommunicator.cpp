#include "serialcommunicator.h"
#include <QSerialPortInfo>
#include <QDebug>

SerialCommunicator::SerialCommunicator(QObject *parent)
    : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::onErrorOccurred);
}

SerialCommunicator::~SerialCommunicator()
{
    disconnect();
}

bool SerialCommunicator::connectToPort(const QString &portName, qint32 baudRate)
{
    if (m_isConnected)
        disconnect();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_isConnected = true;
        emit statusChanged(QString("已连接到 %1").arg(portName));
        return true;
    } else {
        emit statusChanged(QString("连接失败: %1").arg(m_serialPort->errorString()));
        return false;
    }
}

void SerialCommunicator::disconnect()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    m_isConnected = false;
    emit statusChanged("已断开连接");
}

bool SerialCommunicator::sendData(const QByteArray &data)
{
    if (!m_isConnected || !m_serialPort || !m_serialPort->isOpen()) {
        emit statusChanged("串口未连接");
        return false;
    }

    qint64 written = m_serialPort->write(data);
    if (written != data.size()) {
        emit statusChanged("发送数据不完整");
        return false;
    }
    return true;
}

bool SerialCommunicator::isConnected() const
{
    return m_isConnected;
}

void SerialCommunicator::onReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialCommunicator::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit statusChanged(QString("串口错误: %1").arg(m_serialPort->errorString()));
        if (error == QSerialPort::ResourceError) {
            m_isConnected = false;
        }
    }
}
