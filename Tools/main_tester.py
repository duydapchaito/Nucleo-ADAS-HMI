import sys
import struct
import datetime
import time
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout,
                             QHBoxLayout, QGroupBox, QPushButton, QTextEdit, 
                             QSplitter, QMessageBox, QFrame, QLabel)
from PyQt5.QtCore import QThread, pyqtSignal, Qt, QTimer
from PyQt5.QtGui import QTextCursor

from tester import CanTp, UdsClient, DID, Session, ResetType, UdsException, UdpCanAdapter

DARK_STYLESHEET = """
QMainWindow, QWidget { background-color: #18181B; color: #E4E4E7; font-family: 'Segoe UI', sans-serif; font-size: 13px; }
QFrame#StatusBar { background-color: #1E1E24; border: 1px solid #27272A; border-radius: 6px; }
QGroupBox { border: 1px solid #3F3F46; border-radius: 6px; margin-top: 10px; font-weight: bold; color: #38BDF8; padding: 10px; }
QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
QPushButton { background-color: #27272A; border: 1px solid #52525B; border-radius: 4px; padding: 6px; color: #F4F4F5; font-weight: 600; }
QPushButton:hover { background-color: #3F3F46; border-color: #71717A; }
QPushButton:pressed { background-color: #18181B; }
QPushButton:disabled { background-color: #18181B; color: #52525B; border-color: #27272A; }
QTextEdit { background-color: #09090B; border: 1px solid #27272A; border-radius: 6px; font-family: 'Consolas', monospace; padding: 8px; }
"""

GEAR_MAP = {0: 'P', 1: 'R', 2: 'N', 3: 'D'}
WARN_MAP = {0: '0 (Green)', 1: '1 (Yellow)', 2: '2 (Orange)', 3: '3 (Red)'}
SESS_MAP = {1: 'Default (0x01)', 2: 'Programming (0x02)', 3: 'Extended (0x03)'}
SEC_MAP  = {0: 'Locked (L0)', 1: 'Unlocked L1', 2: 'Unlocked L2'}

class UdsWorker(QThread):
    log_sig = pyqtSignal(str)
    done_sig = pyqtSignal(bool)
    
    def __init__(self, uds_client, action_name, action_func):
        super().__init__()
        self.uds = uds_client
        self.action_name = action_name
        self.action_func = action_func
        
    def run(self):
        self.log_sig.emit(f"\n<span style='color:#F4F4F5; font-weight:bold;'>--- [CMD] {self.action_name} ---</span>")
        time.sleep(0.02)
        success = True
        try:
            self.action_func(self.uds, self.log_sig.emit)
            self.log_sig.emit(f"<span style='color:#22C55E;'><b>[OK]</b> {self.action_name} thành công.</span>")
        except UdsException as e:
            self.log_sig.emit(f"<span style='color:#EF4444;'><b>[NRC ERROR]</b> {str(e)}</span>")
        except TimeoutError as e:
            success = False
            self.log_sig.emit(f"<span style='color:#F59E0B;'><b>[TIMEOUT]</b> {str(e)}</span>")
        except Exception as e:
            success = False
            self.log_sig.emit(f"<span style='color:#EF4444;'><b>[SYS ERROR]</b> {str(e)}</span>")
        finally:
            self.done_sig.emit(success)

