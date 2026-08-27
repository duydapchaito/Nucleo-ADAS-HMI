#include "SensorSurCar.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

SensorSurCar::SensorSurCar(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAntialiasing(true);
    setAcceptedMouseButtons(Qt::NoButton);
}

int SensorSurCar::direction() const { return m_direction; }
void SensorSurCar::setDirection(int direction)
{
    if (m_direction == direction) return;
    m_direction = direction;
    emit directionChanged();
    update();
}

int SensorSurCar::alertLevel() const { return m_alertLevel; }
void SensorSurCar::setAlertLevel(int level)
{
    level = qBound(-1, level, 4);
    if (m_alertLevel == level) return;
    m_alertLevel = level;
    emit alertLevelChanged();
    update();
}

qreal SensorSurCar::distance() const { return m_distance; }
void SensorSurCar::setDistance(qreal distance)
{
    if (qFuzzyCompare(m_distance, distance)) return;
    m_distance = distance;
    emit distanceChanged();
    updateLevelFromDistance();
}

void SensorSurCar::updateLevelFromDistance()
{
    if (m_distance < 0) {
        setAlertLevel(-1);
    } else if (m_distance <= 20) {
        setAlertLevel(4); // Đủ 4 vạch (Đỏ xa nhất)
    } else if (m_distance <= 40) {
        setAlertLevel(3); // 3 vạch (Cam)
    } else if (m_distance <= 60) {
        setAlertLevel(2); // 2 vạch (Vàng)
    } else if (m_distance <= 80) {
        setAlertLevel(1); // 1 vạch (Xanh)
    } else {
        setAlertLevel(0); // Không có vạch
    }
}

void SensorSurCar::paint(QPainter *painter)
{
    if (!painter || m_alertLevel <= 0)
        return;

    painter->setRenderHint(QPainter::Antialiasing, true);

    const qreal w = width();
    const qreal h = height();
    if (w <= 0 || h <= 0) return;

    // Xác định tâm xoay sóng dựa theo hướng cảm biến
    QPointF center;
    double centerAngleDegree = 0.0;
    double totalSpanDegree = 60.0; // Độ xòe của quạt

    switch (m_direction)
    {
    case 0: // FRONT CENTER
        center = QPointF(w / 2.0, h * 0.85);
        centerAngleDegree = 90.0; // Hướng lên trên
        totalSpanDegree = 65.0;
        break;

    case 1: // REAR CENTER
        center = QPointF(w / 2.0, h * 0.15);
        centerAngleDegree = 270.0; // Hướng xuống dưới
        totalSpanDegree = 65.0;
        break;

    case 4: // FRONT LEFT
        center = QPointF(w * 0.8, h * 0.85);
        centerAngleDegree = 135.0; // Hướng chéo góc trên-trái
        totalSpanDegree = 55.0;
        break;

    case 5: // FRONT RIGHT
        center = QPointF(w * 0.2, h * 0.85);
        centerAngleDegree = 45.0; // Hướng chéo góc trên-phải
        totalSpanDegree = 55.0;
        break;

    case 6: // REAR LEFT
        center = QPointF(w * 0.8, h * 0.15);
        centerAngleDegree = 225.0; // Hướng chéo góc dưới-trái
        totalSpanDegree = 55.0;
        break;

    case 7: // REAR RIGHT
        center = QPointF(w * 0.2, h * 0.15);
        centerAngleDegree = 315.0; // Hướng chéo góc dưới-phải
        totalSpanDegree = 55.0;
        break;

    default:
        return;
    }

    // Palette màu 4 cấp độ đúng từ trong ra ngoài
    const QList<QColor> arcColors = {
        QColor("#22C55E"), // Vạch 1: Green (Gần xe nhất)
        QColor("#EAB308"), // Vạch 2: Yellow
        QColor("#F97316"), // Vạch 3: Orange
        QColor("#EF4444")  // Vạch 4: Red (Xa xe nhất)
    };

    const int maxArcs = 4;
    int arcsToDraw = qMin(m_alertLevel, maxArcs);

    const qreal baseRadius = 16.0;   // Bán kính vạch 1
    const qreal arcThickness = 6.0;  // Độ dầy vạch
    const qreal arcGap = 3.0;        // Khoảng cách giữa các vạch

    for (int i = 0; i < arcsToDraw; ++i)
    {
        qreal rInner = baseRadius + i * (arcThickness + arcGap);
        qreal rOuter = rInner + arcThickness;

        // Góc xuất phát và góc quét (Qt dùng góc tính theo độ 1/16)
        double startAngle = centerAngleDegree - (totalSpanDegree / 2.0);

        QRectF innerRect(center.x() - rInner, center.y() - rInner, rInner * 2, rInner * 2);
        QRectF outerRect(center.x() - rOuter, center.y() - rOuter, rOuter * 2, rOuter * 2);

        // Tạo hình dải quạt khép kín phẳng 2 đầu (Pie Arc Sector)
        QPainterPath path;
        path.arcMoveTo(outerRect, startAngle);
        path.arcTo(outerRect, startAngle, totalSpanDegree);
        path.arcTo(innerRect, startAngle + totalSpanDegree, -totalSpanDegree);
        path.closeSubpath();

        painter->setPen(Qt::NoPen);
        painter->setBrush(arcColors[i]);
        painter->drawPath(path);
    }
}
