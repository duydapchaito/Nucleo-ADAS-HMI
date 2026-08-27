import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    radius: 22
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1

    // =====================================================
    // HEADER
    // =====================================================

    Text {
        id: title

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.topMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 16

        text: "VEHICLE INFO"

        color: "#B4C4D6"

        font.pixelSize: Math.max(
            9,
            Math.min(11, root.width * 0.03)
        )

        font.bold: true
        font.letterSpacing: 1

        elide: Text.ElideRight
        maximumLineCount: 1
    }

    // =====================================================
    // INFORMATION
    // =====================================================

    ColumnLayout {
        id: infoColumn

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom
        anchors.bottom: parent.bottom

        anchors.leftMargin: 10
        anchors.rightMargin: 10
        anchors.topMargin: 8
        anchors.bottomMargin: 10

        spacing: 2

        InfoRow {
            Layout.fillWidth: true
            Layout.fillHeight: true

            label: "◉   ODO"
            value: "12,560"
            unit: "km"
        }

        InfoRow {
            Layout.fillWidth: true
            Layout.fillHeight: true

            label: "◉   AVG. SPEED"
            value: "46"
            unit: "km/h"
        }

        InfoRow {
            Layout.fillWidth: true
            Layout.fillHeight: true

            label: "◉   MAX SPEED"
            value: "128"
            unit: "km/h"
        }

        InfoRow {
            Layout.fillWidth: true
            Layout.fillHeight: true

            label: "◷   DRIVE TIME"
            value: "02:35"
            unit: "h"
        }
    }
}
