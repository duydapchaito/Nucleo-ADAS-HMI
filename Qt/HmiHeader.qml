import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property string gear: "P"
    property bool connected: false

    color: "#0B1422"

    border.color: "#1B2A3D"
    border.width: 1

    RowLayout {
        anchors.fill: parent

        anchors.leftMargin: 18
        anchors.rightMargin: 18

        Text {
            text: "PARKING"

            color: "#F8FAFC"

            font.pixelSize: 17
            font.bold: true
            font.letterSpacing: 2
        }

        Text {
            text: "ASSIST"

            color: "#0EA5E9"

            font.pixelSize: 17
            font.bold: true
            font.letterSpacing: 2
        }

        Item {
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.preferredWidth: 105
            Layout.preferredHeight: 32

            radius: 16

            color:
                root.connected
                ? "#08261A"
                : "#281116"

            border.color:
                root.connected
                ? "#166534"
                : "#7F1D1D"

            Row {
                anchors.centerIn: parent

                spacing: 7

                Rectangle {
                    width: 7
                    height: 7

                    radius: 4

                    color:
                        root.connected
                        ? "#22C55E"
                        : "#EF4444"
                }

                Text {
                    text:
                        root.connected
                        ? "CONNECTED"
                        : "OFFLINE"

                    color:
                        root.connected
                        ? "#4ADE80"
                        : "#F87171"

                    font.pixelSize: 8
                    font.bold: true
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 42
            Layout.preferredHeight: 32

            radius: 9

            color:
                root.gear === "R"
                ? "#3A1419"
                : "#10243A"

            border.color:
                root.gear === "R"
                ? "#EF4444"
                : "#0EA5E9"

            Text {
                anchors.centerIn: parent

                text: root.gear || "P"

                color:
                    root.gear === "R"
                    ? "#F87171"
                    : "#38BDF8"

                font.pixelSize: 14
                font.bold: true
            }
        }
    }
}
