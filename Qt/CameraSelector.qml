import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property var devices

    signal cameraChanged(var device)

    radius: 16

    color: "#111C2B"

    border.color: "#203047"
    border.width: 1

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        spacing: 6

        Text {
            text: "REAR CAMERA"

            color: "#94A3B8"

            font.pixelSize: 9
            font.bold: true
            font.letterSpacing: 1
        }

        ComboBox {
            id: cameraCombo

            Layout.fillWidth: true
            Layout.preferredHeight: 32

            model: root.devices
            textRole: "description"

            contentItem: Text {
                text:
                    cameraCombo.displayText !== ""
                    ? cameraCombo.displayText
                    : "No camera selected"

                color: "#E2E8F0"

                verticalAlignment:
                    Text.AlignVCenter

                leftPadding: 10

                font.pixelSize: 10
            }

            background: Rectangle {
                radius: 9

                color: "#0A1422"

                border.color: "#26374D"
                border.width: 1
            }

            onActivated: {
                if (root.devices) {
                    root.cameraChanged(
                        root.devices[index]
                  )
                 }
            }
        }
    }
}
