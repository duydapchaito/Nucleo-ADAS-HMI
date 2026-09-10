import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    radius: 18
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1

    // =====================================================
    // HEADER
    // =====================================================

    Text {
        id: title

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 14
        anchors.topMargin: 10

        text: "♧  TPMS"
        color: "#B4C4D6"

        font.pixelSize: 12
        font.bold: true
        font.letterSpacing: 0.8
    }

    // =====================================================
    // CAR AREA
    // =====================================================

    Item {
        id: carArea

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.bottom: parent.bottom

        anchors.topMargin: 4
        anchors.bottomMargin: 8

        // =================================================
        // CAR IMAGE
        // =================================================

        Image {
            id: carImage

            anchors.centerIn: parent

            // Giới hạn theo cả width và height
            width: Math.min(
                       parent.width * 0.30,
                       parent.height * 0.72,
                       85
                   )

            height: Math.min(
                        parent.height * 0.78,
                        130
                    )

            source: "qrc:/tpms.png"

            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        // =================================================
        // SENSOR POSITIONS
        // =================================================

        // Khoảng cách giữa thông số và xe
        readonly property real valueGap: 8

        // Khoảng cách từ mép panel
        readonly property real sideMargin: 8

        // =================================================
        // FRONT LEFT
        // =================================================

        TpmsValue {
            id: frontLeft

            width: Math.min(carArea.width * 0.27, 80)

            anchors.right: carImage.left
            anchors.rightMargin: carArea.valueGap

            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.22

            pressure: "2.4"
            temperature: "28°C"
        }

        // =================================================
        // FRONT RIGHT
        // =================================================

        TpmsValue {
            id: frontRight

            width: Math.min(carArea.width * 0.27, 80)

            anchors.left: carImage.right
            anchors.leftMargin: carArea.valueGap

            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.22

            pressure: "2.5"
            temperature: "27°C"
        }

        // =================================================
        // REAR LEFT
        // =================================================

        TpmsValue {
            id: rearLeft

            width: Math.min(carArea.width * 0.27, 80)

            anchors.right: carImage.left
            anchors.rightMargin: carArea.valueGap

            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.22

            pressure: "2.4"
            temperature: "27°C"
        }

        // =================================================
        // REAR RIGHT
        // =================================================

        TpmsValue {
            id: rearRight

            width: Math.min(carArea.width * 0.27, 80)

            anchors.left: carImage.right
            anchors.leftMargin: carArea.valueGap

            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.22

            pressure: "2.5"
            temperature: "28°C"
        }
    }
}
