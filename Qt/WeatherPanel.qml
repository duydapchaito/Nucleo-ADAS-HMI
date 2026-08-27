import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    radius: 22
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1

    // Không cho nội dung vẽ ra ngoài panel
    clip: true

    // =====================================================
    // HEADER
    // =====================================================

    Text {
        id: title

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.topMargin: 10

        text: "WEATHER"

        color: "#B4C4D6"

        font.pixelSize: Math.max(
            9,
            Math.min(13, root.height * 0.055)
        )

        font.bold: true
        font.letterSpacing: 0.8

        elide: Text.ElideRight
        maximumLineCount: 1
    }

    // =====================================================
    // WEATHER MAIN
    // =====================================================

    RowLayout {
        id: weatherMain

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom

        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.topMargin: 5

        height: Math.min(
            70,
            root.height * 0.36
        )

        spacing: 8

        // ---------------- ICON ----------------

        Item {
            Layout.preferredWidth: Math.min(
                65,
                weatherMain.height
            )

            Layout.preferredHeight: weatherMain.height

            Text {
                anchors.centerIn: parent

                text: "☁☾"

                color: "#E2EAF2"

                font.pixelSize: Math.min(
                    40,
                    parent.height * 0.62
                )
            }
        }

        // ---------------- TEMPERATURE ----------------

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            spacing: 0

            Text {
                Layout.fillWidth: true

                text: "28°C"

                color: "#F8FAFC"

                font.pixelSize: Math.max(
                    22,
                    Math.min(34, root.height * 0.17)
                )

                font.bold: true

                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                Layout.fillWidth: true

                text: "Partly Cloudy"

                color: "#B6C2D0"

                font.pixelSize: Math.max(
                    9,
                    Math.min(13, root.height * 0.06)
                )

                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }
    }

    // =====================================================
    // SEPARATOR
    // =====================================================

    Rectangle {
        id: separator

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: weatherMain.bottom

        anchors.leftMargin: 18
        anchors.rightMargin: 18

        height: 1

        color: "#203047"
    }

    // =====================================================
    // WEATHER METRICS
    // =====================================================

    RowLayout {
        id: metrics

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: separator.bottom
        anchors.bottom: parent.bottom

        anchors.leftMargin: 18
        anchors.rightMargin: 18

        anchors.topMargin: 5
        anchors.bottomMargin: 6

        spacing: 4

        WeatherMetric {
            Layout.fillWidth: true
            Layout.fillHeight: true

            title: "Humidity"
            value: "64%"
        }

        WeatherMetric {
            Layout.fillWidth: true
            Layout.fillHeight: true

            title: "Wind"
            value: "12 km/h"
        }

        WeatherMetric {
            Layout.fillWidth: true
            Layout.fillHeight: true

            title: "Visibility"
            value: "10 km"
        }
    }
}
