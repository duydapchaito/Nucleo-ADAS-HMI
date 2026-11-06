import sys
import socket
import struct
import time
import serial
import serial.tools.list_ports
import can
import threading # QUAN TRỌNG: Cần cho timer clear
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

    def __init__(self, interface, channel, serial_baud, can_bitrate, view_mode="1-Line Smooth"):
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

        # Khởi tạo UDP Socket RX an toàn (Không bị lỗi thiếu socket)
        self.udp_rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp_rx_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.udp_rx_sock.bind((GATEWAY_RECV_IP, GATEWAY_RECV_PORT))
            self.udp_rx_sock.settimeout(0.01)
        except OSError:
            pass

    def set_view_mode(self, mode):
        self.view_mode = mode

    def format_frame_log(self, can_id, payload, tag="HW_CAN"):
        hex_str = " ".join([f"{b:02X}" for b in payload])
        bit_str = " ".join([f"{b:08b}" for b in payload])
        
        gear = "?"
        sensors = []
        err = 0
        if can_id == 0x300 and len(payload) >= 8:
            gear = {0: 'P', 1: 'R', 2: 'N', 3: 'D'}.get(payload[0], f"?({payload[0]})")
            sensors = list(payload[1:7])
            err = payload[7]

        if self.view_mode == "1-Line Smooth":
            if can_id == 0x300 and len(payload) >= 8:
                return f"[{tag}] 0x{can_id:03X} | HEX: {hex_str} | Gear:{gear} Dist:{sensors} Err:0x{err:02X}"
            return f"[{tag}] 0x{can_id:03X} | DLC:{len(payload)} | HEX: {hex_str}"
        elif self.view_mode == "2-Line Compact":
            return (f"[{tag}] 0x{can_id:03X} | HEX: {hex_str} -> [Gear:{gear} | Dist:{sensors} | Err:0x{err:02X}]\n"
                    f"  BIT: {bit_str}")
        else:
            parsed_info = f"\n  └─> [Gear: {gear} | Dist: {sensors} | ErrMask: 0x{err:02X}]" if can_id == 0x300 else ""
            return (f"[{tag}] ID: 0x{can_id:03X} | DLC: {len(payload)} | HEX: {hex_str}\n"
                    f"  BIT: {bit_str}{parsed_info}")

    def forward_udp_if_needed(self, can_id, payload):
        try:
            if can_id == 0x300: # Bản tin COM -> Gửi sang Qt Dashboard
                packet = struct.pack(">HBB", 0xAA55, 0x01, len(payload)) + bytes(payload)
                self.udp_tx_sock.sendto(packet, (QT_IP, QT_PORT))
            elif can_id == 0x7E8: # Bản tin UDS Response -> Gửi sang Tester Script
                packet = struct.pack(">H B", can_id, len(payload)) + bytes(payload)
                self.udp_tx_sock.sendto(packet, (TESTER_IP, TESTER_PORT))
        except Exception:
            pass

    def process_udp_rx(self):
        try:
            data, _ = self.udp_rx_sock.recvfrom(1024)
            if len(data) >= 3:
                can_id, dlc = struct.unpack(">H B", data[:3])
                payload = data[3:3+dlc]
                
                if self.interface == "waveshare" and self.ser:
                    is_ext = can_id > 0x7FF
                    # Dành riêng cho Waveshare: KHÔNG CÓ CHECKSUM. 0xAA + Ctrl + ID + Payload + 0x55
                    ctrl = (0xE0 if is_ext else 0xC0) | (dlc & 0x0F)
                    if is_ext:
                        id_bytes = list(can_id.to_bytes(4, byteorder='little'))
                    else:
                        id_bytes = [can_id & 0xFF, (can_id >> 8) & 0xFF]
                    
                    frame = bytes([0xAA, ctrl] + id_bytes + list(payload) + [0x55])
                    self.ser.write(frame)
                    self.log_signal.emit(self.format_frame_log(can_id, payload, "UDP_TX"))
                
                elif self.bus:
                    msg = can.Message(arbitration_id=can_id, data=payload, is_extended_id=(can_id > 0x7FF))
                    self.bus.send(msg)
                    self.log_signal.emit(self.format_frame_log(can_id, payload, "UDP_TX"))
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
                f"[*] Waveshare Real-time Stream: {self.channel} @ {self.serial_baud} bps | CAN: {self.can_bitrate} bps"
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
                                    payload = bytes(buf[6:6 + dlc])
                                else:
                                    can_id = int.from_bytes(buf[2:4], byteorder='little')
                                    payload = bytes(buf[4:4 + dlc])

                                self.log_signal.emit(self.format_frame_log(can_id, payload, "HW_CAN"))
                                self.forward_udp_if_needed(can_id, payload)
                                del buf[:frame_len]
                            else:
                                del buf[0]
                        else:
                            if len(buf) >= 20 and buf[1] in (0x01, 0x02):
                                dlc = min(buf[4], 8)
                                can_id = int.from_bytes(buf[5:9], byteorder='little')
                                payload = bytes(buf[9:9 + dlc])
                                self.log_signal.emit(self.format_frame_log(can_id, payload, "HW_CAN"))
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
                self.log_signal.emit(self.format_frame_log(0x300, mock_payload, "MOCK_UDP"))
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
                    self.log_signal.emit(self.format_frame_log(msg.arbitration_id, msg.data, "HW_CAN"))
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


# --- GIỮ NGUYÊN PHẦN UI (GatewayApp) TỪ CODE CŨ CỦA BẠN ---
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

        can_group = QGroupBox("CAN Bus Stream (ISO 11898 / UDP Broadcast)")
        can_group.setFixedHeight(96)
        can_vbox = QVBoxLayout(can_group)
        can_vbox.setContentsMargins(8, 4, 8, 6)
        can_vbox.setSpacing(6)

        can_row1 = QHBoxLayout()
        self.can_iface_combo = QComboBox()
        self.can_iface_combo.setMinimumWidth(110)
        self.can_iface_combo.currentTextChanged.connect(self.on_can_interface_changed)

        self.can_chan_combo = QComboBox()
        self.can_chan_combo.setEditable(True)
        self.can_chan_combo.setMinimumWidth(100)

        self.can_sbaud_combo = QComboBox()
        self.can_sbaud_combo.addItems(["2000000", "1000000", "115200"])
        self.can_sbaud_combo.setCurrentText("2000000")
        self.can_sbaud_combo.setMinimumWidth(95)

        self.can_bitrate_combo = QComboBox()
        self.can_bitrate_combo.addItems(["125000", "250000", "500000", "1000000"])
        self.can_bitrate_combo.setCurrentText("500000")
        self.can_bitrate_combo.setMinimumWidth(95)

        self.can_view_combo = QComboBox()
        self.can_view_combo.addItems(["1-Line Smooth", "2-Line Compact", "3-Line Full"])
        self.can_view_combo.setCurrentText("1-Line Smooth")
        self.can_view_combo.setMinimumWidth(135)
        self.can_view_combo.currentTextChanged.connect(self.on_view_mode_changed)

        can_row1.addWidget(QLabel("Iface:"))
        can_row1.addWidget(self.can_iface_combo)
        can_row1.addWidget(QLabel("Channel:"))
        can_row1.addWidget(self.can_chan_combo)
        can_row1.addWidget(QLabel("USB Baud:"))
        can_row1.addWidget(self.can_sbaud_combo)
        can_row1.addWidget(QLabel("CAN Rate:"))
        can_row1.addWidget(self.can_bitrate_combo)
        can_row1.addWidget(QLabel("View Mode:"))
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