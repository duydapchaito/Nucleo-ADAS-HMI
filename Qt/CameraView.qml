import QtQuick
import QtMultimedia

Item {
    id: root
    anchors.fill: parent

    // Nhận object cameraDevice từ Main.qml gửi sang
    property var activeCamDevice

    Canvas {
        id: guideLines
        anchors.fill: parent
        z: 3
        visible: camera.active

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.lineCap = "round"
            ctx.lineJoin = "round"

            var topY    = height * 0.45
            var bottomY = height * 1.0

            function lerp(a, b, t) { return a + (b - a) * t }

            var zones = [
                { t0: 0.0,  t1: 0.25, color: "rgba(220, 0, 0, 0.35)" },
                { t0: 0.25, t1: 0.55, color: "rgba(255, 170, 0, 0.25)" },
                { t0: 0.55, t1: 1.0,  color: "rgba(0, 200, 0, 0.20)" }
            ]

            var pLeftNear   = { x: width * 0.15, y: bottomY }
            var pLeftFar    = { x: width * 0.40, y: topY }
            var ctrlLeft    = { x: width * 0.10, y: lerp(bottomY, topY, 0.5) }

            var pRightNear  = { x: width * 0.85, y: bottomY }
            var pRightFar   = { x: width * 0.60, y: topY }
            var ctrlRight   = { x: width * 0.90, y: lerp(bottomY, topY, 0.5) }

            // Nền màu
            for (var i = 0; i < zones.length; i++) {
                var z = zones[i]
                var y0 = lerp(bottomY, topY, z.t0); var y1 = lerp(bottomY, topY, z.t1)
                var lx0 = lerp(pLeftNear.x, pLeftFar.x, z.t0); var rx0 = lerp(pRightNear.x, pRightFar.x, z.t0)
                var lx1 = lerp(pLeftNear.x, pLeftFar.x, z.t1); var rx1 = lerp(pRightNear.x, pRightFar.x, z.t1)

                ctx.beginPath(); ctx.moveTo(lx0, y0); ctx.lineTo(lx1, y1); ctx.lineTo(rx1, y1); ctx.lineTo(rx0, y0); ctx.closePath()
                ctx.fillStyle = z.color; ctx.fill()
            }

            // Biên vàng
            ctx.lineWidth = 5; ctx.strokeStyle = "rgba(255, 200, 0, 1.0)"
            ctx.beginPath(); ctx.moveTo(pLeftNear.x, pLeftNear.y); ctx.bezierCurveTo(ctrlLeft.x, ctrlLeft.y, ctrlLeft.x, ctrlLeft.y, pLeftFar.x, pLeftFar.y); ctx.stroke()
            ctx.beginPath(); ctx.moveTo(pRightNear.x, pRightNear.y); ctx.bezierCurveTo(ctrlRight.x, ctrlRight.y, ctrlRight.x, ctrlRight.y, pRightFar.x, pRightFar.y); ctx.stroke()

            // Vạch ngang
            ctx.lineWidth = 2; ctx.strokeStyle = "rgba(255, 255, 255, 0.7)"
            for (var j = 1; j <= 6; j++) {
                var t = j / 7; var yGrid = lerp(bottomY, topY, t)
                var lxGrid = lerp(pLeftNear.x, pLeftFar.x, t); var rxGrid = lerp(pRightNear.x, pRightFar.x, t)
                ctx.beginPath(); ctx.moveTo(lxGrid, yGrid); ctx.lineTo(rxGrid, yGrid); ctx.stroke()
            }

            // Vạch dừng
            ctx.lineWidth = 6; ctx.strokeStyle = "rgba(220, 0, 0, 1.0)"
            var lyDanger = lerp(bottomY, topY, 0.20); var lxDanger = lerp(pLeftNear.x, pLeftFar.x, 0.20); var rxDanger = lerp(pRightNear.x, pRightFar.x, 0.20)
            ctx.beginPath(); ctx.moveTo(lxDanger, lyDanger); ctx.lineTo(rxDanger, lyDanger); ctx.stroke()
        }
        onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
    }

    Rectangle {
        anchors { fill: parent; margins: 10 }
        clip: true
        radius: 5
        color: "transparent"
        z: 2

        VideoOutput {
            id: videoOutput
            anchors.fill: parent
            fillMode: VideoOutput.PreserveAspectCrop
        }
    }

    Text {
        text: "Rear Camera"
        color: "white"
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.margins: 15
        z: 3
    }

    // CaptureSession chuẩn của Qt 6
    CaptureSession {
        camera: Camera {
            id: camera
            // Liên kết thuộc tính cameraDevice với object được chọn
            cameraDevice: root.activeCamDevice
            active: true
        }
        videoOutput: videoOutput
    }
}
