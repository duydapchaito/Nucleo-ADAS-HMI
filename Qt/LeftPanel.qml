import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    radius: 28
    color: "#0B1422"
    border.color: "#20324A"
    border.width: 2

    property string gear
    property real alertFrontLeft
    property real alertFrontCenter
    property real alertFrontRight
    property real alertRearLeft
    property real alertRearCenter
    property real alertRearRight

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        VehicleState {
            Layout.fillWidth: true
            Layout.preferredHeight: 180
            Layout.minimumHeight: 160
            gear: root.gear
        }

        GridLayout {

            Layout.fillWidth: true
            Layout.preferredHeight: 145

            columns: 2
            rows: 2

            columnSpacing: 10
            rowSpacing: 10

            MusicPlayer {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            TPMS {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            VehicleInfo {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            WeatherPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        ParkingSensorPanel {

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 250



           alertFrontLeft: root.alertFrontLeft
           alertFrontRight: root.alertFrontRight
           alertFrontCenter: root.alertFrontCenter
           alertRearLeft: root.alertRearLeft
           alertRearRight: root.alertRearRight
           alertRearCenter: root.alertRearCenter

        }

        // HmiFooter {
        //     Layout.fillWidth: true
        //     Layout.preferredHeight: 28
        // }
    }
}
