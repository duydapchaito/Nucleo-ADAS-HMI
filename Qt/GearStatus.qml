import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    property string gear: serialController ? serialController.gear : "P"

    radius: 16
    color: "#111C2B"

    border.color: "#203047"
    border.width: 1

    // =========================
    // GEAR COLOR
    // =========================
    function gearColor() {
        switch (root.gear) {
        case "R":
            return "#EF4444"

        case "N":
            return "#F59E0B"

        case "D":
            return "#22C55E"

        case "P":
        default:
            return "#3B82F6"
        }
    }

    // =========================
    // GEAR NAME
    // =========================
    function gearName() {
        switch (root.gear) {
        case "R":
            return "REVERSE"

        case "N":
            return "NEUTRAL"

        case "D":
            return "DRIVE"

        case "P":
        default:
            return "PARK"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12

        spacing: 8

        // =========================
        // TITLE
        // =========================
        Text {
            Layout.fillWidth: true

            text: "VEHICLE STATE"

            color: "#94A3B8"

            font.pixelSize: 9
            font.bold: true
            font.letterSpacing: 1
        }

        // =========================
        // CONTENT
        // =========================
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            spacing: 14

            // =========================
            // GEAR CIRCLE
            // =========================
            Rectangle {
                Layout.preferredWidth: 82
                Layout.preferredHeight: 82

                Layout.alignment: Qt.AlignVCenter

                radius: width / 2

                color: "#08111E"

                border.color: root.gearColor()
                border.width: 2

                // Inner glow/ring
                Rectangle {
                    anchors.centerIn: parent

                    width: parent.width - 10
                    height: parent.height - 10

                    radius: width / 2

                    color: "transparent"

                    border.color: root.gearColor()
                    border.width: 1

                    opacity: 0.25
                }

                Text {
                    anchors.centerIn: parent

                    text: root.gear || "P"

                    color: root.gearColor()

                    font.pixelSize: 32
                    font.bold: true
                }
            }

            // =========================
            // GEAR INFORMATION
            // =========================
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Layout.alignment: Qt.AlignVCenter

                spacing: 5

                Text {
                    Layout.fillWidth: true

                    text: "CURRENT GEAR"

                    color: "#64748B"

                    font.pixelSize: 8
                    font.bold: true
                    font.letterSpacing: 0.5
                }

                Text {
                    Layout.fillWidth: true

                    text: root.gearName()

                    color: "#E2E8F0"

                    font.pixelSize: 15
                    font.bold: true

                    elide: Text.ElideRight
                }

                // =========================
                // STATUS BAR
                // =========================
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4

                    radius: 2

                    color: "#1E293B"

                    Rectangle {
                        width: parent.width * 0.7
                        height: parent.height

                        radius: 2

                        color: root.gearColor()

                        Behavior on color {
                            ColorAnimation {
                                duration: 180
                            }
                        }
                    }
                }

                // =========================
                // GEAR INDICATOR
                // =========================
                Row {
                    spacing: 5

                    Repeater {
                        model: ["P", "R", "N", "D"]

                        Rectangle {
                            width: 20
                            height: 16

                            radius: 4

                            color: modelData === root.gear
                                   ? root.gearColor()
                                   : "#182536"

                            border.color: modelData === root.gear
                                          ? root.gearColor()
                                          : "#26364A"

                            border.width: 1

                            Text {
                                anchors.centerIn: parent

                                text: modelData

                                color: modelData === root.gear
                                       ? "#FFFFFF"
                                       : "#64748B"

                                font.pixelSize: 8
                                font.bold: true
                            }

                            Behavior on color {
                                ColorAnimation {
                                    duration: 150
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // =========================
    // GEAR CHANGE ANIMATION
    // =========================
    Behavior on border.color {
        ColorAnimation {
            duration: 180
        }
    }
}
