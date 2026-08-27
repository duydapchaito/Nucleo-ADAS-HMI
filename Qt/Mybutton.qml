import QtQuick
import QtQuick.Controls

Item {
    implicitWidth: 80
    implicitHeight: 60
    property string buttonColor : "lightgray"
    property real btnRadius: 0
    property string iconSource: ""
    property bool  btnEnanble: false
    signal btnclicked()

    Rectangle {
        id: btnBackground
        anchors.fill: parent
        color: btnEnanble ? buttonColor : "gray"
        radius: btnRadius

        Image {
            id: btnIcon
            anchors.fill: parent
            anchors.margins: 10
            source: iconSource
            antialiasing: true
            fillMode: Image.PreserveAspectFit
            opacity: btnEnanble ? 1.0 : 0.5
        }

        MouseArea {
            anchors.fill: parent
            enabled: btnEnanble
            onClicked: {
                btnclicked()
            }

        }
    }
}
