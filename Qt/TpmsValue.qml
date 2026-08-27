import QtQuick
import QtQuick.Layouts

ColumnLayout {
    id: root

    property string pressure: "2.4"
    property string temperature: "28°C"

    spacing: 0

    Text {
        text: root.pressure + " bar"

        color: "#29C8FF"

        font.pixelSize: 11
        font.bold: true

        Layout.alignment: Qt.AlignHCenter
    }

    Text {
        text: root.temperature

        color: "#7F91A5"

        font.pixelSize: 9

        Layout.alignment: Qt.AlignHCenter
    }
}
