import QtQuick 2.15
import QtQuick.Layouts 1.15
import SensorUI 1.0

Rectangle {
    id: root

    radius: 18
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1

    // =====================================================
    // 1. LẤY GEAR & SENSOR DATA TỪ SERIAL CONTROLLER
    // =====================================================
    property string gear: serialController ? serialController.gear : "P"

    property real alertFrontLeft:   serialController ? serialController.alertFrontLeft : -1
    property real alertFrontCenter: serialController ? serialController.alertFrontCenter : -1
    property real alertFrontRight:  serialController ? serialController.alertFrontRight : -1

    property real alertRearLeft:    serialController ? serialController.alertRearLeft : -1
    property real alertRearCenter:  serialController ? serialController.alertRearCenter : -1
    property real alertRearRight:   serialController ? serialController.alertRearRight : -1

    // =====================================================
    // 2. MASK ẨN/HIỆN THEO CẦN SỐ (GEAR)
    // =====================================================
    property real displayFrontLeft:   (gear === "R") ? -1 : alertFrontLeft
    property real displayFrontCenter: (gear === "R") ? -1 : alertFrontCenter
    property real displayFrontRight:  (gear === "R") ? -1 : alertFrontRight

    property real displayRearLeft:    (gear === "D") ? -1 : alertRearLeft
    property real displayRearCenter:  (gear === "D") ? -1 : alertRearCenter
    property real displayRearRight:   (gear === "D") ? -1 : alertRearRight

    // =====================================================
    // 3. TÍNH KHOẢNG CÁCH NHỎ NHẤT ĐỂ HIỂN THỊ TRẠNG THÁI
    // =====================================================
    property real minimumDistance: Math.min(
        root.displayFrontLeft >= 0 ? root.displayFrontLeft : 999,
        root.displayFrontCenter >= 0 ? root.displayFrontCenter : 999,
        root.displayFrontRight >= 0 ? root.displayFrontRight : 999,
        root.displayRearLeft >= 0 ? root.displayRearLeft : 999,
        root.displayRearCenter >= 0 ? root.displayRearCenter : 999,
        root.displayRearRight >= 0 ? root.displayRearRight : 999
    )

    function statusText() {
        if (minimumDistance <= 20) return "▲ DANGER"
        if (minimumDistance <= 40) return "⚠ WARNING"
        if (minimumDistance <= 80) return "✓ SAFE"
        return "READY"
    }

    function statusColor() {
        if (minimumDistance <= 20) return "#EF4444"
        if (minimumDistance <= 40) return "#FBBF24"
        return "#22C55E"
    }

    // Header
    RowLayout {
        id: headerRow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.topMargin: 10
        height: 24
        z: 2

        Text {
            text: "PARKING ASSIST"
            color: "#B4C4D6"
            font.pixelSize: 12
            font.bold: true
            font.letterSpacing: 0.8
            Layout.fillWidth: true
        }

        Text {
            text: root.statusText()
            color: root.statusColor()
            font.pixelSize: 11
            font.bold: true
        }
    }

    // Vùng trung tâm hiển thị xe và sóng cảm biến
    Item {
        anchors.top: headerRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 10

        // Hình ảnh xe đặt ở chính giữa
        Image {
            id: carBody
            anchors.centerIn: parent
            width: 90
            height: 160
            source: "qrc:/carRED.png"
            fillMode: Image.PreserveAspectFit
            mipmap: true
            z: 2
        }

        // =========================================================
        // FRONT SENSORS (Phía trước)
        // =========================================================
        SensorSurCar {
            width: 60; height: 65
            anchors.bottom: carBody.top
            anchors.bottomMargin: -40
            anchors.horizontalCenter: carBody.horizontalCenter
            direction: 0 // FRONT CENTER
            alertLevel: root.displayFrontCenter // Sửa distance -> alertLevel
        }

        SensorSurCar {
            width: 60; height: 60
            anchors.bottom: carBody.top
            anchors.bottomMargin: -45
            anchors.right: carBody.left
            anchors.rightMargin: -40
            direction: 4 // FRONT LEFT
            alertLevel: root.displayFrontLeft
        }

        SensorSurCar {
            width: 60; height: 60
            anchors.bottom: carBody.top
            anchors.bottomMargin: -45
            anchors.left: carBody.right
            anchors.leftMargin: -40
            direction: 5 // FRONT RIGHT
            alertLevel: root.displayFrontRight
        }

        // =========================================================
        // REAR SENSORS (Phía sau)
        // =========================================================
        SensorSurCar {
            width: 60; height: 65
            anchors.top: carBody.bottom
            anchors.topMargin: -40
            anchors.horizontalCenter: carBody.horizontalCenter
            direction: 1 // REAR CENTER
            alertLevel: root.displayRearCenter
        }

        SensorSurCar {
            width: 60; height: 60
            anchors.top: carBody.bottom
            anchors.topMargin: -45
            anchors.right: carBody.left
            anchors.rightMargin: -40
            direction: 6 // REAR LEFT
            alertLevel: root.displayRearLeft
        }

        SensorSurCar {
            width: 60; height: 60
            anchors.top: carBody.bottom
            anchors.topMargin: -45
            anchors.left: carBody.right
            anchors.leftMargin: -40
            direction: 7 // REAR RIGHT
            alertLevel: root.displayRearRight
        }
    }
}
