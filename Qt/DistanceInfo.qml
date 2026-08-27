import QtQuick 2.15

Column {
    property string direction: "FRONT"
    property real distance: 80
    property int level: distance < 20 ? 2 : (distance < 50 ? 1 : 0)

    spacing: 1
    z: 10

    Text {
        text: parent.direction
        color: "#8FA2B9"
        font.pixelSize: 12
        font.bold: true
    }

    Text {
        text: Math.round(parent.distance) + " cm"
        color: "#F1F5F9"
        font.pixelSize: 16
        font.bold: true
    }

    Text {
        text: parent.level === 2 ? "▲ DANGER"
             : parent.level === 1 ? "⚠ WARNING" : "✓ SAFE"
        color: parent.level === 2 ? "#EF4444"
             : parent.level === 1 ? "#FBBF24" : "#22C55E"
        font.pixelSize: 11
        font.bold: true
    }
}