class TesterApp(QMainWindow):
    gui_log_sig = pyqtSignal(str)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("ISO 14229 UDS - Diagnostic Tester Tool")
        self.resize(1080, 740)
        self.setStyleSheet(DARK_STYLESHEET)
        
        self.gui_log_sig.connect(self._safe_append_log)
        
        try:
            self.adapter = UdpCanAdapter(tx_port=5007, rx_port=5006)
        except Exception as e:
            QMessageBox.critical(self, "Lỗi Khởi Tạo Mạng", 
                                 f"Không thể mở cổng UDP 5006/5007.\nChi tiết lỗi: {e}")
            sys.exit(1)

        def tp_send_logged(can_id, data):
            hex_str = " ".join([f"{b:02X}" for b in data])
            self.gui_log_sig.emit(f"<span style='color:#F59E0B;'> [TX] 0x{can_id:03X} | {hex_str}</span>")
            self.adapter.send_frame(can_id, data)
            
        def tp_recv_logged():
            msg = self.adapter.recv_frame()
            if msg:
                hex_str = " ".join([f"{b:02X}" for b in msg['data']])
                self.gui_log_sig.emit(f"<span style='color:#38BDF8;'> [RX] 0x{msg['id']:03X} | {hex_str}</span>")
            return msg

        self.tp = CanTp(
            send_func=tp_send_logged,
            recv_func=tp_recv_logged,
            req_id=0x7E0,
            res_id=0x7E8,
            flush_func=self.adapter.clear_buffer
        )
        self.uds = UdsClient(self.tp)
        self.worker = None
        self._last_link_state = -1

        self.init_ui()

        # Timer tự động quét trạng thái cổng UDP & lưu lượng mỗi 300ms
        self.monitor_timer = QTimer(self)
        self.monitor_timer.timeout.connect(self._update_link_monitor)
        self.monitor_timer.start(300)

        self.gui_log_sig.emit("<span style='color:#A1A1AA;'>[*] Hệ thống Tester đã sẵn sàng. Tự động giám sát & phục hồi kết nối UDP 5006/5007.</span>")

    def init_ui(self):
        central = QWidget()
        self.setCentralWidget(central)
        root_layout = QVBoxLayout(central)
        root_layout.setContentsMargins(8, 8, 8, 8)
        root_layout.setSpacing(8)

        # --- THANH TRẠNG THÁI LIVE UDP / GATEWAY MONITOR (XANH - VÀNG - ĐỎ) ---
        status_bar = QFrame()
        status_bar.setObjectName("StatusBar")
        status_bar.setFixedHeight(40)
        status_layout = QHBoxLayout(status_bar)
        status_layout.setContentsMargins(12, 4, 12, 4)

        lbl_title = QLabel("⚡ UDS DIAGNOSTIC TESTER")
        lbl_title.setStyleSheet("color: #F4F4F5; font-weight: bold; font-size: 13px;")

        self.lbl_link_badge = QLabel("● Đang dò cổng UDP...")
        self.lbl_link_badge.setStyleSheet(
            "background-color: #27272A; color: #F59E0B; font-weight: bold; "
            "padding: 4px 10px; border-radius: 4px; font-family: 'Consolas', monospace;"
        )

        self.lbl_traffic_info = QLabel("UDP TX: 5007 | RX: 5006")
        self.lbl_traffic_info.setStyleSheet("color: #A1A1AA; font-family: 'Consolas', monospace;")

        status_layout.addWidget(lbl_title)
        status_layout.addSpacing(15)
        status_layout.addWidget(self.lbl_link_badge)
        status_layout.addStretch()
        status_layout.addWidget(self.lbl_traffic_info)
        root_layout.addWidget(status_bar)

        # --- PHẦN THÂN CHÍNH ---
        ctrl_panel = QWidget()
        ctrl_panel.setFixedWidth(330)
        ctrl_layout = QVBoxLayout(ctrl_panel)
        ctrl_layout.setContentsMargins(0, 0, 0, 0)

        # Nhóm 1: Session & Reset
        grp_sess = QGroupBox("1. Session && ECU Reset")
        vbox_sess = QVBoxLayout(grp_sess)
        self.btn_tp  = self.create_btn("Tester Present (0x3E)", lambda: self.run_uds("Tester Present (0x3E)", self._act_tp))
        self.btn_ext = self.create_btn("Extended Session (0x10 03)", lambda: self.run_uds("Extended Session (0x10 03)", self._act_ext_sess))
        self.btn_def = self.create_btn("Default Session (0x10 01)", lambda: self.run_uds("Default Session (0x10 01)", self._act_def_sess))
        self.btn_rst = self.create_btn("Hard Reset ECU (0x11 01)", lambda: self.run_uds("Hard Reset (0x11 01)", self._act_hard_reset))
        self.btn_rst.setStyleSheet("background-color: #7f1d1d; border-color: #991b1b;")
        
        vbox_sess.addWidget(self.btn_tp)
        vbox_sess.addWidget(self.btn_ext)
        vbox_sess.addWidget(self.btn_def)
        vbox_sess.addWidget(self.btn_rst)
        ctrl_layout.addWidget(grp_sess)

        # Nhóm 2: Đọc Dữ Liệu (0x0101 -> 0x0104)
        grp_read = QGroupBox("2. Read Data By ID (0x22)")
        vbox_read = QVBoxLayout(grp_read)
        self.btn_r_0101 = self.create_btn("Read 4 HW Raw + Mask (0x0101)", lambda: self.run_uds("Read DID 0x0101 (4 HW Raw + Mask)", self._act_read_0101))
        self.btn_r_0102 = self.create_btn("Read 6 Logic + Mask (0x0102)", lambda: self.run_uds("Read DID 0x0102 (6 Logic + Mask)", self._act_read_0102))
        self.btn_r_0103 = self.create_btn("Read Sys Status (0x0103)", lambda: self.run_uds("Read DID 0x0103 (Sys Status)", self._act_read_0103))
        self.btn_r_0104 = self.create_btn("Read Full Telemetry (0x0104)", lambda: self.run_uds("Read DID 0x0104 (Full Telemetry)", self._act_read_0104))
        self.btn_r_0104.setStyleSheet("background-color: #1e3a8a; border-color: #3b82f6;")

        vbox_read.addWidget(self.btn_r_0101)
        vbox_read.addWidget(self.btn_r_0102)
        vbox_read.addWidget(self.btn_r_0103)
        vbox_read.addWidget(self.btn_r_0104)
        ctrl_layout.addWidget(grp_read)

        ctrl_layout.addStretch()

        # Nút Clear Log
        self.btn_clear = QPushButton("⎚ Clear Log")
        self.btn_clear.setStyleSheet("background-color: #450a0a; border: 1px solid #7f1d1d; color: #fca5a5;")
        self.btn_clear.clicked.connect(lambda: self.log_view.clear())
        ctrl_layout.addWidget(self.btn_clear)

        self.log_view = QTextEdit()
        self.log_view.setReadOnly(True)

        splitter = QSplitter(Qt.Horizontal)
        splitter.addWidget(ctrl_panel)
        splitter.addWidget(self.log_view)
        splitter.setSizes([330, 750])
        root_layout.addWidget(splitter, 1)

    def _update_link_monitor(self):
        state, fps, ago = self.adapter.get_link_health()

        if state == 0:
            # ĐỎ: Gateway đã Stop CAN hoặc tắt
            self.lbl_link_badge.setText("● OFFLINE: Gateway CAN Stopped (UDP 5007 Closed)")
            self.lbl_link_badge.setStyleSheet(
                "background-color: #450A0A; color: #FCA5A5; border: 1px solid #DC2626; "
                "font-weight: bold; padding: 4px 10px; border-radius: 4px; font-family: 'Consolas', monospace;"
            )
            self.lbl_traffic_info.setText("Traffic: 0 fps | Hãy bật 'Start CAN' trên Gateway")
            if self._last_link_state not in (-1, 0):
                self.gui_log_sig.emit("<span style='color:#EF4444;'>[!] Phát hiện Gateway ngắt cổng UDP 5007 (Stop CAN). Đang chờ kết nối lại...</span>")

        elif state == 1:
            # VÀNG: Gateway đã mở cổng 5007, đang chờ lệnh hoặc vừa Timeout ECU
            status_txt = "● STANDBY: Gateway Ready (Last Cmd Timeout)" if self.adapter.last_uds_timeout else "● STANDBY: Gateway UDP Ready (Idle)"
            self.lbl_link_badge.setText(status_txt)
            self.lbl_link_badge.setStyleSheet(
                "background-color: #422006; color: #FDE047; border: 1px solid #D97706; "
                "font-weight: bold; padding: 4px 10px; border-radius: 4px; font-family: 'Consolas', monospace;"
            )
            ago_str = f"{ago:.1f}s ago" if ago < 900 else "No RX yet"
            self.lbl_traffic_info.setText(f"UDP 5006/5007: OPEN | Last RX: {ago_str}")
            if self._last_link_state == 0:
                self.gui_log_sig.emit("<span style='color:#22C55E;'>[*] Cổng UDP 5007 trên Gateway đã mở lại! Sẵn sàng gửi lệnh UDS.</span>")

        else:
            # XANH: Đường truyền UDP & ECU phản hồi tốt
            self.lbl_link_badge.setText("● ONLINE: UDP & ECU Link Active")
            self.lbl_link_badge.setStyleSheet(
                "background-color: #052E16; color: #4ADE80; border: 1px solid #16A34A; "
                "font-weight: bold; padding: 4px 10px; border-radius: 4px; font-family: 'Consolas', monospace;"
            )
            self.lbl_traffic_info.setText(f"Rate: {fps} pkt/s | Last RX: {ago:.1f}s ago")
            if self._last_link_state == 0:
                self.gui_log_sig.emit("<span style='color:#22C55E;'>[*] Kết nối UDP & ECU đã khôi phục hoàn toàn!</span>")

        self._last_link_state = state

    def create_btn(self, text, callback):
        btn = QPushButton(text)
        btn.clicked.connect(callback)
        return btn

    def _safe_append_log(self, text):
        if text.startswith("\n"):
            self.log_view.append("")
            text = text[1:]
            
        t = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
        self.log_view.append(f"<span style='color:#52525B;'>[{t}]</span> {text}")
        self.log_view.moveCursor(QTextCursor.End)

    def set_buttons_enabled(self, enabled):
        for btn in self.findChildren(QPushButton):
            if btn != self.btn_clear:
                btn.setEnabled(enabled)

    def _on_worker_done(self, success):
        if not success:
            self.adapter.last_uds_timeout = True
        self.set_buttons_enabled(True)
        self._update_link_monitor()

    def run_uds(self, name, func):
        if self.worker is not None and self.worker.isRunning():
            return
        self.set_buttons_enabled(False)
        self.worker = UdsWorker(self.uds, name, func)
        self.worker.log_sig.connect(self.gui_log_sig.emit)
        self.worker.done_sig.connect(self._on_worker_done)
        self.worker.start()

    # --- ĐỊNH NGHĨA CÁC HÀM XỬ LÝ UDS ---
    
    def _act_hard_reset(self, uds, log):
        uds.ecu_reset(ResetType.HARD)
        log("   └─> ECU đang tiến hành Reset phần cứng...")

    def _act_tp(self, uds, log):
        uds.tester_present()

    def _act_ext_sess(self, uds, log):
        uds.session_control(Session.EXTENDED)

    def _act_def_sess(self, uds, log):
        uds.session_control(Session.DEFAULT)

    def _act_read_0101(self, uds, log):
        data = uds.read_did(DID.RADAR_HW_RAW)
        if len(data) == 5:
            fl, fr, rl, rr = [x * 2 for x in data[0:4]]
            raw_mask = data[4]
            log(f"   └─> 4 Mắt Raw (cm): FL={fl}, FR={fr}, RL={rl}, RR={rr}")
            log(f"   └─> Raw ErrMask: <b>0x{raw_mask:02X}</b> (Bit 0..3: {raw_mask & 0x0F:04b}b)")
        elif len(data) == 9:
            fl, fr, rl, rr = struct.unpack(">HHHH", data[0:8])
            raw_mask = data[8]
            log(f"   └─> 4 Mắt Raw Ticks (us): FL={fl}, FR={fr}, RL={rl}, RR={rr}")
            log(f"   └─> Raw ErrMask: <b>0x{raw_mask:02X}</b> (Bit 0..3: {raw_mask & 0x0F:04b}b)")
        else:
            log(f"   └─> [LỖI] Kích thước DID 0x0101 sai: Nhận {len(data)} bytes, đợi 5 bytes.")

    def _act_read_0102(self, uds, log):
        data = uds.read_did(DID.RADAR_LOGIC_DIST)
        if len(data) != 7:
            log(f"   └─> [LỖI] Kích thước DID 0x0102 sai: Nhận {len(data)} bytes, đợi 7 bytes.")
            return
        fl, fc, fr, rl, rc, rr = [x * 2 for x in data[0:6]]
        logic_mask = data[6]
        log(f"   └─> Logic Trước (cm): FL={fl} | FC(Ảo)={fc} | FR={fr}")
        log(f"   └─> Logic Sau   (cm): RL={rl} | RC(Ảo)={rc} | RR={rr}")
        log(f"   └─> Logic ErrMask: <b>0x{logic_mask:02X}</b> (Bit 0..5: {logic_mask & 0x3F:06b}b)")

    def _act_read_0103(self, uds, log):
        data = uds.read_did(DID.RADAR_SYS_STATUS)
        if len(data) != 5:
            log(f"   └─> [LỖI] Kích thước DID 0x0103 sai: Nhận {len(data)} bytes, đợi 5 bytes.")
            return
        gear, warn, err, sess, sec = struct.unpack(">BBBBB", data)
        log(f"   └─> Gear: <b>{GEAR_MAP.get(gear, f'?({gear})')}</b> | Warn: {WARN_MAP.get(warn, str(warn))} | ErrMask: 0x{err:02X}")
        log(f"   └─> Session: {SESS_MAP.get(sess, f'0x{sess:02X}')} | Security: {SEC_MAP.get(sec, f'0x{sec:02X}')}")

    def _act_read_0104(self, uds, log):
        data = uds.read_did(DID.RADAR_FULL_TELEM)
        if len(data) != 11:
            log(f"   └─> [LỖI] Kích thước DID 0x0104 sai: Nhận {len(data)} bytes, đợi 11 bytes.")
            return

        gear = data[0]
        fl, fc, fr, rl, rc, rr = [x * 2 for x in data[1:7]]
        warn, err, sess, sec = data[7:11]
        
        log(f"   └─> Gear: <b>{GEAR_MAP.get(gear, f'?({gear})')}</b> | Warn: {WARN_MAP.get(warn, str(warn))} | ErrMask: 0x{err:02X}")
        log(f"   └─> Khoảng cách (cm): [Front: FL={fl}, FC={fc}, FR={fr}] - [Rear: RL={rl}, RC={rc}, RR={rr}]")
        log(f"   └─> Session: {SESS_MAP.get(sess, f'0x{sess:02X}')} | Security: {SEC_MAP.get(sec, f'0x{sec:02X}')}")

    def closeEvent(self, event):
        self.adapter.close()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = TesterApp()
    window.show()
    sys.exit(app.exec_())