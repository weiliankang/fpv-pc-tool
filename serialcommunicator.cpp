#include "serialcommunicator.h"
#include "serialprotocolhandler.h"
#include <QSerialPortInfo>
#include <QString>
#include <QDebug>
#include <cstring>

SerialCommunicator::SerialCommunicator(QObject *parent)
    : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialCommunicator::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialCommunicator::onErrorOccurred);

    m_mockTimer = new QTimer(this);
    m_mockTimer->setSingleShot(true);
    connect(m_mockTimer, &QTimer::timeout, this, &SerialCommunicator::onMockReply);
}

SerialCommunicator::~SerialCommunicator()
{
    disconnect();
}

bool SerialCommunicator::connectToPort(const QString &portName, qint32 baudRate)
{
    if (m_isConnected)
        disconnect();

    // 如果选择了虚拟串口（名字里包含 Mock/Simulator），自动启用 mock 模式
    if (portName.contains("Mock", Qt::CaseInsensitive) ||
        portName.contains("Simulator", Qt::CaseInsensitive) ||
        portName.contains("模拟", Qt::CaseInsensitive)) {
        m_mockEnabled = true;
        m_isConnected = true;
        emit statusChanged(QString("Mock模式已启用"));
        return true;
    }

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
    m_mockTimer->stop();
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
    m_isConnected = false;
    m_mockEnabled = false;
    emit statusChanged("已断开连接");
}

// ========== 构造模拟 OSD character_map 数据 ==========
// 生成一个模拟的 FPV OSD 字符地图
static void fillMockOsdCharMap(quint16 map[20][53])
{
    auto put = [&](int row, int colStart, const QString &text) {
        for (int i = 0; i < text.size() && colStart + i < 53; ++i) {
            map[row][colStart + i] = text[i].unicode();
        }
    };
    put(0,  0, "SAT:12 HDOP:0.8 GPS:3D");
    put(0, 23, "MODE:MANUAL");
    put(0, 37, QString("BAT:%1V %2%").arg(4.2, 0, 'f', 1).arg(85));
    put(1, 0, "=====================================================");
    put(2,  0, QString("ALT:%1m").arg(156, 4));
    put(2, 15, QString("SPD:%1km/h").arg(45, 2));
    put(2, 30, QString("DST:%1m").arg(892, 4));
    put(2, 44, QString("RSSI:%1%").arg(78));
    put(3,  0, QString("CRS:%1\xb0").arg(315, 3));
    put(3, 15, QString("VSPD:%1m/s").arg(2, 3, 10, QChar(' ')));
    put(3, 30, QString("GND:%1m").arg(145, 4));
    put(3, 44, QString("LQ:%1%").arg(92));
    map[5][26] = 0x2B;
    for (int i = 1; i <= 8; ++i) {
        map[5][26 - i] = 0x2D;
        map[5][26 + i] = 0x2D;
    }
    map[5][26 - 9] = 0x3C;
    map[5][26 + 9] = 0x3E;
    map[5][26] = 0x5E;
    put(7, 0, "----------------------------------------------");
    put(8,  0, QString("CUR:%1.%2A").arg(12).arg(3));
    put(8, 18, QString("PWR:%1W").arg(210, 3));
    put(8, 32, QString("EF:%1mAh/km").arg(382, 3));
    put(8, 47, QString("CAP:%1").arg(2840, 4));
    put(9,  0, QString("THR:%1%").arg(62));
    put(9, 15, QString("RPM:%1").arg(18500, 5));
    put(9, 30, QString("TEM:%1\xb0C").arg(58, 2));
    put(9, 44, QString("FL:%1%").arg(2));
    put(15,  0, "TIM: 05:23.7");
    put(15, 16, QString("HOME:%1m %2\xb0").arg(892).arg(180));
    put(15, 38, "ARMED");
    put(16,  0, "FLIGHT: 05:23.7");
    put(16, 18, "DIST: 892m");
    put(16, 33, "RTH ON");
    put(17,  0, "Sat:12  GPS:3D  HDOP:0.8");
    put(17, 30, "PRESS:1013hPa");
    put(17, 45, "FC: INAV 6.1");
}

