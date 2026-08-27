import QtQuick 2.15

Rectangle {
    property string title: ""
    property string icon: "●"

    radius: 14
    color: "#0D1827"
    border.color: "#1D3148"
    border.width: 1

    Text {
        anchors.centerIn: parent
        text: icon + "\n" + title
        horizontalAlignment: Text.AlignHCenter
        color: "#DCE6F0"
        font.pixelSize: 16
        lineHeight: 1.5
    }
}
