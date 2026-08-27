import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    property bool connected: false

    Layout.fillWidth: true
    Layout.preferredHeight: 48

    spacing: 10

    // ================= ICON =================
    Rectangle {
        Layout.preferredWidth: 40
        Layout.preferredHeight: 40
        Layout.alignment: Qt.AlignVCenter

        radius: 11

        color: "#0EA5E9"
        opacity: 0.15

        border.color: "#0EA5E9"
        border.width: 1

        Text {
            anchors.centerIn: parent

            text: "P"
            color: "#38BDF8"

            font.pixelSize: 21
            font.bold: true
        }
    }

    // ================= TITLE =================
    ColumnLayout {
        Layout.fillWidth: true
        Layout.minimumWidth: 0

        spacing: 1

        Text {
            Layout.fillWidth: true

            text: "PARKING ASSIST"

            color: "#F8FAFC"

            font.pixelSize: 16
            font.bold: true
            font.letterSpacing: 1.0

            elide: Text.ElideRight
        }

        Text {
            Layout.fillWidth: true

            text: "VEHICLE CONTROL SYSTEM"

            color: "#64748B"

            font.pixelSize: 8
            font.bold: true
            font.letterSpacing: 1.2

            elide: Text.ElideRight
        }
    }

    // ================= STATUS =================
    Rectangle {
        Layout.preferredWidth: 9
        Layout.preferredHeight: 9
        Layout.alignment: Qt.AlignVCenter

        radius: width / 2

        color: root.connected
               ? "#22C55E"
               : "#EF4444"

        // Glow nhẹ
        Rectangle {
            anchors.centerIn: parent

            width: parent.width + 6
            height: parent.height + 6

            radius: width / 2

            color: "transparent"
            border.width: 1
            border.color: parent.color
            opacity: 0.25
        }
    }
}
