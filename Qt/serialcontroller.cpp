#include "serialcontroller.h"

#include <QDebug>


SerialController::SerialController(QObject *parent)
    : QObject(parent)
{
    connect(
        &serial,
        &QSerialPort::readyRead,
        this,
        &SerialController::readData
        );

    scanPorts();
}


// ============================================================
// SCAN PORTS
// ============================================================

void SerialController::scanPorts()
{
    QStringList ports;

    const auto infos =
        QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos)
    {
        ports.append(info.portName());
    }

    if (m_portList != ports)
    {
        m_portList = ports;

        emit portListChanged();
    }

    qDebug() << "Available COM ports:"
             << m_portList;
}


// ============================================================
// CONNECT
// ============================================================

void SerialController::connectPort(
    const QString &portName,
    int baudRate)
{
    if (serial.isOpen())
    {
        serial.close();
    }


    serial.setPortName(portName);

    serial.setBaudRate(baudRate);

    serial.setDataBits(
        QSerialPort::Data8
        );

    serial.setParity(
        QSerialPort::NoParity
        );

    serial.setStopBits(
        QSerialPort::OneStop
        );

    serial.setFlowControl(
        QSerialPort::NoFlowControl
        );


    if (serial.open(QIODevice::ReadOnly))
    {
        m_isConnected = true;

        emit isConnectedChanged();

        qDebug()
            << "Connected:"
            << portName
            << "Baud:"
            << baudRate;
    }
    else
    {
        m_isConnected = false;

        emit isConnectedChanged();

        qDebug()
            << "Failed to connect:"
            << serial.errorString();
    }
}


// ============================================================
// DISCONNECT
// ============================================================

void SerialController::disconnectPort()
{
    if (serial.isOpen())
    {
        serial.close();
    }

    m_isConnected = false;

    emit isConnectedChanged();

    qDebug() << "Serial disconnected";
}


// ============================================================
// READ DATA
// ============================================================

void SerialController::readData()
{
    m_receiveBuffer.append(
        serial.readAll()
        );


    // ========================================================
    // PROCESS COMPLETE LINES
    // ========================================================

    while (m_receiveBuffer.contains('\n'))
    {
        int index =
            m_receiveBuffer.indexOf('\n');


        QByteArray line =
            m_receiveBuffer.left(index);


        m_receiveBuffer.remove(
            0,
            index + 1
            );


        line = line.trimmed();


        if (line.isEmpty())
            continue;


        qDebug()
            << "RX:"
            << line;


        // ====================================================
        // CHECK HEADER
        // ====================================================

        if (!line.startsWith("$RADAR:"))
            continue;


        // Remove "$RADAR:"
        QByteArray data =
            line.mid(7);


        QList<QByteArray> parts =
            data.split(',');


        // ====================================================
        // EXPECT:
        //
        // GEAR
        // FL
        // FC
        // FR
        // RL
        // RC
        // RR
        //
        // = 7 fields
        // ====================================================

        if (parts.size() != 7)
        {
            qDebug()
            << "Invalid RADAR packet:"
            << parts;

            continue;
        }


        // ====================================================
        // GEAR
        // ====================================================

        QString newGear =
            QString(parts[0]).trimmed();


        if (newGear.isEmpty())
            continue;


        if (newGear != m_gear)
        {
            m_gear = newGear;

            emit gearChanged();
        }


        // ====================================================
        // SENSOR VALUES
        // ====================================================

        bool okFL = false;
        bool okFC = false;
        bool okFR = false;

        bool okRL = false;
        bool okRC = false;
        bool okRR = false;


        int fl =
            parts[1].toInt(&okFL);

        int fc =
            parts[2].toInt(&okFC);

        int fr =
            parts[3].toInt(&okFR);

        int rl =
            parts[4].toInt(&okRL);

        int rc =
            parts[5].toInt(&okRC);

        int rr =
            parts[6].toInt(&okRR);


        if (!okFL ||
            !okFC ||
            !okFR ||
            !okRL ||
            !okRC ||
            !okRR)
        {
            qDebug()
            << "Invalid sensor data:"
            << parts;

            continue;
        }


        // ====================================================
        // LIMIT ALERT LEVEL
        //
        // -1 = hidden
        //  0 = SAFE
        //  1 = WARNING
        //  2 = DANGER
        // ====================================================

        fl = qBound(0, fl, 2);
        fc = qBound(0, fc, 2);
        fr = qBound(0, fr, 2);

        rl = qBound(0, rl, 2);
        rc = qBound(0, rc, 2);
        rr = qBound(0, rr, 2);


        // ====================================================
        // FRONT LEFT
        // ====================================================

        if (fl != m_alertFrontLeft)
        {
            m_alertFrontLeft = fl;

            emit alertFrontLeftChanged();
        }


        // ====================================================
        // FRONT CENTER
        // ====================================================

        if (fc != m_alertFrontCenter)
        {
            m_alertFrontCenter = fc;

            emit alertFrontCenterChanged();
        }


        // ====================================================
        // FRONT RIGHT
        // ====================================================

        if (fr != m_alertFrontRight)
        {
            m_alertFrontRight = fr;

            emit alertFrontRightChanged();
        }


        // ====================================================
        // REAR LEFT
        // ====================================================

        if (rl != m_alertRearLeft)
        {
            m_alertRearLeft = rl;

            emit alertRearLeftChanged();
        }


        // ====================================================
        // REAR CENTER
        // ====================================================

        if (rc != m_alertRearCenter)
        {
            m_alertRearCenter = rc;

            emit alertRearCenterChanged();
        }


        // ====================================================
        // REAR RIGHT
        // ====================================================

        if (rr != m_alertRearRight)
        {
            m_alertRearRight = rr;

            emit alertRearRightChanged();
        }


        // ====================================================
        // DEBUG
        // ====================================================

        qDebug()
            << "Gear:"
            << m_gear
            << "| FL:"
            << m_alertFrontLeft
            << "| FC:"
            << m_alertFrontCenter
            << "| FR:"
            << m_alertFrontRight
            << "| RL:"
            << m_alertRearLeft
            << "| RC:"
            << m_alertRearCenter
            << "| RR:"
            << m_alertRearRight;
    }
}
