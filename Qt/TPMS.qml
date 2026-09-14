import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    radius: 18
    color: "#0D141E"
    border.color: "#1E2F45"
    border.width: 1.5

    // =====================================================
    // HEADER
    // =====================================================
    RowLayout {
        id: headerLayout
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12
        spacing: 6

        Rectangle {
            width: 8
            height: 8
            radius: 4
            color: "#00E5FF"
        }

        Text {
            text: "TPMS MONITOR"
            color: "#8E9EAF"
            font.pixelSize: 12
            font.bold: true
            font.letterSpacing: 1.2
        }
    }

    // =====================================================
    // MAIN TPMS AREA
    // =====================================================
    Item {
        id: carArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerLayout.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 8

        // =================================================
        // CAR IMAGE
        // =================================================
        Image {
            id: carImage
            anchors.centerIn: parent

            // Kích thước xe
            width: Math.min(parent.width * 0.30, 200)
            height: parent.height * 0.90
            source: "qrc:/tpms.png"

            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
        }

        // =================================================
        // TPMS VALUE COMPONENT
        // =================================================
        component TpmsValue: Item {
            property string pressure: "0.0"
            property string temperature: "0°C"
            property color statusColor: "#00E5FF"

            width: 78
            height: 48

            // STATUS BAR
            Rectangle {
                id: statusBar
                width: 3
                height: 25
                radius: 1.5
                color: statusColor
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            // VALUE AREA
            Column {
                id: valueColumn
                anchors.left: statusBar.right
                anchors.leftMargin: 7
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                // PRESSURE
                Row {
                    spacing: 2
                    Text {
                        text: pressure
                        color: "#FFFFFF"
                        font.pixelSize: 14
                        font.bold: true
                        font.family: "Roboto"
                    }
                    Text {
                        text: "Bar"
                        color: "#6C7D93"
                        font.pixelSize: 10
                        font.bold: true
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                    }
                }

                // TEMPERATURE
                Text {
                    text: temperature
                    color: "#00E5FF"
                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

        // =================================================
        // SENSOR POSITIONS (Đã fix lỗi lặp anchors)
        // =================================================

        // FRONT LEFT
        TpmsValue {
            id: frontLeft
            width: Math.min(carArea.width * 0.27, 80)
            anchors.right: carImage.left
            anchors.rightMargin: -8
            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.28
            pressure: "2.4"
            temperature: "28°C"
            statusColor: "#00E5FF"
        }

        // FRONT RIGHT
        TpmsValue {
            id: frontRight
            width: Math.min(carArea.width * 0.27, 80)
            anchors.left: carImage.right
            anchors.leftMargin: 5
            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.28
            pressure: "2.5"
            temperature: "27°C"
            statusColor: "#00E5FF"
        }

        // REAR LEFT
        TpmsValue {
            id: rearLeft
            width: Math.min(carArea.width * 0.27, 80)
            anchors.right: carImage.left
            anchors.rightMargin: -8
            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.28
            pressure: "2.4"
            temperature: "27°C"
            statusColor: "#00E5FF"
        }

        // REAR RIGHT
        TpmsValue {
            id: rearRight
            width: Math.min(carArea.width * 0.27, 80)
            anchors.left: carImage.right
            anchors.leftMargin: 5
            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.28
            pressure: "2.5"
            temperature: "28°C"
            statusColor: "#00E5FF"
        }
    }
}
