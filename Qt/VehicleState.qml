    import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    radius: 18
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1

    property string gear: "P"
    property int battery: 85
    property int rangeKm: 412

    // =====================================================
    // HEADER
    // =====================================================

    Text {
        id: title

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12

        text: "VEHICLE STATE"
        color: "#A8B9CD"

        font.pixelSize: 13
        font.bold: true
        font.letterSpacing: 1.0
    }

    // =====================================================
    // MAIN CONTENT
    // =====================================================

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.bottom: parent.bottom

        anchors.leftMargin: 14
        anchors.rightMargin: 14
        anchors.topMargin: 8
        anchors.bottomMargin: 12

        spacing: 14

        // =================================================
        // GEAR DISPLAY
        // =================================================

        Item {
            Layout.preferredWidth: Math.min(
                parent.height,
                parent.width * 0.36
            )

            Layout.fillHeight: true

            Rectangle {
                id: gearCircle

                anchors.centerIn: parent

                width: Math.min(
                    parent.width,
                    parent.height
                ) * 0.88

                height: width

                radius: width / 2

                color: "#07101D"

                border.color: "#19B7F2"
                border.width: 2
            }

            Rectangle {
                anchors.centerIn: gearCircle

                width: gearCircle.width * 0.88
                height: width

                radius: width / 2

                color: "transparent"

                border.color: "#1B3550"
                border.width: 1
            }

            Column {
                anchors.centerIn: gearCircle

                spacing: 0

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: root.gear

                    color: "#F8FAFC"

                    font.pixelSize: Math.min(
                        gearCircle.width * 0.38,
                        48
                    )

                    font.bold: true
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: root.gear === "D"
                          ? "DRIVE"
                          : root.gear === "R"
                            ? "REVERSE"
                            : root.gear === "N"
                              ? "NEUTRAL"
                              : "PARK"

                    color: "#2DA9FF"

                    font.pixelSize: Math.min(
                        gearCircle.width * 0.10,
                        13
                    )

                    font.bold: true

                    elide: Text.ElideRight
                }
            }
        }

        // =================================================
        // RIGHT INFORMATION
        // =================================================

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 7

            // ---------------- GEAR POSITION ----------------

            Text {
                text: "GEAR POSITION"

                color: "#8FA2B9"

                font.pixelSize: 11
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 4

                Repeater {
                    model: ["P", "R", "N", "D"]

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 34

                        radius: 7

                        color: modelData === root.gear
                               ? "#164F9E"
                               : "#0B1625"

                        border.color: modelData === root.gear
                                      ? "#2D8CFF"
                                      : "#1D2D41"

                        border.width: 1

                        Text {
                            anchors.centerIn: parent

                            text: modelData

                            color: modelData === root.gear
                                   ? "white"
                                   : "#8B9BAE"

                            font.pixelSize: 14
                            font.bold: true
                        }
                    }
                }
            }

            // ---------------- SEPARATOR ----------------

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1

                color: "#203047"
            }

            // ---------------- BATTERY / RANGE ----------------

            RowLayout {
                Layout.fillWidth: true

                spacing: 8

                // BATTERY
                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Text {
                        text: "BATTERY"

                        color: "#8FA2B9"

                        font.pixelSize: 10
                        font.bold: true
                    }

                    RowLayout {
                        spacing: 2

                        Text {
                            text: root.battery

                            color: "#29C8FF"

                            font.pixelSize: 20
                            font.bold: true
                        }

                        Text {
                            text: "%"

                            color: "#A9B8C8"

                            font.pixelSize: 11

                            Layout.alignment: Qt.AlignBottom
                        }
                    }
                }

                // RANGE
                ColumnLayout {
                    Layout.fillWidth: true

                    spacing: 0

                    Text {
                        text: "RANGE"

                        color: "#8FA2B9"

                        font.pixelSize: 10
                        font.bold: true
                    }

                    RowLayout {
                        spacing: 3

                        Text {
                            text: root.rangeKm

                            color: "#29C8FF"

                            font.pixelSize: 20
                            font.bold: true
                        }

                        Text {
                            text: "km"

                            color: "#A9B8C8"

                            font.pixelSize: 11

                            Layout.alignment: Qt.AlignBottom
                        }
                    }
                }
            }

            // ---------------- BATTERY BAR ----------------

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 6

                radius: 3

                color: "#1A293B"

                clip: true

                Rectangle {
                    width: parent.width *
                           Math.max(
                               0,
                               Math.min(root.battery, 100)
                           ) / 100

                    height: parent.height

                    radius: 3

                    color: root.battery <= 20
                           ? "#EF4444"
                           : root.battery <= 40
                             ? "#F59E0B"
                             : "#2AAEFF"

                    Behavior on width {
                        NumberAnimation {
                            duration: 400
                        }
                    }
                }
            }
        }
    }
}
