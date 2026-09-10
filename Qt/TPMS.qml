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
<<<<<<< HEAD
        anchors.leftMargin: 14
        anchors.topMargin: 10

        text: "♧  TPMS"
        color: "#B4C4D6"
=======
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
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

            color: "#8E9EAF"

            font.pixelSize: 12
            font.bold: true
            font.letterSpacing: 1.2
        }
    }


    // =====================================================
<<<<<<< HEAD
    // CAR AREA
=======
    // MAIN TPMS AREA
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
    // =====================================================

    Item {
        id: carArea

        anchors.left: parent.left
        anchors.right: parent.right
<<<<<<< HEAD
        anchors.top: title.bottom
=======
        anchors.top: headerLayout.bottom
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
        anchors.bottom: parent.bottom

        anchors.leftMargin: 8
        anchors.rightMargin: 8
        anchors.topMargin: 4
        anchors.bottomMargin: 8

<<<<<<< HEAD
        // =================================================
        // CAR IMAGE
        // =================================================

=======

        // =================================================
        // CAR IMAGE
        // =================================================

>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
        Image {
            id: carImage

            anchors.centerIn: parent

<<<<<<< HEAD
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
=======
            // Xe lớn hơn trước
            width: Math.min(parent.width * 0.30, 200)
            height: parent.height * 0.90

            source: "qrc:/tpms-cr.png"

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

            // Kích thước theo nội dung
            width: 78
            height: 48


            // ---------------------------------------------
            // STATUS BAR
            // ---------------------------------------------

            Rectangle {
                id: statusBar

                width: 3
                height: 25

                radius: 1.5

                color: statusColor

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }


            // ---------------------------------------------
            // VALUE AREA
            // ---------------------------------------------

            Column {
                id: valueColumn

                anchors.left: statusBar.right
                anchors.leftMargin: 7

                anchors.verticalCenter: parent.verticalCenter

                spacing: 0


                // -----------------------------------------
                // PRESSURE
                // -----------------------------------------

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


                // -----------------------------------------
                // TEMPERATURE
                // -----------------------------------------

                Text {
                    text: temperature

                    color: "#00E5FF"

                    font.pixelSize: 11
                    font.bold: true
                }
            }
        }

>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

        // =================================================
        // FRONT LEFT
        // =================================================

        TpmsValue {
            id: frontLeft

<<<<<<< HEAD
            width: Math.min(carArea.width * 0.27, 80)

            anchors.right: carImage.left
            anchors.rightMargin: carArea.valueGap

            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.22
=======
            anchors.right: carImage.left

            // Không sát xe quá
            anchors.rightMargin: -8

            // Vị trí phía trước
            anchors.verticalCenter: carImage.top

            anchors.verticalCenterOffset: carImage.height * 0.28
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

            pressure: "2.4"
            temperature: "28°C"

            statusColor: "#00E5FF"
        }

<<<<<<< HEAD
=======

>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
        // =================================================
        // FRONT RIGHT
        // =================================================

        TpmsValue {
            id: frontRight

<<<<<<< HEAD
            width: Math.min(carArea.width * 0.27, 80)

            anchors.left: carImage.right
            anchors.leftMargin: carArea.valueGap

            anchors.verticalCenter: carImage.top
            anchors.verticalCenterOffset: carImage.height * 0.22
=======
            anchors.left: carImage.right
            anchors.leftMargin: 5

            anchors.verticalCenter: carImage.top

            anchors.verticalCenterOffset: carImage.height * 0.28
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

            pressure: "2.5"
            temperature: "27°C"

            statusColor: "#00E5FF"
        }

<<<<<<< HEAD
=======

>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
        // =================================================
        // REAR LEFT
        // =================================================

        TpmsValue {
            id: rearLeft

<<<<<<< HEAD
            width: Math.min(carArea.width * 0.27, 80)

            anchors.right: carImage.left
            anchors.rightMargin: carArea.valueGap

            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.22
=======
            anchors.right: carImage.left
            anchors.rightMargin: -8

            anchors.verticalCenter: carImage.bottom

            anchors.verticalCenterOffset: -carImage.height * 0.28
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

            pressure: "2.4"
            temperature: "27°C"

            statusColor: "#00E5FF"
        }

<<<<<<< HEAD
=======

>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380
        // =================================================
        // REAR RIGHT
        // =================================================

        TpmsValue {
            id: rearRight

<<<<<<< HEAD
            width: Math.min(carArea.width * 0.27, 80)

            anchors.left: carImage.right
            anchors.leftMargin: carArea.valueGap

            anchors.verticalCenter: carImage.bottom
            anchors.verticalCenterOffset: -carImage.height * 0.22
=======
            anchors.left: carImage.right
            anchors.leftMargin: 5

            anchors.verticalCenter: carImage.bottom

            anchors.verticalCenterOffset: -carImage.height * 0.28
>>>>>>> e10429312576b156a936d2b9dcadbc53e3641380

            pressure: "2.5"
            temperature: "28°C"

            statusColor: "#00E5FF"
        }
    }
}
