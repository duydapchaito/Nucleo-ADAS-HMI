import QtQuick 2.15

Item {
    id: root

    // Khoảng cách nhận vào từ cảm biến (cm)
    property real distance: 100

    // Hướng của cảm biến: "front_left", "front_center", "front_right", "rear_left", "rear_center", "rear_right"
    property string direction: "front_center"

    width: 80
    height: 80

    // Số vạch sóng tối đa hiển thị (ví dụ 4-5 vạch)
    property int maxBars: 4

    // Tính số lượng vạch sóng cần sáng dựa trên khoảng cách (khoảng cách càng nhỏ = càng nhiều vạch báo nguy hiểm)
    function activeBarsCount() {
        if (distance <= 20) return 4;
        if (distance <= 40) return 3;
        if (distance <= 60) return 2;
        if (distance <= 80) return 1;
        return 0; // An toàn / ngoài tầm
    }

    // Lấy màu tương ứng với từng tầng vạch (vạch 0 gần xe nhất -> Xanh, vạch xa nhất -> Đỏ)
    function getBarColor(barIndex) {
        switch (barIndex) {
            case 0: return "#22C55E"; // Xanh lá (Gần nhất)
            case 1: return "#EAB308"; // Vàng
            case 2: return "#F97316"; // Cam
            case 3: return "#EF4444"; // Đỏ (Xa nhất)
            default: return "#22C55E";
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);

            var activeCount = root.activeBarsCount();
            if (activeCount === 0) return;

            var cx = width / 2;
            var cy = height / 2;

            // Xác định góc quay (Angle offset) dựa vào vị trí cảm biến
            var startAngle = 0;
            var endAngle = 0;

            // Thiết lập góc quạt phát ra cho 6 cảm biến (3 trước, 3 sau)
            if (root.direction === "front_center") {
                startAngle = Math.PI * 1.25;
                endAngle = Math.PI * 1.75;
                cy = height * 0.9;
            } else if (root.direction === "front_left") {
                startAngle = Math.PI * 1.05;
                endAngle = Math.PI * 1.45;
                cx = width * 0.8;
                cy = height * 0.9;
            } else if (root.direction === "front_right") {
                startAngle = Math.PI * 1.55;
                endAngle = Math.PI * 1.95;
                cx = width * 0.2;
                cy = height * 0.9;
            } else if (root.direction === "rear_center") {
                startAngle = Math.PI * 0.25;
                endAngle = Math.PI * 0.75;
                cy = height * 0.1;
            } else if (root.direction === "rear_left") {
                startAngle = Math.PI * 0.55;
                endAngle = Math.PI * 0.95;
                cx = width * 0.8;
                cy = height * 0.1;
            } else if (root.direction === "rear_right") {
                startAngle = Math.PI * 0.05;
                endAngle = Math.PI * 0.45;
                cx = width * 0.2;
                cy = height * 0.1;
            }

            var baseRadius = 12;      // Bán kính vạch đầu tiên
            var barThickness = 6;     // Độ dày mỗi vạch
            var barGap = 4;           // Khoảng cách giữa các vạch

            for (var i = 0; i < activeCount; ++i) {
                var rInner = baseRadius + i * (barThickness + barGap);
                var rOuter = rInner + barThickness;

                ctx.beginPath();
                ctx.arc(cx, cy, rInner, startAngle, endAngle, false);
                ctx.arc(cx, cy, rOuter, endAngle, startAngle, true);
                ctx.closePath();

                // Tạo hiệu ứng chuyển màu gradient mượt cho từng vạch
                var grad = ctx.createRadialGradient(cx, cy, rInner, cx, cy, rOuter);
                var mainColor = root.getBarColor(i);

                grad.addColorStop(0, mainColor);
                grad.addColorStop(1, Qt.tint(mainColor, "#22000000")); // Tạo hiệu ứng nổi khối nhẹ

                ctx.fillStyle = grad;
                ctx.globalAlpha = 0.9;
                ctx.fill();
            }
        }

        Connections {
            target: root
            function onDistanceChanged() { canvas.requestPaint(); }
            function onDirectionChanged() { canvas.requestPaint(); }
        }
    }
}