// ========== 构造模拟 OSD 回复协议包 ==========
void SerialCommunicator::buildMockOsdReply()
{
    const int MAP_ROWS = 20;
    const int MAP_COLS = 53;
    const int MAP_BYTES = MAP_ROWS * MAP_COLS * 2;  // 2120
    const int DATA_CONTENT_SIZE = 1 + MAP_BYTES;     // 2121

    QByteArray osdPayload(DATA_CONTENT_SIZE, 0);
    osdPayload[0] = 0x57; // cmd_type

    quint16 (*map)[MAP_COLS] = reinterpret_cast<quint16(*)[MAP_COLS]>(osdPayload.data() + 1);
    fillMockOsdCharMap(map);

    // wire format: FE EF | cmd(1) | dataLen(2) | dataContent(2121) | checksum(2) | 0D 0A
    QByteArray replyPacket;
    replyPacket.append(static_cast<char>(0xFE));
    replyPacket.append(static_cast<char>(0xEF));
    replyPacket.append(static_cast<char>(0xA2));
    replyPacket.append(static_cast<char>((DATA_CONTENT_SIZE >> 8) & 0xFF));
    replyPacket.append(static_cast<char>(DATA_CONTENT_SIZE & 0xFF));
    replyPacket.append(osdPayload);

    quint16 checksum = SerialProtocolHandler::calculateChecksum(osdPayload);
    replyPacket.append(static_cast<char>((checksum >> 8) & 0xFF));
    replyPacket.append(static_cast<char>(checksum & 0xFF));
    replyPacket.append(static_cast<char>(0x0D));
    replyPacket.append(static_cast<char>(0x0A));

    // 打印回复协议到 qDebug
    qDebug().noquote() << "\n===== OSD Mock 回复协议包 =====";
    for (int i = 0; i < replyPacket.size(); ++i) {
        quint8 b = static_cast<quint8>(replyPacket.at(i));
        QString desc;
        if      (i == 0)                          desc = " 头(0xFE)";
        else if (i == 1)                          desc = " 头(0xEF)";
        else if (i == 2)                          desc = " 命令(0xA2=查询响应)";
        else if (i == 3)                          desc = " 数据长度(高)";
        else if (i == 4)                          desc = " 数据长度(低)";
        else if (i == 5)                          desc = " cmd_type(0x57=OSD)";
        else if (i < 5 + DATA_CONTENT_SIZE)       desc = QString(" OSD数据[%1]").arg(i - 5);
        else if (i == replyPacket.size() - 4)     desc = " 校验和(高)";
        else if (i == replyPacket.size() - 3)     desc = " 校验和(低)";
        else if (i == replyPacket.size() - 2)     desc = " 尾(0x0D)";
        else if (i == replyPacket.size() - 1)     desc = " 尾(0x0A)";
        qDebug().noquote() << QString("  [%1] 0x%2 %3")
                                  .arg(i, 3, 10, QChar('0'))
                                  .arg(b, 2, 16, QChar('0'))
                                  .arg(desc);
    }
    qDebug() << "总长度:" << replyPacket.size() << "bytes";

    emit dataReceived(replyPacket);
    emit statusChanged("Mock: OSD 模拟数据已回复");
}

void SerialCommunicator::onMockReply()
{
    qDebug() << "Mock: onMockReply called but no longer auto-triggered";
}

void SerialCommunicator::triggerMockOsdReply()
{
    qDebug() << "🎨 Mock: 触发 OSD 模拟回复";
    buildMockOsdReply();
}

bool SerialCommunicator::sendData(const QByteArray &data)
{
    m_lastSentData = data;

    if (m_mockEnabled) {
        return true;
    }

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
