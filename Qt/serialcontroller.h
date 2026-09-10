#ifndef SERIALCONTROLLER_H
#define SERIALCONTROLLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringList>

class SerialController : public QObject
{
    Q_OBJECT

    // =====================================================
    // GEAR
    // =====================================================

    Q_PROPERTY(QString gear
                   READ gear
                       NOTIFY gearChanged)

    // =====================================================
    // FRONT 3 SENSORS
    // =====================================================

    Q_PROPERTY(int alertFrontLeft
                   READ alertFrontLeft
                       NOTIFY alertFrontLeftChanged)

    Q_PROPERTY(int alertFrontCenter
                   READ alertFrontCenter
                       NOTIFY alertFrontCenterChanged)

    Q_PROPERTY(int alertFrontRight
                   READ alertFrontRight
                       NOTIFY alertFrontRightChanged)

    // =====================================================
    // REAR 3 SENSORS
    // =====================================================

    Q_PROPERTY(int alertRearLeft
                   READ alertRearLeft
                       NOTIFY alertRearLeftChanged)

    Q_PROPERTY(int alertRearCenter
                   READ alertRearCenter
                       NOTIFY alertRearCenterChanged)

    Q_PROPERTY(int alertRearRight
                   READ alertRearRight
                       NOTIFY alertRearRightChanged)

    // =====================================================
    // SERIAL
    // =====================================================

    Q_PROPERTY(QStringList portList
                   READ portList
                       NOTIFY portListChanged)

    Q_PROPERTY(bool isConnected
                   READ isConnected
                       NOTIFY isConnectedChanged)


public:

    explicit SerialController(QObject *parent = nullptr);


    // =====================================================
    // GETTERS
    // =====================================================

    QString gear() const
    {
        return m_gear;
    }


    int alertFrontLeft() const
    {
        return m_alertFrontLeft;
    }


    int alertFrontCenter() const
    {
        return m_alertFrontCenter;
    }


    int alertFrontRight() const
    {
        return m_alertFrontRight;
    }


    int alertRearLeft() const
    {
        return m_alertRearLeft;
    }


    int alertRearCenter() const
    {
        return m_alertRearCenter;
    }


    int alertRearRight() const
    {
        return m_alertRearRight;
    }


    QStringList portList() const
    {
        return m_portList;
    }


    bool isConnected() const
    {
        return m_isConnected;
    }


    // =====================================================
    // QML FUNCTIONS
    // =====================================================

    Q_INVOKABLE void scanPorts();

    Q_INVOKABLE void connectPort(
        const QString &portName,
        int baudRate
        );

    Q_INVOKABLE void disconnectPort();


signals:

    // =====================================================
    // GEAR
    // =====================================================

    void gearChanged();


    // =====================================================
    // FRONT
    // =====================================================

    void alertFrontLeftChanged();
    void alertFrontCenterChanged();
    void alertFrontRightChanged();


    // =====================================================
    // REAR
    // =====================================================

    void alertRearLeftChanged();
    void alertRearCenterChanged();
    void alertRearRightChanged();


    // =====================================================
    // SERIAL
    // =====================================================

    void portListChanged();
    void isConnectedChanged();


private slots:

    void readData();


private:

    QSerialPort serial;


    // =====================================================
    // DATA
    // =====================================================

    QString m_gear = "R";


    // Front
    int m_alertFrontLeft   = 0;
    int m_alertFrontCenter = 0;
    int m_alertFrontRight  = 0;


    // Rear
    int m_alertRearLeft    = 0;
    int m_alertRearCenter  = 0;
    int m_alertRearRight   = 0;


    // =====================================================
    // SERIAL STATE
    // =====================================================

    QStringList m_portList;

    bool m_isConnected = false;


    // =====================================================
    // RECEIVE BUFFER
    // =====================================================

    QByteArray m_receiveBuffer;
};

#endif // SERIALCONTROLLER_H
