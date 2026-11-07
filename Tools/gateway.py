import sys
import socket
import struct
import time
import serial
import serial.tools.list_ports
import can
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QGroupBox, QLabel, QComboBox,
                             QPushButton, QTextEdit, QSplitter, QFrame)
from PyQt5.QtCore import QThread, pyqtSignal, Qt
from PyQt5.QtGui import QTextCursor, QTextOption

# --- CẤU HÌNH ĐÍCH UDP CHO QT DASHBOARD (COM) ---
QT_IP = "127.0.0.1"
QT_PORT = 5005

# --- CẤU HÌNH ĐÍCH UDP CHO UDS TESTER (TP) ---
TESTER_IP = "127.0.0.1"
TESTER_PORT = 5006

# --- CẤU HÌNH GATEWAY LẮNG NGHE LỆNH UDS TỪ TESTER ---
GATEWAY_RECV_IP = "127.0.0.1"
GATEWAY_RECV_PORT = 5007

# --- DANH SÁCH 3 CHẾ ĐỘ HIỂN THỊ & BỘ LỌC ---
VIEW_MODE_ALL_1LINE = "1. All Frames (1-Line)"
VIEW_MODE_COM_ONLY  = "2. COM Filter (0x300 Detail)"
VIEW_MODE_CANTP_UDS = "3. CanTp / UDS Filter (0x7Ex)"

# --- BẢNG TRA CỨU TÊN DỊCH VỤ UDS & NRC PHỤC VỤ GIẢI MÃ LOG ---
UDS_SID_NAMES = {
    0x10: "SessionCtrl",
    0x11: "ECUReset",
    0x14: "ClearDTC",
    0x19: "ReadDTC",
    0x22: "ReadDID",
    0x27: "SecurityAccess",
    0x2E: "WriteDID",
    0x31: "RoutineCtrl",
    0x3E: "TesterPresent",
}

UDS_NRC_NAMES = {
    0x10: "GeneralReject",
    0x11: "ServiceNotSupported",
    0x12: "SubFuncNotSupported",
    0x13: "InvalidLengthOrFormat",
    0x22: "ConditionsNotCorrected",
    0x31: "RequestOutOfRange",
    0x33: "SecurityAccessDenied",
    0x35: "InvalidKey",
    0x36: "ExceedNumOfAttempts",
    0x72: "GeneralProgrammingFailure",
    0x7F: "ServiceNotSupportedInActiveSession",
}

# --- DARK THEME STYLESHEET ---
DARK_STYLESHEET = """
QMainWindow, QWidget { background-color: #18181B; color: #E4E4E7; font-family: 'Segoe UI', sans-serif; font-size: 12px; }
QFrame#TopBar { background-color: #1E1E24; border: 1px solid #27272A; border-radius: 6px; }
QGroupBox { border: 1px solid #27272A; border-radius: 6px; margin-top: 8px; padding-top: 12px; padding-bottom: 6px; padding-left: 8px; padding-right: 8px; background-color: #1E1E24; font-weight: bold; color: #A1A1AA; }
QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
QComboBox { background-color: #27272A; border: 1px solid #3F3F46; border-radius: 4px; padding: 3px 8px; color: #F4F4F5; }
QComboBox:disabled { background-color: #18181B; color: #52525B; border-color: #27272A; }
QComboBox::drop-down { border: none; }
QPushButton { background-color: #27272A; border: 1px solid #3F3F46; border-radius: 4px; padding: 4px 12px; color: #F4F4F5; font-weight: 600; }
QPushButton:hover { background-color: #3F3F46; border-color: #52525B; }
QPushButton:pressed { background-color: #18181B; }
QPushButton#ScanBtn { background-color: #2563EB; border: 1px solid #3B82F6; color: #FFFFFF; }
QPushButton#ScanBtn:hover { background-color: #1D4ED8; }
QSplitter::handle { background-color: #27272A; width: 4px; }
QSplitter::handle:hover { background-color: #3B82F6; }
"""


class UartWorker(QThread):
    log_signal = pyqtSignal(str)
    status_signal = pyqtSignal(bool)
    hw_lost_signal = pyqtSignal()

    def __init__(self, port, baudrate):
        super().__init__()
        self.port = port
        self.baudrate = baudrate
        self.running = True
        self.serial_conn = None

    def run(self):
        try:
            self.serial_conn = serial.Serial(self.port, self.baudrate, timeout=0.05)
            self.status_signal.emit(True)
            self.log_signal.emit(f"[*] Connected to {self.port} @ {self.baudrate} bps")
            
            while self.running:
                line = self.serial_conn.readline()
                if line:
                    try:
                        decoded = line.decode("utf-8", errors="ignore").strip()
                        if decoded:
                            self.log_signal.emit(f"> {decoded}")
                    except Exception:
                        pass
        except Exception as e:
            self.running = False
            self.status_signal.emit(False)
            self.log_signal.emit(f"[!] UART Hardware Disconnected / Error: {e}")
            self.hw_lost_signal.emit()

    def stop(self):
        self.running = False
        if self.serial_conn and self.serial_conn.is_open:
            try:
                self.serial_conn.close()
            except Exception:
                pass
        self.status_signal.emit(False)
        self.log_signal.emit("[*] UART Disconnected.")


