import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtMultimedia 6.5
import Parking_Sensor 1.0

Window {
    id: root

    width: 1600
    height: 900
    minimumWidth: 1200
    minimumHeight: 700

    visible: true
    title: "Parking Assist HMI"
    color: "#020817"

    MediaDevices {
        id: mediaDevices
    }

    // ==============================
    // VEHICLE DATA
    // ==============================

    property string gearValue:
        serialController ? serialController.gear : "P"



    // ==============================
    // PARKING SENSOR ALERT LEVEL
    // ==============================

    property int alertFrontLeft:
        serialController ? serialController.alertFrontLeft : 0

    property int alertFrontCenter:
        serialController ? serialController.alertFrontCenter : 0

    property int alertFrontRight:
        serialController ? serialController.alertFrontRight : 0

    property int alertRearLeft:
        serialController ? serialController.alertRearLeft : 0

    property int alertRearCenter:
        serialController ? serialController.alertRearCenter : 0

    property int alertRearRight:
        serialController ? serialController.alertRearRight : 0

    // ==============================
    // BACKGROUND
    // ==============================

    Rectangle {
        anchors.fill: parent
        color: "#020817"

        Image {
            anchors.fill: parent

            source: "qrc:/dashboard_bg.png"

            fillMode: Image.PreserveAspectCrop
            opacity: 0.34
            smooth: true
        }

        Rectangle {
            anchors.fill: parent
            color: "#020817"
            opacity: 0.58
        }
    }


    // ==============================
    // MAIN LAYOUT
    // ==============================

    RowLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 16

        // ==============================
        // LEFT PANEL - 30%
        // ==============================
        LeftPanel {
            id: leftPanel

            Layout.fillHeight: true
            Layout.preferredWidth: 3
            Layout.fillWidth: true
            Layout.minimumWidth: 0

            gear: root.gearValue

            alertFrontLeft: root.alertFrontLeft
            alertFrontRight: root.alertFrontRight
            alertFrontCenter: root.alertFrontCenter
            alertRearLeft: root.alertRearLeft
            alertRearRight: root.alertRearRight
            alertRearCenter: root.alertRearCenter

        }

        // ==============================
        // RIGHT PANEL - 70%
        // ==============================
        DashboardView {
            id: dashboard

            Layout.fillHeight: true
            Layout.preferredWidth: 7
            Layout.fillWidth: true
            Layout.minimumWidth: 0

            gear: root.gearValue
        }
    }
}
