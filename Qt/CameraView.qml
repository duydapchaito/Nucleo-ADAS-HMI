import QtQuick
import QtMultimedia
import QtQuick.Layouts

Item {
    id: root
    anchors.fill: parent

    property var activeCamDevice
    property real steeringAngle: 0.0

    // Sử dụng ColumnLayout để chia màn hình thành 2 phần riêng biệt
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // ==========================================
        // 1. KHUNG CAMERA VIEW (PHÍA TRÊN)
        // ==========================================
        Rectangle {
            id: videoContainer
            Layout.fillWidth: true
            Layout.fillHeight: true // Tự động chiếm toàn bộ chiều cao còn lại
            clip: true
            radius: 5
            color: "black"

            VideoOutput {
                id: videoOutput
                anchors.fill: parent
                fillMode: VideoOutput.PreserveAspectCrop
            }

            Canvas {
                id: guideLines
                anchors.fill: videoOutput
                z: 3
                visible: camera.active

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.lineCap = "round"
                    ctx.lineJoin = "round"

                    var topY    = height * 0.55
                    var bottomY = height * 0.95

                    function lerp(a, b, t) { return a + (b - a) * t }

                    var bendOffset = root.steeringAngle * (width * 0.25)

                    var pLeftNear   = { x: width * 0.18, y: bottomY }
                    var pLeftFar    = { x: width * 0.42, y: topY }

                    var pRightNear  = { x: width * 0.82, y: bottomY }
                    var pRightFar   = { x: width * 0.58, y: topY }

                    var ctrlLeft  = { x: lerp(pLeftNear.x, pLeftFar.x, 0.5) + bendOffset, y: lerp(bottomY, topY, 0.5) }
                    var ctrlRight = { x: lerp(pRightNear.x, pRightFar.x, 0.5) + bendOffset, y: lerp(bottomY, topY, 0.5) }

                    var pLeftFarCurved  = { x: pLeftFar.x + bendOffset, y: topY }
                    var pRightFarCurved = { x: pRightFar.x + bendOffset, y: topY }

                    // 1. Vẽ 2 dải biên vàng
                    ctx.lineWidth = 6
                    ctx.strokeStyle = "#FFD700"

                    ctx.beginPath()
                    ctx.moveTo(pLeftNear.x, pLeftNear.y)
                    ctx.quadraticCurveTo(ctrlLeft.x, ctrlLeft.y, pLeftFarCurved.x, pLeftFarCurved.y)
                    ctx.stroke()

                    ctx.beginPath()
                    ctx.moveTo(pRightNear.x, pRightNear.y)
                    ctx.quadraticCurveTo(ctrlRight.x, ctrlRight.y, pRightFarCurved.x, pRightFarCurved.y)
                    ctx.stroke()

                    // 2. Vẽ 4 vạch ngang
                    ctx.lineWidth = 4
                    var horizontalGridSteps = [0.0, 0.30, 0.60, 0.85]

                    for (var j = 0; j < horizontalGridSteps.length; j++) {
                        var t = horizontalGridSteps[j]
                        var yGrid = lerp(bottomY, topY, t)

                        var lx = Math.pow(1-t, 2) * pLeftNear.x
                               + 2 * (1-t) * t * ctrlLeft.x
                               + Math.pow(t, 2) * pLeftFarCurved.x

                        var rx = Math.pow(1-t, 2) * pRightNear.x
                               + 2 * (1-t) * t * ctrlRight.x
                               + Math.pow(t, 2) * pRightFarCurved.x

                        ctx.beginPath()
                        ctx.moveTo(lx, yGrid)
                        ctx.lineTo(rx, yGrid)
                        ctx.stroke()
                    }
                }
                onWidthChanged: requestPaint()
                onHeightChanged: requestPaint()
            }

            Text {
                text: "Rear Camera"
                color: "white"
                font.bold: true
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.margins: 15
                z: 4
            }
        }

        // ==========================================
        // 2. KHUNG PANEL DÀN ĐỀU ICON (PHÍA DƯỚI)
        // ==========================================
        Rectangle {
            id: bottomPanel
            Layout.fillWidth: true
            Layout.preferredHeight: 60 // Độ cao cố định của khung panel dưới
            color: "#0D0E12"           // Màu nền tối tách biệt hoàn toàn

            RowLayout {
                anchors.fill: parent
                spacing: 0

                ListModel {
                    id: iconModel
                    ListElement { iconName: "back"; iconPath: "qrc:/assets/icons/back.svg" }
                    ListElement { iconName: "mode2d"; iconPath: "qrc:/assets/icons/2d.svg" }
                    ListElement { iconName: "radar"; iconPath: "qrc:/assets/icons/radar.svg" }
                    ListElement { iconName: "tow"; iconPath: "qrc:/assets/icons/tow.svg" }
                    ListElement { iconName: "cam_side_left"; iconPath: "qrc:/assets/icons/cam_left.svg" }
                    ListElement { iconName: "cam_side_right"; iconPath: "qrc:/assets/icons/cam_right.svg" }
                    ListElement { iconName: "cam_360"; iconPath: "qrc:/assets/icons/cam_360.svg" }
                    ListElement { iconName: "settings"; iconPath: "qrc:/assets/icons/settings.svg" }
                    ListElement { iconName: "recording"; iconPath: "qrc:/assets/icons/recording.svg" }
                }

                Repeater {
                    model: iconModel
                    delegate: Rectangle {
                        // Thiết lập co giãn đều theo chiều ngang
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: btnMouse.containsPress ? "#33FFFFFF" : (btnMouse.containsMouse ? "#11FFFFFF" : "transparent")

                        // Đường kẻ gạch đứng phân cách giữa các icon
                        Rectangle {
                            width: 1
                            height: parent.height * 0.4
                            color: "#22FFFFFF"
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            visible: index < iconModel.count - 1
                        }

                        Image {
                            anchors.centerIn: parent
                            width: 24
                            height: 24
                            source: model.iconPath
                            fillMode: Image.PreserveAspectFit
                        }

                        MouseArea {
                            id: btnMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                console.log("Selected option:", model.iconName)
                            }
                        }
                    }
                }
            }
        }
    }

    CaptureSession {
        camera: Camera {
            id: camera
            cameraDevice: root.activeCamDevice
            active: true
        }
        videoOutput: videoOutput
    }
}
