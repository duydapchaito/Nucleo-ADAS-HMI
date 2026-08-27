#ifndef SENSORSURCAR_H
#define SENSORSURCAR_H

#include <QQuickPaintedItem>
#include <QColor>
#include <QList>

class SensorSurCar : public QQuickPaintedItem
{
    Q_OBJECT

    // Hướng cảm biến:
    // 0: FRONT CENTER, 1: REAR CENTER
    // 4: FRONT LEFT,   5: FRONT RIGHT
    // 6: REAR LEFT,    7: REAR RIGHT
    Q_PROPERTY(int direction READ direction WRITE setDirection NOTIFY directionChanged)

    // Mức cảnh báo từ STM32:
    // -1: Tắt cảm biến (Số N/P)
    //  0: An toàn (Không có vật cản)
    //  1: Khoảng cách Xa (Vạch 1 - Xanh)
    //  2: Khoảng cách Trung bình (Vạch 1+2 - Xanh/Vàng)
    //  3: Khoảng cách Gần (Vạch 1+2+3 - Xanh/Vàng/Cam)
    //  4: Khoảng cách Nguy hiểm (Vạch 1+2+3+4 - Xanh/Vàng/Cam/Đỏ)
    Q_PROPERTY(int alertLevel READ alertLevel WRITE setAlertLevel NOTIFY alertLevelChanged)

    // Hỗ trợ truyền khoảng cách thực tế từ STM32 (cm) nếu không dùng alertLevel
    Q_PROPERTY(qreal distance READ distance WRITE setDistance NOTIFY distanceChanged)

public:
    explicit SensorSurCar(QQuickItem *parent = nullptr);

    void paint(QPainter *painter) override;

    int direction() const;
    void setDirection(int direction);

    int alertLevel() const;
    void setAlertLevel(int level);

    qreal distance() const;
    void setDistance(qreal distance);

signals:
    void directionChanged();
    void alertLevelChanged();
    void distanceChanged();

private:
    int m_direction = 0;
    int m_alertLevel = -1;
    qreal m_distance = -1.0;

    // Chuyển khoảng cách (cm) sang alertLevel tương ứng
    void updateLevelFromDistance();
};

#endif // SENSORSURCAR_H
