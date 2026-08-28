import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 6.5

Rectangle {
    id: root
    radius: 28
    color: "#050C17"
    border.color: "#20324A"
    border.width: 2

    property string gear: "P"
    property int battery: 85

    Image {
        anchors.fill: parent
        source: "qrc:/0468e38a-fdaa-4e69-990a-c3627833e7f8.png"
        fillMode: Image.PreserveAspectCrop
        opacity: 0.48
    }

    MediaDevices {
        id: mediaDevices
    }

    CameraView {
        anchors.fill: parent
        activeCamDevice: mediaDevices.defaultVideoInput
        visible: root.gear === "R"
        z: 1
    }

    Rectangle {
        anchors.fill: parent
        color: "#020817"
        opacity: 0.48
        z: 2
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"

            Column {
                anchors.centerIn: parent
                spacing: 26

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.gear === "R" ? "REVERSE CAMERA"
                         : root.gear === "D" ? "DRIVING MODE"
                         : root.gear === "N" ? "NEUTRAL" : "PARKING"
                    color: root.gear === "R" ? "#EF4444" : "#20B8FF"
                    font.pixelSize: 28
                    font.bold: true
                    font.letterSpacing: 2
                }

                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 270
                    height: 270
                    radius: 135
                    color: "#07111F"
                    border.color: root.gear === "R" ? "#EF4444" : "#1C7DD1"
                    border.width: 3

                    Rectangle {
                        anchors.centerIn: parent
                        width: 244
                        height: 244
                        radius: 122
                        color: "transparent"
                        border.color: "#20344B"
                        border.width: 2
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 2

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: root.gear
                            color: "#F8FAFC"
                            font.pixelSize: 86
                            font.bold: true
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: root.gear === "R" ? "REVERSE"
                                 : root.gear === "D" ? "DRIVE"
                                 : root.gear === "N" ? "NEUTRAL" : "PARK"
                            color: "#33B8FF"
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }
                }

                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 14

                    Text { text: "E"; color: "#B8C5D2"; font.pixelSize: 16; font.bold: true }

                    Rectangle {
                        width: 260; height: 12
                        radius: 6
                        color: "#1A2838"
                        clip: true

                        Rectangle {
                            width: parent.width * root.battery / 100
                            height: parent.height
                            radius: 6
                            color: "#21B6F5"
                        }
                    }

                    Text { text: "F"; color: "#B8C5D2"; font.pixelSize: 16; font.bold: true }
                }
            }
        }

    }
}
