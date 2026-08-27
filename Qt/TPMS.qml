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
    // CAR
    // =====================================================

    Item {
        id: carArea

        anchors.left: parent.left
        anchors.right: parent.right

        anchors.top: title.bottom
        anchors.bottom: parent.bottom

        anchors.topMargin: 4
        anchors.bottomMargin: 8

        // ---------------- CAR BODY ----------------

        Rectangle {
            id: car

            anchors.centerIn: parent

            width: Math.min(
                parent.width * 0.20,
                60
            )

            height: Math.min(
                parent.height * 0.78,
                115
            )

            radius: width * 0.45

            color: "#778391"

            border.color: "#AEB8C3"
            border.width: 1.5

            // Inner cabin
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right

                anchors.leftMargin: width * 0.10
                anchors.rightMargin: width * 0.10

                y: parent.height * 0.20

                height: parent.height * 0.60

                radius: width * 0.30

                color: "#101821"
            }

            // =================================================
            // TIRES
            // =================================================

            Rectangle {
                x: -width * 0.12
                y: parent.height * 0.18

                width: parent.width * 0.16
                height: parent.height * 0.20

                radius: 4

                color: "#16A765"
            }

            Rectangle {
                x: parent.width * 0.96
                y: parent.height * 0.18

                width: parent.width * 0.16
                height: parent.height * 0.20

                radius: 4

                color: "#16A765"
            }

            Rectangle {
                x: -width * 0.12
                y: parent.height * 0.62

                width: parent.width * 0.16
                height: parent.height * 0.20

                radius: 4

                color: "#16A765"
            }

            Rectangle {
                x: parent.width * 0.96
                y: parent.height * 0.62

                width: parent.width * 0.16
                height: parent.height * 0.20

                radius: 4

                color: "#16A765"
            }
        }

        // =====================================================
        // TOP LEFT
        // =====================================================

        TpmsValue {
            id: frontLeft

            anchors.right: car.left
            anchors.rightMargin: 4

            anchors.verticalCenter: car.top
            anchors.verticalCenterOffset: car.height * 0.22

            pressure: "2.4"
            temperature: "28°C"
        }

        // =====================================================
        // TOP RIGHT
        // =====================================================

        TpmsValue {
            id: frontRight

            anchors.left: car.right
            anchors.leftMargin: 4

            anchors.verticalCenter: car.top
            anchors.verticalCenterOffset: car.height * 0.22

            pressure: "2.5"
            temperature: "27°C"
        }

        // =====================================================
        // BOTTOM LEFT
        // =====================================================

        TpmsValue {
            id: rearLeft

            anchors.right: car.left
            anchors.rightMargin: 4

            anchors.verticalCenter: car.bottom
            anchors.verticalCenterOffset: -car.height * 0.12

            pressure: "2.4"
            temperature: "27°C"
        }

        // =====================================================
        // BOTTOM RIGHT
        // =====================================================

        TpmsValue {
            id: rearRight

            anchors.left: car.right
            anchors.leftMargin: 4

            anchors.verticalCenter: car.bottom
            anchors.verticalCenterOffset: -car.height * 0.22

            pressure: "2.5"
            temperature: "28°C"
        }
    }
}
