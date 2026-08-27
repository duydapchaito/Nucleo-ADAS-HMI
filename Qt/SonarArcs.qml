import QtQuick 2.15

Item {
    id: root
    width: 120
    height: 70

    property string direction: "front"
    property int level: 0

    function arcColor() {
        return level === 2 ? "#EF4444" : (level === 1 ? "#F59E0B" : "#22C55E")
    }

    Repeater {
        model: 3

        Rectangle {
            property int i: index
            width: root.direction === "left" || root.direction === "right"
                   ? 10 + i * 4 : 58 + i * 18
            height: root.direction === "left" || root.direction === "right"
                    ? 58 + i * 8 : 10
            radius: 30
            color: "transparent"
            border.color: root.arcColor()
            border.width: 5

            anchors.centerIn: parent

            opacity: 0.95 - i * 0.15

            transform: Rotation {
                origin.x: width / 2
                origin.y: height / 2
                angle: root.direction === "left" ? 90
                       : root.direction === "right" ? -90
                       : root.direction === "rear" ? 180 : 0
            }
        }
    }
}
