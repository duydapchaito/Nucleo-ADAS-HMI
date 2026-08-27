import QtQuick 2.15

Column {
    property string title: ""
    property string value: ""
    spacing: 2

    Text { text: title; color: "#7F94A9"; font.pixelSize: 12 }
    Text { text: value; color: "#E5EDF5"; font.pixelSize: 14; font.bold: true }
}
