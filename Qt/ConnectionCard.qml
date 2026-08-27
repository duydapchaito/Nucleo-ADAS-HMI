import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Rectangle {
//     id: root

//     property bool connected: false
//     property var ports

//     radius: 16

//     color: "#111C2B"

//     border.color: "#203047"
//     border.width: 1

//     ColumnLayout {
//         anchors.fill: parent
//         anchors.margins: 10

//         spacing: 7

//         RowLayout {
//             Layout.fillWidth: true

//             Text {
//                 text: "CONNECTION"

//                 color: "#94A3B8"

//                 font.pixelSize: 9
//                 font.bold: true
//                 font.letterSpacing: 1
//             }

//             Item {
//                 Layout.fillWidth: true
//             }

//             Text {
//                 text: root.connected
//                       ? "ONLINE"
//                       : "OFFLINE"

//                 color: root.connected
//                        ? "#22C55E"
//                        : "#EF4444"

//                 font.pixelSize: 9
//                 font.bold: true
//             }
//         }

//         RowLayout {
//             Layout.fillWidth: true

//             spacing: 7

//             ComboBox {
//                 id: portCombo

//                 Layout.fillWidth: true
//                 Layout.preferredHeight: 36

//                 model: root.ports

//                 enabled: !root.connected

//                 contentItem: Text {
//                     text: portCombo.displayText !== ""
//                           ? portCombo.displayText
//                           : "Select COM port"

//                     color: "#E2E8F0"

//                     verticalAlignment:
//                         Text.AlignVCenter

//                     leftPadding: 12

//                     font.pixelSize: 11
//                 }

//                 background: Rectangle {
//                     radius: 10

//                     color: "#0A1422"

//                     border.color:
//                         portCombo.activeFocus
//                         ? "#0EA5E9"
//                         : "#26374D"

//                     border.width: 1
//                 }
//             }

//             Mybutton {
//                 implicitWidth: 90
//                 implicitHeight: 36

//                 btnEnanble: true

//                 buttonColor:
//                     root.connected
//                     ? "#7F1D1D"
//                     : "#15803D"

//                 Text {
//                     anchors.centerIn: parent

//                     text:
//                         root.connected
//                         ? "Ngắt"
//                         : "Kết nối"

//                     color: "white"

//                     font.pixelSize: 10
//                     font.bold: true
//                 }

//                 onBtnclicked: {
//                     if (root.connected) {
//                         serialController.disconnectPort()
//                     }
//                     else if (portCombo.currentText !== "") {
//                         serialController.connectPort(
//                             portCombo.currentText,
//                             115200
//                         )
//                     }
//                 }
//             }
//         }
//     }
// }
