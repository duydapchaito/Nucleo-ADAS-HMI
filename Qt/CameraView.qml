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
            Layout.preferredHeight: 75 // Độ cao cố định của khung panel dưới
            color: "#0D0E12"           // Màu nền tối tách biệt hoàn toàn

            RowLayout {
                anchors.fill: parent
                spacing: 0

                ListModel {
                    id: iconModel
                    ListElement { iconName: "back"; iconPath: "qrc:/back.png" }
                    ListElement { iconName: "mode2d"; iconPath: "qrc:/SurroundView.png" }
                    ListElement { iconName: "radar"; iconPath: "qrc:/frontsensor.png" }
                    ListElement { iconName: "tow"; iconPath: "qrc:/backsensor.png" }
                    ListElement { iconName: "cam_side_left"; iconPath: "qrc:/LeftView.png" }
                    ListElement { iconName: "cam_side_right"; iconPath: "qrc:/Gemini_Generated_Image_zd75fvzd75fvzd75.png" }
                    ListElement { iconName: "cam_360"; iconPath: "qrc:/sensor360.png" }
                    ListElement { iconName: "settings"; iconPath: "qrc:/Gemini_Generated_Image_car5zocar5zocar5-Picsart-BackgroundRemover.png" }
                    ListElement { iconName: "recording"; iconPath: "qrc:/Gemini_Generated_Image_w3m46yw3m46yw3m4-Picsart-BackgroundRemover.png" }
                }

                Repeater {
                    model: iconModel

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        color: btnMouse.containsPress
                               ? "#33FFFFFF"
                               : (btnMouse.containsMouse
                                  ? "#11FFFFFF"
                                  : "transparent")


                        // =============================================
                        // VẠCH PHÂN CÁCH
                        // =============================================

                        Rectangle {
                            width: 1
                            height: parent.height * 0.5

                            color: "#22FFFFFF"

                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter

                            visible: index < iconModel.count - 1
                        }


                        // =============================================
                        // ICON
                        // =============================================

                        Item {
                            id: iconContainer

                            anchors.centerIn: parent

                            width: 44
                            height: 44


                            // Glow nhẹ phía sau
                            Image {
                                anchors.centerIn: parent

                                width: 40
                                height: 40

                                source: model.iconPath

                                fillMode: Image.PreserveAspectFit

                                smooth: true
                                mipmap: false

                                opacity: 0.18

                                scale: 1.08
                            }


                            // Icon chính
                            Image {
                                id: menuIcon

                                anchors.centerIn: parent

                                width: 38
                                height: 38

                                source: model.iconPath

                                fillMode: Image.PreserveAspectFit

                                smooth: true
                                mipmap: false

                                opacity: 1.0

                                asynchronous: false
                            }
                        }


                        // =============================================
                        // MOUSE
                        // =============================================

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
    }        }
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