class CanWorker(QThread):
    log_signal = pyqtSignal(str)
    status_signal = pyqtSignal(int)
    hw_lost_signal = pyqtSignal()

    def __init__(self, interface, channel, serial_baud, can_bitrate, view_mode=VIEW_MODE_ALL_1LINE):
        super().__init__()
        self.interface = interface
        self.channel = channel
        self.serial_baud = serial_baud
        self.can_bitrate = can_bitrate
        self.view_mode = view_mode
        self.running = True
        self.bus = None
        self.ser = None

        # Khởi tạo UDP Socket TX
        self.udp_tx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # Khởi tạo UDP Socket RX an toàn
        self.udp_rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_rx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        if hasattr(socket, "SIO_UDP_CONNRESET"):
            try:
                self.udp_rx_sock.ioctl(socket.SIO_UDP_CONNRESET, False)
            except Exception:
                pass
        try:
            self.udp_rx_sock.bind((GATEWAY_RECV_IP, GATEWAY_RECV_PORT))
            self.udp_rx_sock.settimeout(0.01)
        except OSError:
            pass

    def set_view_mode(self, mode):
        self.view_mode = mode

    @staticmethod
    def _decode_uds_summary(uds_bytes: bytes) -> str:
        """Giải mã nhanh ý nghĩa gói UDS từ byte đầu tiên (SID / Positive Response / NRC)"""
        if not uds_bytes:
            return "Empty UDS"
        sid = uds_bytes[0]

        # 1. Negative Response (0x7F)
        if sid == 0x7F and len(uds_bytes) >= 3:
            req_sid = uds_bytes[1]
            nrc = uds_bytes[2]
            sid_name = UDS_SID_NAMES.get(req_sid, f"0x{req_sid:02X}")
            nrc_name = UDS_NRC_NAMES.get(nrc, f"0x{nrc:02X}")
            return f"NRC 0x{nrc:02X} ({nrc_name}) on {sid_name}"

        # 2. Request SID
        if sid in UDS_SID_NAMES:
            name = UDS_SID_NAMES[sid]
            if sid in (0x22, 0x2E) and len(uds_bytes) >= 3:
                did = (uds_bytes[1] << 8) | uds_bytes[2]
                return f"REQ {name}(0x{sid:02X}) DID=0x{did:04X}"
            elif len(uds_bytes) >= 2:
                return f"REQ {name}(0x{sid:02X}) Sub=0x{uds_bytes[1]:02X}"
            return f"REQ {name}(0x{sid:02X})"

        # 3. Positive Response (SID + 0x40)
        orig_sid = sid - 0x40
        if orig_sid in UDS_SID_NAMES:
            name = UDS_SID_NAMES[orig_sid]
            if orig_sid in (0x22, 0x2E) and len(uds_bytes) >= 3:
                did = (uds_bytes[1] << 8) | uds_bytes[2]
                return f"POS_RSP {name}(0x{sid:02X}) DID=0x{did:04X}"
            elif len(uds_bytes) >= 2:
                return f"POS_RSP {name}(0x{sid:02X}) Sub=0x{uds_bytes[1]:02X}"
            return f"POS_RSP {name}(0x{sid:02X})"

        return f"UDS Raw (0x{sid:02X})"

    def _decode_cantp_short(self, payload: bytes) -> str:
        """Giải mã 1 dòng ngắn gọn cho khung CanTp (Dùng ở Mode 1)"""
        if not payload:
            return "Empty"
        pci_type = (payload[0] >> 4) & 0x0F
        if pci_type == 0x0:
            sf_len = payload[0] & 0x0F
            uds_info = self._decode_uds_summary(payload[1 : 1 + sf_len])
            return f"[SF Len={sf_len}B] {uds_info}"
        elif pci_type == 0x1 and len(payload) >= 2:
            ff_len = ((payload[0] & 0x0F) << 8) | payload[1]
            uds_info = self._decode_uds_summary(payload[2:8])
            return f"[FF Total={ff_len}B] {uds_info}"
        elif pci_type == 0x2:
            sn = payload[0] & 0x0F
            chunk_hex = " ".join([f"{b:02X}" for b in payload[1:]])
            return f"[CF SN={sn:X}] Data=[{chunk_hex}]"
        elif pci_type == 0x3 and len(payload) >= 3:
            fs = payload[0] & 0x0F
            fs_str = {0: "CTS", 1: "WAIT", 2: "OVFLW"}.get(fs, f"?({fs})")
            return f"[FC {fs_str}] BS={payload[1]} STmin={payload[2]}ms"
        return "CanTp Unknown PCI"

    def _format_cantp_detailed(self, can_id: int, payload: bytes, tag: str) -> str:
        """Giải mã chi tiết chuẩn ISO 15765-2 CanTp & ISO 14229 UDS (Dùng ở Mode 3)"""
        hex_str = " ".join([f"{b:02X}" for b in payload])
        if can_id == 0x7DF:
            dir_str = "Tester -> Broadcast (Func)"
        elif can_id == 0x7E0:
            dir_str = "Tester -> ECU (Phys Req)"
        elif can_id == 0x7E8:
            dir_str = "ECU -> Tester (Phys Res)"
        else:
            dir_str = "UDS Frame"

        header_line = f"[{tag}] 0x{can_id:03X} ({dir_str}) | DLC:{len(payload)} | HEX: {hex_str}"
        if not payload:
            return header_line

        pci_type = (payload[0] >> 4) & 0x0F
        if pci_type == 0x0:
            sf_len = payload[0] & 0x0F
            uds_slice = payload[1 : 1 + sf_len]
            uds_hex = " ".join([f"{b:02X}" for b in uds_slice])
            uds_desc = self._decode_uds_summary(uds_slice)
            detail_line = f"  └─> [CanTp: SF (Single Frame) | Len={sf_len}B] -> {uds_desc} | Payload: [{uds_hex}]"
        elif pci_type == 0x1 and len(payload) >= 2:
            ff_len = ((payload[0] & 0x0F) << 8) | payload[1]
            uds_slice = payload[2:8]
            uds_hex = " ".join([f"{b:02X}" for b in uds_slice])
            uds_desc = self._decode_uds_summary(uds_slice)
            detail_line = f"  └─> [CanTp: FF (First Frame)  | TotalLen={ff_len}B] -> {uds_desc} | First6B: [{uds_hex}]"
        elif pci_type == 0x2:
            sn = payload[0] & 0x0F
            chunk_hex = " ".join([f"{b:02X}" for b in payload[1:]])
            detail_line = f"  └─> [CanTp: CF (Consecutive)  | SeqNum={sn} (0x2{sn:X})] -> Chunk7B: [{chunk_hex}]"
        elif pci_type == 0x3 and len(payload) >= 3:
            fs = payload[0] & 0x0F
            fs_str = {0: "0 (ClearToSend)", 1: "1 (Wait)", 2: "2 (Overflow)"}.get(fs, str(fs))
            bs = payload[1]
            stmin = payload[2]
            detail_line = f"  └─> [CanTp: FC (Flow Control) | Status={fs_str} | BlockSize={bs} | STmin={stmin}ms]"
        else:
            detail_line = "  └─> [CanTp: Unknown PCI Frame]"

        return f"{header_line}\n{detail_line}"

    def format_frame_log(self, can_id, payload, tag="HW_CAN"):
        """
        Định dạng và lọc hiển thị theo 3 chế độ.
        Trả về None nếu khung CAN bị chặn bởi bộ lọc hiển thị hiện tại.
        """
        is_com_frame   = can_id in (0x300, 0x200)
        is_cantp_frame = can_id in (0x7DF, 0x7E0, 0x7E8)

        # --- CHẾ ĐỘ 2: CHỈ LỌC VÀ HIỂN THỊ KỸ KHUNG COM (0x300 / 0x200) ---
        if self.view_mode == VIEW_MODE_COM_ONLY:
            if not is_com_frame:
                return None

            hex_str = " ".join([f"{b:02X}" for b in payload])
            bit_str = " ".join([f"{b:08b}" for b in payload])

            if can_id == 0x300 and len(payload) >= 8:
                gear = {0: 'P', 1: 'R', 2: 'N', 3: 'D'}.get(payload[0], f"?({payload[0]})")
                cm_dists = [b * 2 for b in payload[1:7]]
                err = payload[7]
                sens_err_bits = f"{err & 0x3F:06b}b"
                gear_err_code = (err >> 6) & 0x03
                gear_err_str = {0: "OK", 1: "OutRange", 2: "Glitch", 3: "Lost"}.get(gear_err_code, "?")

                return (
                    f"[{tag}] 0x{can_id:03X} (Radar COM) | DLC:{len(payload)} | HEX: {hex_str}\n"
                    f"  ├─> Gear: {gear} | Front(cm): [FL:{cm_dists[0]:3d}, FC:{cm_dists[1]:3d}, FR:{cm_dists[2]:3d}] "
                    f"| Rear(cm): [RL:{cm_dists[3]:3d}, RC:{cm_dists[4]:3d}, RR:{cm_dists[5]:3d}]\n"
                    f"  └─> ErrMask: 0x{err:02X} (Sens6:{sens_err_bits}, GearFault:{gear_err_str}) | BIT: {bit_str}"
                )
            return f"[{tag}] 0x{can_id:03X} (Vehicle COM) | DLC:{len(payload)} | HEX: {hex_str}\n  └─> BIT: {bit_str}"

        # --- CHẾ ĐỘ 3: CHỈ LỌC VÀ HIỂN THỊ KỸ KHUNG CANTP / UDS (0x7DF, 0x7E0, 0x7E8) ---
        elif self.view_mode == VIEW_MODE_CANTP_UDS:
            if not is_cantp_frame:
                return None
            return self._format_cantp_detailed(can_id, payload, tag)

        # --- CHẾ ĐỘ 1 (MẶC ĐỊNH): HIỂN THỊ TẤT CẢ CÁC KHUNG TRÊN 1 DÒNG NHANH ---
        hex_str = " ".join([f"{b:02X}" for b in payload])
        if can_id == 0x300 and len(payload) >= 8:
            gear = {0: 'P', 1: 'R', 2: 'N', 3: 'D'}.get(payload[0], f"?({payload[0]})")
            cm_dists = [b * 2 for b in payload[1:7]]
            err = payload[7]
            return f"[{tag}] 0x{can_id:03X} | HEX: {hex_str} | COM -> Gear:{gear} Dist(cm):{cm_dists} Err:0x{err:02X}"
        elif is_cantp_frame:
            tp_summary = self._decode_cantp_short(payload)
            return f"[{tag}] 0x{can_id:03X} | HEX: {hex_str} | TP  -> {tp_summary}"
        else:
            return f"[{tag}] 0x{can_id:03X} | DLC:{len(payload)} | HEX: {hex_str}"

    def emit_frame_if_matched(self, can_id, payload, tag="HW_CAN"):
        log_text = self.format_frame_log(can_id, payload, tag)
        if log_text is not None:
            self.log_signal.emit(log_text)

    def forward_udp_if_needed(self, can_id, payload):
        """Định tuyến UDP chạy ngầm độc lập hoàn toàn với bộ lọc hiển thị GUI"""
        try:
            if can_id == 0x300:
                # 1. Gửi sang Qt Dashboard (Port 5005)
                packet = struct.pack(">HBB", 0xAA55, 0x01, len(payload)) + bytes(payload)
                self.udp_tx_sock.sendto(packet, (QT_IP, QT_PORT))
                # 2. Gửi nhịp sống sang Tester (Port 5006) để đèn Link Monitor luôn XANH
                hb_pkt = struct.pack(">H B", can_id, len(payload)) + bytes(payload)
                self.udp_tx_sock.sendto(hb_pkt, (TESTER_IP, TESTER_PORT))
            elif can_id == 0x7E8:
                # Bản tin UDS Response -> Gửi sang Tester (Port 5006)
                packet = struct.pack(">H B", can_id, len(payload)) + bytes(payload)
                self.udp_tx_sock.sendto(packet, (TESTER_IP, TESTER_PORT))
        except Exception:
            pass

    def process_udp_rx(self):
        try:
            data, _ = self.udp_rx_sock.recvfrom(1024)
            # Các gói thăm dò 1-byte (b'\x00') từ Tester sẽ tự động bị bỏ qua vì len < 3
            if len(data) >= 3:
                can_id, dlc = struct.unpack(">H B", data[:3])
                payload = data[3 : 3 + dlc]
                
                if self.interface == "waveshare" and self.ser:
                    is_ext = can_id > 0x7FF
                    ctrl = (0xE0 if is_ext else 0xC0) | (dlc & 0x0F)
                    if is_ext:
                        id_bytes = list(can_id.to_bytes(4, byteorder='little'))
                    else:
                        id_bytes = [can_id & 0xFF, (can_id >> 8) & 0xFF]
                    
                    frame = bytes([0xAA, ctrl] + id_bytes + list(payload) + [0x55])
                    self.ser.write(frame)
                    self.emit_frame_if_matched(can_id, payload, "UDP_TX")
                
                elif self.bus:
                    msg = can.Message(arbitration_id=can_id, data=payload, is_extended_id=(can_id > 0x7FF))
                    self.bus.send(msg)
                    self.emit_frame_if_matched(can_id, payload, "UDP_TX")
        except socket.timeout:
            pass
        except Exception:
            pass

    def send_waveshare_init(self, ser):
        rate_map = {
            1000000: 0x01, 800000: 0x02, 500000: 0x03, 400000: 0x04,
            250000:  0x05, 200000: 0x06, 125000: 0x07, 100000: 0x08, 50000: 0x09
        }
        speed_code = rate_map.get(self.can_bitrate, 0x03)
        body = [
            0x12, speed_code, 0x01,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ]
        checksum = sum(body) & 0xFF
        ser.write(bytes([0xAA, 0x55] + body + [checksum]))
        time.sleep(0.02)

    def run_waveshare_native(self):
        try:
            self.ser = serial.Serial(self.channel, self.serial_baud, timeout=0.005)
            self.send_waveshare_init(self.ser)
            self.status_signal.emit(1)
            self.log_signal.emit(
                f"[*] Waveshare Stream: {self.channel} @ {self.serial_baud} bps | CAN: {self.can_bitrate} bps"
            )
        except Exception as e:
            self.status_signal.emit(0)
            self.log_signal.emit(f"[!] Không mở được cổng {self.channel}: {e}")
            self.hw_lost_signal.emit()
            return

        buf = bytearray()
        while self.running:
            self.process_udp_rx()
            
            try:
                waiting = self.ser.in_waiting
                chunk = self.ser.read(waiting if waiting > 0 else 1)
                if chunk:
                    buf.extend(chunk)
                    while len(buf) >= 5:
                        if buf[0] != 0xAA:
                            del buf[0]
                            continue

                        ctrl = buf[1]
                        if ctrl == 0x55:
                            if len(buf) < 20:
                                break
                            del buf[:20]
                            continue

                        if (ctrl & 0xC0) == 0xC0:
                            is_ext = bool(ctrl & 0x20)
                            dlc = ctrl & 0x0F
                            if dlc > 8:
                                del buf[0]
                                continue
                            id_len = 4 if is_ext else 2
                            frame_len = 2 + id_len + dlc + 1

                            if len(buf) < frame_len:
                                break

                            if buf[frame_len - 1] == 0x55:
                                if is_ext:
                                    can_id = int.from_bytes(buf[2:6], byteorder='little')
                                    payload = bytes(buf[6 : 6 + dlc])
                                else:
                                    can_id = int.from_bytes(buf[2:4], byteorder='little')
                                    payload = bytes(buf[4 : 4 + dlc])

                                self.emit_frame_if_matched(can_id, payload, "HW_CAN")
                                self.forward_udp_if_needed(can_id, payload)
                                del buf[:frame_len]
                            else:
                                del buf[0]
                        else:
                            if len(buf) >= 20 and buf[1] in (0x01, 0x02):
                                dlc = min(buf[4], 8)
                                can_id = int.from_bytes(buf[5:9], byteorder='little')
                                payload = bytes(buf[9 : 9 + dlc])
                                self.emit_frame_if_matched(can_id, payload, "HW_CAN")
                                self.forward_udp_if_needed(can_id, payload)
                                del buf[:20]
                            else:
                                del buf[0]
            except Exception as e:
                self.running = False
                self.status_signal.emit(0)
                self.log_signal.emit(f"[!] Mất kết nối phần cứng Waveshare ({self.channel}): {e}")
                self.hw_lost_signal.emit()
                break

    def run(self):
        if self.interface == "waveshare":
            self.run_waveshare_native()
            return

        if self.interface == "mock":
            self.status_signal.emit(2)
            self.log_signal.emit(f"[*] MOCK Generator Active -> UDP Qt:{QT_PORT} | Tester:{TESTER_PORT}")
            mock_tick = 0
            while self.running:
                self.process_udp_rx()
                mock_tick = (mock_tick + 1) % 50
                mock_payload = bytes([0, 15 + (mock_tick % 20), 13, 35, 62, 53, 66, 0x00])
                self.forward_udp_if_needed(0x300, mock_payload)
                self.emit_frame_if_matched(0x300, mock_payload, "MOCK_UDP")
                time.sleep(0.05)
            return

        try:
            kwargs = {"channel": self.channel, "interface": self.interface, "bitrate": self.can_bitrate}
            if self.interface in ("slcan", "seeedstudio"):
                kwargs["ttyBaudrate"] = self.serial_baud

            self.bus = can.interface.Bus(**kwargs)
            self.status_signal.emit(1)
            self.log_signal.emit(f"[*] HW Online: {self.interface} ({self.channel}) | CAN: {self.can_bitrate}")
            while self.running:
                self.process_udp_rx()
                msg = self.bus.recv(timeout=0.01)
                if msg:
                    self.emit_frame_if_matched(msg.arbitration_id, msg.data, "HW_CAN")
                    self.forward_udp_if_needed(msg.arbitration_id, msg.data)
        except Exception as e:
            self.running = False
            self.status_signal.emit(0)
            self.log_signal.emit(f"[!] Lỗi / Ngắt kết nối {self.interface}: {e}")
            self.hw_lost_signal.emit()

    def stop(self):
        self.running = False
        try:
            self.udp_tx_sock.close()
            self.udp_rx_sock.close()
        except Exception:
            pass
        if self.ser and self.ser.is_open:
            try:
                self.ser.close()
            except Exception:
                pass
        if self.bus:
            try:
                self.bus.shutdown()
            except Exception:
                pass
        self.status_signal.emit(0)
        self.log_signal.emit("[*] CAN Stream Stopped.")


class GatewayApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Automotive Communication Gateway - Physical & Mock Abstraction Layer")
        self.resize(1600, 900)
        self.setStyleSheet(DARK_STYLESHEET)
        
        self.uart_thread = None
        self.can_thread = None
        
        self.init_ui()
        self.scan_all_hardware()

    def init_ui(self):
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        root_layout = QVBoxLayout(central_widget)
        root_layout.setContentsMargins(8, 8, 8, 8)
        root_layout.setSpacing(6)

        top_bar = QFrame()
        top_bar.setObjectName("TopBar")
        top_bar.setFixedHeight(40)
        top_layout = QHBoxLayout(top_bar)
        top_layout.setContentsMargins(12, 4, 12, 4)
        top_layout.setSpacing(10)

        lbl_title = QLabel("⚡ HARDWARE GATEWAY MANAGER")
        lbl_title.setStyleSheet("color: #F4F4F5; font-weight: bold; font-size: 13px;")

        self.lbl_hw_summary = QLabel("Active Ports: Scanning...")
        self.lbl_hw_summary.setStyleSheet("color: #A1A1AA; font-family: 'Consolas', monospace;")

        self.btn_global_scan = QPushButton("↻ Scan Hardware (USB / COM)")
        self.btn_global_scan.setObjectName("ScanBtn")
        self.btn_global_scan.setCursor(Qt.PointingHandCursor)
        self.btn_global_scan.clicked.connect(self.scan_all_hardware)

        top_layout.addWidget(lbl_title)
        top_layout.addSpacing(12)
        top_layout.addWidget(self.lbl_hw_summary)
        top_layout.addStretch()
        top_layout.addWidget(self.btn_global_scan)
        root_layout.addWidget(top_bar)

        self.main_splitter = QSplitter(Qt.Horizontal)

        # UART Panel
        uart_panel = QWidget()
        uart_panel_layout = QVBoxLayout(uart_panel)
        uart_panel_layout.setContentsMargins(0, 0, 4, 0)
        uart_panel_layout.setSpacing(6)

        uart_group = QGroupBox("UART Stream (Real Hardware)")
        uart_group.setFixedHeight(96)
        uart_vbox = QVBoxLayout(uart_group)
        uart_vbox.setContentsMargins(8, 4, 8, 6)
        uart_vbox.setSpacing(6)

        uart_row1 = QHBoxLayout()
        self.uart_port_combo = QComboBox()
        self.uart_port_combo.setMinimumWidth(85)
        self.uart_baud_combo = QComboBox()
        self.uart_baud_combo.addItems(["9600", "115200", "500000", "1000000", "2000000"])
        self.uart_baud_combo.setCurrentText("115200")
        self.uart_baud_combo.setMinimumWidth(90)

        uart_row1.addWidget(QLabel("Port:"))
        uart_row1.addWidget(self.uart_port_combo, 1)
        uart_row1.addSpacing(6)
        uart_row1.addWidget(QLabel("Baud:"))
        uart_row1.addWidget(self.uart_baud_combo, 1)

        uart_row2 = QHBoxLayout()
        self.btn_uart_conn = QPushButton("Connect")
        self.btn_uart_conn.setMinimumWidth(85)
        self.btn_uart_conn.clicked.connect(self.toggle_uart)
        self.btn_uart_clear = QPushButton("Clear")
        self.btn_uart_clear.clicked.connect(lambda: self.uart_log.clear())
        self.lbl_uart_status = QLabel("● Disconnected")
        self.lbl_uart_status.setStyleSheet("color: #EF4444; font-weight: bold;")

        uart_row2.addWidget(self.btn_uart_conn)
        uart_row2.addWidget(self.btn_uart_clear)
        uart_row2.addStretch()
        uart_row2.addWidget(self.lbl_uart_status)

        uart_vbox.addLayout(uart_row1)
        uart_vbox.addLayout(uart_row2)

        self.uart_log = QTextEdit()
        self.uart_log.setReadOnly(True)
        self.uart_log.setLineWrapMode(QTextEdit.WidgetWidth)
        self.uart_log.setWordWrapMode(QTextOption.WrapAnywhere)
        self.uart_log.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.uart_log.setStyleSheet(
            "background-color: #09090B; color: #22C55E; border: 1px solid #27272A; "
            "border-radius: 6px; font-family: 'Consolas', monospace; font-size: 12px; padding: 6px;"
        )

        uart_panel_layout.addWidget(uart_group)
        uart_panel_layout.addWidget(self.uart_log)

        # CAN Panel
        can_panel = QWidget()
        can_panel_layout = QVBoxLayout(can_panel)
        can_panel_layout.setContentsMargins(4, 0, 0, 0)
        can_panel_layout.setSpacing(6)

        can_group = QGroupBox("CAN Bus Stream (ISO 11898 / UDP Router)")
        can_group.setFixedHeight(96)
        can_vbox = QVBoxLayout(can_group)
        can_vbox.setContentsMargins(8, 4, 8, 6)
        can_vbox.setSpacing(6)

        can_row1 = QHBoxLayout()
        self.can_iface_combo = QComboBox()
        self.can_iface_combo.setMinimumWidth(105)
        self.can_iface_combo.currentTextChanged.connect(self.on_can_interface_changed)

        self.can_chan_combo = QComboBox()
        self.can_chan_combo.setEditable(True)
        self.can_chan_combo.setMinimumWidth(90)

        self.can_sbaud_combo = QComboBox()
        self.can_sbaud_combo.addItems(["2000000", "1000000", "115200"])
        self.can_sbaud_combo.setCurrentText("2000000")
        self.can_sbaud_combo.setMinimumWidth(90)

        self.can_bitrate_combo = QComboBox()
        self.can_bitrate_combo.addItems(["125000", "250000", "500000", "1000000"])
        self.can_bitrate_combo.setCurrentText("500000")
        self.can_bitrate_combo.setMinimumWidth(85)

        self.can_view_combo = QComboBox()
        self.can_view_combo.addItems([
            VIEW_MODE_ALL_1LINE,
            VIEW_MODE_COM_ONLY,
            VIEW_MODE_CANTP_UDS,
        ])
        self.can_view_combo.setCurrentText(VIEW_MODE_ALL_1LINE)
        self.can_view_combo.setMinimumWidth(210)
        self.can_view_combo.currentTextChanged.connect(self.on_view_mode_changed)

        can_row1.addWidget(QLabel("Iface:"))
        can_row1.addWidget(self.can_iface_combo)
        can_row1.addWidget(QLabel("Channel:"))
        can_row1.addWidget(self.can_chan_combo)
        can_row1.addWidget(QLabel("USB Baud:"))
        can_row1.addWidget(self.can_sbaud_combo)
        can_row1.addWidget(QLabel("CAN Rate:"))
        can_row1.addWidget(self.can_bitrate_combo)
        can_row1.addWidget(QLabel("Filter / Mode:"))
        can_row1.addWidget(self.can_view_combo)
        can_row1.addStretch()

        can_row2 = QHBoxLayout()
        self.btn_can_conn = QPushButton("Start CAN")
        self.btn_can_conn.setMinimumWidth(95)
        self.btn_can_conn.clicked.connect(self.toggle_can)
        self.btn_can_clear = QPushButton("Clear")
        self.btn_can_clear.setMinimumWidth(75)
        self.btn_can_clear.clicked.connect(lambda: self.can_log.clear())
        self.lbl_can_status = QLabel("● Stopped")
        self.lbl_can_status.setStyleSheet("color: #71717A; font-weight: bold;")

        can_row2.addWidget(self.btn_can_conn)
        can_row2.addWidget(self.btn_can_clear)
        can_row2.addStretch()
        can_row2.addWidget(self.lbl_can_status)

        can_vbox.addLayout(can_row1)
        can_vbox.addLayout(can_row2)

        self.can_log = QTextEdit()
        self.can_log.setReadOnly(True)
        self.can_log.setLineWrapMode(QTextEdit.WidgetWidth)
        self.can_log.setWordWrapMode(QTextOption.WrapAnywhere)
        self.can_log.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.can_log.setStyleSheet(
            "background-color: #09090B; color: #38BDF8; border: 1px solid #27272A; "
            "border-radius: 6px; font-family: 'Consolas', monospace; font-size: 12px; padding: 6px;"
        )

        can_panel_layout.addWidget(can_group)
        can_panel_layout.addWidget(self.can_log)

        self.main_splitter.addWidget(uart_panel)
        self.main_splitter.addWidget(can_panel)
        self.main_splitter.setStretchFactor(0, 1)
        self.main_splitter.setStretchFactor(1, 3)
        self.main_splitter.setSizes([400, 1200])

        root_layout.addWidget(self.main_splitter, 1)

    def on_view_mode_changed(self, mode):
        if self.can_thread and self.can_thread.isRunning():
            self.can_thread.set_view_mode(mode)
        self.append_log(self.can_log, f"\n[#] Switched Display Filter -> {mode}")

    def append_log(self, text_edit, text):
        doc = text_edit.document()
        if doc.blockCount() > 800:
            cursor = QTextCursor(doc)
            cursor.movePosition(QTextCursor.Start)
            cursor.movePosition(QTextCursor.Down, QTextCursor.KeepAnchor, 200)
            cursor.removeSelectedText()
        text_edit.append(text)
        text_edit.moveCursor(QTextCursor.End)

    def scan_all_hardware(self):
        ports_info = list(serial.tools.list_ports.comports())
        active_ports = [p.device for p in ports_info]

        if active_ports:
            ports_str = ", ".join(active_ports)
            self.lbl_hw_summary.setText(f"Active USB/COM ({len(active_ports)}): [{ports_str}]")
            self.lbl_hw_summary.setStyleSheet("color: #22C55E; font-family: 'Consolas', monospace; font-weight: bold;")
        else:
            self.lbl_hw_summary.setText("Active USB/COM (0): [None - Mock/Virtual Only]")
            self.lbl_hw_summary.setStyleSheet("color: #F59E0B; font-family: 'Consolas', monospace;")

        prev_uart = self.uart_port_combo.currentText()
        self.uart_port_combo.clear()
        if active_ports:
            self.uart_port_combo.addItems(active_ports)
            if prev_uart in active_ports:
                self.uart_port_combo.setCurrentText(prev_uart)
            elif "COM3" in active_ports:
                self.uart_port_combo.setCurrentText("COM3")

        prev_iface = self.can_iface_combo.currentText()
        self.can_iface_combo.blockSignals(True)
        self.can_iface_combo.clear()

        available_ifaces = []
        if active_ports:
            available_ifaces.extend(["waveshare", "slcan"])
        available_ifaces.extend(["mock", "virtual"])

        self.can_iface_combo.addItems(available_ifaces)
        if prev_iface in available_ifaces:
            self.can_iface_combo.setCurrentText(prev_iface)
        else:
            self.can_iface_combo.setCurrentIndex(0)
        self.can_iface_combo.blockSignals(False)

        self.on_can_interface_changed(self.can_iface_combo.currentText(), active_ports)

    def on_can_interface_changed(self, interface, pre_scanned_ports=None):
        if pre_scanned_ports is None:
            active_ports = [p.device for p in serial.tools.list_ports.comports()]
        else:
            active_ports = pre_scanned_ports

        prev_chan = self.can_chan_combo.currentText()
        self.can_chan_combo.clear()

        if interface in ("waveshare", "slcan"):
            if active_ports:
                self.can_chan_combo.addItems(active_ports)
                if prev_chan in active_ports:
                    self.can_chan_combo.setCurrentText(prev_chan)
                elif "COM4" in active_ports:
                    self.can_chan_combo.setCurrentText("COM4")
                elif len(active_ports) > 1 and self.uart_port_combo.currentText() == active_ports[0]:
                    self.can_chan_combo.setCurrentText(active_ports[1])
            self.can_sbaud_combo.setEnabled(True)
            self.can_sbaud_combo.setCurrentText("2000000" if interface == "waveshare" else "115200")
        elif interface == "mock":
            self.can_chan_combo.addItems(["UDP_5005"])
            self.can_sbaud_combo.setEnabled(False)
        elif interface == "virtual":
            self.can_chan_combo.addItems(["vbus0", "vbus1"])
            self.can_sbaud_combo.setEnabled(False)

    def on_hardware_lost(self):
        if self.uart_thread and not self.uart_thread.isRunning():
            self.btn_uart_conn.setText("Connect")
        if self.can_thread and not self.can_thread.isRunning():
            self.btn_can_conn.setText("Start CAN")
        self.scan_all_hardware()

    def toggle_uart(self):
        if self.uart_thread is None or not self.uart_thread.isRunning():
            port = self.uart_port_combo.currentText()
            if not port:
                self.append_log(self.uart_log, "[!] Không tìm thấy cổng COM thực tế nào.")
                return
            baud = int(self.uart_baud_combo.currentText())
            self.uart_thread = UartWorker(port, baud)
            self.uart_thread.log_signal.connect(lambda msg: self.append_log(self.uart_log, msg))
            self.uart_thread.status_signal.connect(self.update_uart_status)
            self.uart_thread.hw_lost_signal.connect(self.on_hardware_lost)
            self.uart_thread.start()
            self.btn_uart_conn.setText("Disconnect")
        else:
            self.uart_thread.stop()
            self.uart_thread.wait()
            self.btn_uart_conn.setText("Connect")

    def update_uart_status(self, connected):
        if connected:
            self.lbl_uart_status.setText("● Connected")
            self.lbl_uart_status.setStyleSheet("color: #22C55E; font-weight: bold;")
        else:
            self.lbl_uart_status.setText("● Disconnected")
            self.lbl_uart_status.setStyleSheet("color: #EF4444; font-weight: bold;")
            self.btn_uart_conn.setText("Connect")

    def toggle_can(self):
        if self.can_thread is None or not self.can_thread.isRunning():
            iface = self.can_iface_combo.currentText()
            chan = self.can_chan_combo.currentText()
            if not iface or not chan:
                self.append_log(self.can_log, "[!] Chưa có thiết bị hoặc kênh CAN hợp lệ.")
                return
            sbaud = int(self.can_sbaud_combo.currentText())
            bitrate = int(self.can_bitrate_combo.currentText())
            vmode = self.can_view_combo.currentText()
            self.can_thread = CanWorker(iface, chan, sbaud, bitrate, vmode)
            self.can_thread.log_signal.connect(lambda msg: self.append_log(self.can_log, msg))
            self.can_thread.status_signal.connect(self.update_can_status)
            self.can_thread.hw_lost_signal.connect(self.on_hardware_lost)
            self.can_thread.start()
            self.btn_can_conn.setText("Stop CAN")
        else:
            self.can_thread.stop()
            self.can_thread.wait()
            self.btn_can_conn.setText("Start CAN")

    def update_can_status(self, code):
        if code == 1:
            self.lbl_can_status.setText("● HW Online")
            self.lbl_can_status.setStyleSheet("color: #22C55E; font-weight: bold;")
        elif code == 2:
            self.lbl_can_status.setText("● Mock Streaming")
            self.lbl_can_status.setStyleSheet("color: #F59E0B; font-weight: bold;")
        else:
            self.lbl_can_status.setText("● Stopped")
            self.lbl_can_status.setStyleSheet("color: #71717A; font-weight: bold;")
            self.btn_can_conn.setText("Start CAN")

    def closeEvent(self, event):
        if self.uart_thread:
            self.uart_thread.stop()
            self.uart_thread.wait()
        if self.can_thread:
            self.can_thread.stop()
            self.can_thread.wait()
        event.accept()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = GatewayApp()
    window.showMaximized()
    sys.exit(app.exec_())