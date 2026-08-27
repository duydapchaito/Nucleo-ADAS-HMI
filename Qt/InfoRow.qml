import QtQuick 2.15

Item {
    property string label: ""
    property string value: ""
    property string unit: ""

    width: parent.width
    height: 40

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: "#1E3045"
    }

    Text {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        text: parent.label
        color: "#94A7BA"
        font.pixelSize: 14
    }

    Row {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Text { text: parent.parent.value; color: "#2DC2FF"; font.pixelSize: 18; font.bold: true }
        Text { text: parent.parent.unit; color: "#9BAABD"; font.pixelSize: 13; anchors.bottom: parent.bottom }
    }
}
