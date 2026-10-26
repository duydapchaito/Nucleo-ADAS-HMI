# Công Cụ Gateway Giám Sát UART & CAN Bus (Python / PyQt5)
Ứng dụng giao diện Desktop hiệu năng cao được xây dựng bằng Python và PyQt5, đóng vai trò trạm trung chuyển dữ liệu (Gateway) và giám sát thời gian thực cho hệ thống ECU STM32. Công cụ hỗ trợ thu thập dòng log UART phần cứng, giải mã các gói tin CAN Bus qua thiết bị Waveshare USB-CAN-A (kèm chế độ giả lập Mock), đồng thời chuyển tiếp luồng dữ liệu telemetry qua UDP socket lên ứng dụng Dashboard HMI.

## Tính Năng Nổi Bật
*   **Nhận Diện Phần Cứng Tự Động:** Quét tập trung toàn bộ cổng COM vật lý qua thanh điều khiển chung; tự động ẩn/hiện thiết bị Waveshare/SLCAN theo thời gian thực khi cắm hoặc rút cáp USB.
*   **Giải Mã CAN Tốc Độ Cao:** Đọc trực tiếp luồng nhị phân non-blocking ở baudrate 2.000.000 bps, giải mã chi tiết các trường ID, DLC, Payload HEX/BIT, trạng thái cần số và khoảng cách cảm biến siêu âm.
*   **Tùy Biến Luồng Hiển Thị:** Hỗ trợ 3 chế độ xem log CAN (1-Line Smooth chạy song song mượt mà với log UART, 2-Line Compact và 3-Line Full hiển thị chi tiết từng bit).

*   **Phân Tuyến UDP Nội Bộ:** Đóng gói tự động bản tin CAN 0x300 thành khung nhị phân chuẩn và chuyển tiếp tức thì qua UDP port 5005 đến HMI Dashboard mà không gây nghẽn giao diện.

*   **Bố Cục Tối Ưu (25% UART - 75% CAN):** Phân chia khung nhìn khoa học, chống tràn dòng (wrap text), đảm bảo theo dõi telemetry thông suốt ở chế độ toàn màn hình.

## Hướng Dẫn Cài Đặt & Khởi Chạy
**Yêu cầu hệ thống:**
*   Python 3.10 trở lên.

*   Hệ điều hành: Windows 11 hoặc Linux (Ubuntu).

*   Thiết bị phần cứng: Mạch STM32 kết nối cáp USB-to-UART, module Waveshare USB-CAN-A (tùy chọn khi chạy thực tế).

**1. Thiết lập môi trường ảo (Virtualenv)**
**Trên Windows:**

```
cd Tools

# Khởi tạo môi trường ảo
python -m venv .venv

# Kích hoạt môi trường
.\.venv\Scripts\activate

# Cài đặt các thư viện phụ thuộc
pip install -r requirements.txt
```

*(Nếu PowerShell chặn chạy script, mở PowerShell quyền Administrator và thực thi:* `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`*)*

**Trên Linux (Ubuntu):**

```
cd Tools

# Khởi tạo và kích hoạt môi trường ảo
python3 -m venv .venv
source .venv/bin/activate

# Cài đặt các thư viện phụ thuộc
pip install -r requirements.txt
```
**2. Khởi chạy ứng dụng**

Sau khi kích hoạt môi trường ảo (`.venv`):

```
python gateway.py
```

## Hướng Dẫn Vận Hành Nhanh
1.  **Quét thiết bị:** Nhấn nút ↻ Scan Hardware (USB / COM) trên thanh tiêu đề chung để nạp danh sách cổng COM hiện có.

2.  **Kênh UART:** Chọn đúng cổng COM của mạch STM32 (ví dụ COM3), đặt Baud 115200 và nhấn Connect.

3.  **Kênh CAN Bus:**
*  *Chạy phần cứng thật:* Chọn `Iface: waveshare`, chọn cổng COM thiết bị (ví dụ `COM4`), giữ USB Baud `2000000`, CAN Rate `500000` và nhấn **Start CAN**.
*   *Chạy giả lập:* Chọn `Iface: mock` và nhấn Start CAN để phát luồng dữ liệu mẫu kiểm thử giao diện.

4.  **Giám sát:** Lựa chọn chế độ hiển thị phù hợp tại ô View Mode để theo dõi dòng tin trôi song song giữa UART và CAN.

## Cấu Hình Luồng UDP
Dữ liệu telemetry của cụm cảm biến lùi (`CAN ID: 0x300`) được giải mã và chuyển tiếp trực tiếp đến cổng mạng nội bộ:
*   **Địa chỉ đích:** `127.0.0.1` (Localhost)
*   **Cổng tiếp nhận:** `5005`
*   **Cấu trúc gói tin:**
    *   Header: `0xAA55` (Magic Word - 2 bytes)
    *   Message ID: `0x01` (1 byte)
    *   Data Length: `0x08` (1 byte)
    *   Payload: 8 bytes nhị phân chứa trạng thái Gear, khoảng cách 6 mắt cảm biến và mã lỗi hệ thống.