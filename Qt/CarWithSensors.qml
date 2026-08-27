import QtQuick 2.15
import SensorUI 1.0

Item {
    id: root

    width: 400
    height: 400

    property string gear: serialController.gear

    // =====================================================
    // 6 SENSOR ALERT LEVEL
    // =====================================================

    property int alertFrontLeft:   serialController.alertFrontLeft
    property int alertFrontCenter: serialController.alertFrontCenter
    property int alertFrontRight:  serialController.alertFrontRight

    property int alertRearLeft:    serialController.alertRearLeft
    property int alertRearCenter:  serialController.alertRearCenter
    property int alertRearRight:   serialController.alertRearRight


    // =====================================================
    // GEAR MASK
    // =====================================================

    // Khi R:
    // chỉ hiển thị cảm biến phía sau
    //
    // Khi D:
    // chỉ hiển thị cảm biến phía trước
    //
    // Khi P/N:
    // có thể hiển thị cả 6

    property int displayFrontLeft:
        (gear === "R") ? -1 : alertFrontLeft

    property int displayFrontCenter:
        (gear === "R") ? -1 : alertFrontCenter

    property int displayFrontRight:
        (gear === "R") ? -1 : alertFrontRight


    property int displayRearLeft:
        (gear === "D") ? -1 : alertRearLeft

    property int displayRearCenter:
        (gear === "D") ? -1 : alertRearCenter

    property int displayRearRight:
        (gear === "D") ? -1 : alertRearRight


    // =====================================================
    // CAR
    // =====================================================

    Item {
        id: car

        width: Math.min(root.width, root.height) * 0.32
        height: width * 1.9

        anchors.centerIn: parent

        Image {
            anchors.fill: parent

            source: "qrc:/carRED.png"

            fillMode: Image.PreserveAspectFit
        }


        // =================================================
        // FRONT LEFT
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 4

            alertLevel: root.displayFrontLeft
        }


        // =================================================
        // FRONT CENTER
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 0

            alertLevel: root.displayFrontCenter
        }


        // =================================================
        // FRONT RIGHT
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 5

            alertLevel: root.displayFrontRight
        }


        // =================================================
        // REAR LEFT
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 6

            alertLevel: root.displayRearLeft
        }


        // =================================================
        // REAR CENTER
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 1

            alertLevel: root.displayRearCenter
        }


        // =================================================
        // REAR RIGHT
        // =================================================

        SensorSurCar {
            anchors.fill: parent

            direction: 7

            alertLevel: root.displayRearRight
        }
    }
}
