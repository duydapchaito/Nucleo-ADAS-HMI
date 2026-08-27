import QtQuick
import QtQuick.Layouts

Rectangle {
    property bool connected: false

    color: "#0B1422"

    border.color: "#1B2A3D"
    border.width: 1

    RowLayout {
        anchors.fill: parent

        anchors.leftMargin: 18
        anchors.rightMargin: 18

        Text {
            text: "SYSTEM"

            color: "#64748B"

            font.pixelSize: 8
            font.bold: true
        }

        Text {
            text:
                connected
                ? "READY"
                : "WAITING FOR CONNECTION"

            color:
                connected
                ? "#22C55E"
                : "#F59E0B"

            font.pixelSize: 9
            font.bold: true
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 18

            color: "#26374D"
        }

        Text {
            text: "UART 115200"

            color: "#64748B"

            font.pixelSize: 8
        }

        Item {
            Layout.fillWidth: true
        }

        Text {
            text: "PARKING HMI 1.0"

            color: "#475569"

            font.pixelSize: 8
            font.bold: true
        }
    }
}
