import socket
import struct
import threading
import time

class UdpCanAdapter:
    def __init__(self, tx_ip="127.0.0.1", tx_port=5007, rx_ip="127.0.0.1", rx_port=5006):
        self.tx_addr = (tx_ip, tx_port)
        self.rx_addr = (rx_ip, rx_port)

        # 1. Tách riêng Socket TX và Socket RX để không bao giờ ảnh hưởng chéo
        self.tx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._disable_win_connreset(self.tx_sock)

        self.rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.rx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._disable_win_connreset(self.rx_sock)
        self.rx_sock.bind(self.rx_addr)
        self.rx_sock.settimeout(0.02)

        # 2. Socket chuyên dụng để dò trạng thái cổng 5007 của Gateway (Probe)
        self.probe_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.probe_sock.settimeout(0.005)

        self.recv_buffer = []
        self.lock = threading.Lock()
        self.running = True

        # Biến giám sát trạng thái đường truyền thời gian thực
        self.gateway_port_open = False
        self.last_rx_time = 0.0
        self.last_uds_timeout = False
        self._frame_timestamps = []

        self.rx_thread = threading.Thread(target=self._recv_loop, daemon=True)
        self.rx_thread.start()

        self.probe_thread = threading.Thread(target=self._probe_loop, daemon=True)
        self.probe_thread.start()

    @staticmethod
    def _disable_win_connreset(sock):
        """Tắt lỗi WSAECONNRESET (WinError 10054) đặc trưng của UDP Windows"""
        if hasattr(socket, "SIO_UDP_CONNRESET"):
            try:
                sock.ioctl(socket.SIO_UDP_CONNRESET, False)
            except Exception:
                pass

    def send_frame(self, can_id: int, data: bytearray):
        packet = struct.pack(">H B", can_id, len(data)) + bytes(data)
        try:
            self.tx_sock.sendto(packet, self.tx_addr)
        except OSError:
            pass

    def clear_buffer(self):
        """Xóa sạch các frame tồn đọng trước khi bắt đầu phiên UDS mới"""
        with self.lock:
            self.recv_buffer.clear()

    def _recv_loop(self):
        while self.running:
            try:
                data, _ = self.rx_sock.recvfrom(1024)
                if len(data) >= 3:
                    can_id, dlc = struct.unpack(">H B", data[:3])
                    payload = data[3 : 3 + dlc]

                    now = time.time()
                    self.last_rx_time = now
                    self.gateway_port_open = True
                    self._frame_timestamps.append(now)

                    # Chỉ đẩy gói UDS (khác 0x300) vào buffer cho CanTp xử lý
                    if can_id != 0x300:
                        self.last_uds_timeout = False
                        with self.lock:
                            self.recv_buffer.append({'id': can_id, 'data': payload})
            except socket.timeout:
                continue
            except (ConnectionResetError, OSError):
                # Tuyệt đối KHÔNG break khi gặp lỗi ICMP/OS, giữ luồng luôn sống
                time.sleep(0.01)
                continue
            except Exception:
                time.sleep(0.01)
                continue

    def _probe_loop(self):
        """Định kỳ mỗi 400ms dò xem cổng 5007 của Gateway có đang mở hay đã bị Stop CAN"""
        while self.running:
            try:
                # Gửi gói 1-byte (0x00): Gateway sẽ bỏ qua vì len < 3,
                # nhưng nếu Gateway đóng cổng 5007 thì Windows sẽ báo ConnectionResetError ngay!
                self.probe_sock.sendto(b"\x00", self.tx_addr)
                try:
                    self.probe_sock.recvfrom(16)
                except socket.timeout:
                    # Không bị ICMP từ chối -> Cổng 5007 của Gateway ĐANG MỞ!
                    self.gateway_port_open = True
                except ConnectionResetError:
                    # Bị ICMP Port Unreachable -> Gateway đã tắt hoặc Stop CAN!
                    self.gateway_port_open = False
            except OSError:
                self.gateway_port_open = False

            time.sleep(0.4)

    def get_link_health(self):
        """
        Trả về tuple: (state_code, fps, last_rx_ago)
        state_code:
          0 = RED    (Gateway CAN Stopped / Port 5007 Closed)
          1 = YELLOW (Gateway Port 5007 Open, nhưng đang Idle hoặc vừa bị Timeout)
          2 = GREEN  (Đang có lưu lượng CAN / ECU phản hồi tốt)
        """
        now = time.time()
        # Giữ lại các frame trong vòng 1.0 giây gần nhất để tính FPS
        self._frame_timestamps = [t for t in self._frame_timestamps if (now - t) <= 1.0]
        fps = len(self._frame_timestamps)
        ago = (now - self.last_rx_time) if self.last_rx_time > 0 else 999.0

        if not self.gateway_port_open:
            return 0, 0, ago
        if self.last_uds_timeout or (ago > 2.5 and fps == 0):
            return 1, fps, ago
        return 2, fps, ago

    def recv_frame(self):
        with self.lock:
            if self.recv_buffer:
                return self.recv_buffer.pop(0)
        return None

    def close(self):
        self.running = False
        self.rx_thread.join(timeout=0.5)
        for s in (self.tx_sock, self.rx_sock, self.probe_sock):
            try:
                s.close()
            except Exception:
                pass