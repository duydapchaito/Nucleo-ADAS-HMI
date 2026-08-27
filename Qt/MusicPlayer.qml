import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root

    radius: 22
    color: "#101C2B"
    border.color: "#203650"
    border.width: 1
    clip: true

    property bool playing: true
    property int progress: 36

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

        text: "♫  MUSIC PLAYER"

        color: "#B4C4D6"

        font.pixelSize: Math.max(
            8,
            Math.min(12, root.height * 0.045)
        )

        font.bold: true
        font.letterSpacing: 0.8

        elide: Text.ElideRight
    }

    // =====================================================
    // ALBUM + SONG
    // =====================================================

    RowLayout {
        id: songArea

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: title.bottom

        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.topMargin: 7

        height: Math.min(
            82,
            root.height * 0.32
        )

        spacing: 10

        // Album
        Rectangle {
            Layout.preferredWidth: Math.min(
                82,
                songArea.height
            )

            Layout.preferredHeight: Math.min(
                82,
                songArea.height
            )

            radius: 9

            color: "#172B43"
            border.color: "#2777B8"

            Text {
                anchors.centerIn: parent

                text: "♪"

                color: "#37BFFF"

                font.pixelSize: Math.min(
                    38,
                    parent.width * 0.5
                )
            }
        }

        // Song information
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            spacing: 2

            Text {
                Layout.fillWidth: true

                text: "Night Drive"

                color: "#F1F5F9"

                font.pixelSize: Math.max(
                    12,
                    Math.min(18, root.width * 0.042)
                )

                font.bold: true

                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                Layout.fillWidth: true

                text: "The Midnight"

                color: "#A7B7C9"

                font.pixelSize: Math.max(
                    9,
                    Math.min(13, root.width * 0.03)
                )

                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                Layout.fillWidth: true

                text: "Endless Summer"

                color: "#A7B7C9"

                font.pixelSize: Math.max(
                    9,
                    Math.min(13, root.width * 0.03)
                )

                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }
    }

    // =====================================================
    // PROGRESS BAR
    // =====================================================

    Rectangle {
        id: progressBackground

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: songArea.bottom

        anchors.leftMargin: 18
        anchors.rightMargin: 18
        anchors.topMargin: 6

        height: 4

        radius: 2

        color: "#1C2B3D"

        Rectangle {
            width: parent.width * root.progress / 100
            height: parent.height

            radius: 2
            color: "#24AFFF"
        }
    }

    // =====================================================
    // TIME
    // =====================================================

    Text {
        id: currentTime

        anchors.left: parent.left
        anchors.top: progressBackground.bottom

        anchors.leftMargin: 18
        anchors.topMargin: 3

        text: "01:26"

        color: "#8EA1B6"

        font.pixelSize: Math.max(
            8,
            Math.min(11, root.height * 0.035)
        )
    }

    Text {
        id: totalTime

        anchors.right: parent.right
        anchors.top: progressBackground.bottom

        anchors.rightMargin: 18
        anchors.topMargin: 3

        text: "03:47"

        color: "#8EA1B6"

        font.pixelSize: Math.max(
            8,
            Math.min(11, root.height * 0.035)
        )
    }

    // =====================================================
    // CONTROLS
    // =====================================================

    Row {
        id: controls

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

        anchors.bottomMargin: 7

        spacing: Math.max(
            14,
            Math.min(24, root.width * 0.055)
        )

        // Previous
        Text {
            anchors.verticalCenter: playButton.verticalCenter

            text: "◀"

            color: "#E8F0F8"

            font.pixelSize: Math.max(
                16,
                Math.min(23, root.width * 0.052)
            )
        }

        // Play / Pause
        Rectangle {
            id: playButton

            width: Math.min(
                48,
                root.height * 0.18
            )

            height: width

            radius: width / 2

            color: "#0C1828"

            border.color: "#238BFF"
            border.width: 2

            Text {
                anchors.centerIn: parent

                text: root.playing ? "Ⅱ" : "▶"

                color: "white"

                font.pixelSize: Math.max(
                    14,
                    parent.width * 0.38
                )
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    root.playing = !root.playing
                }
            }
        }

        // Next
        Text {
            anchors.verticalCenter: playButton.verticalCenter

            text: "▶"

            color: "#E8F0F8"

            font.pixelSize: Math.max(
                16,
                Math.min(23, root.width * 0.052)
            )
        }
    }
}
