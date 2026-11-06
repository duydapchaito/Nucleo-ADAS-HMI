from .did_definitions import SID, NRC

class UdsException(Exception):
    pass

class UdsClient:
    def __init__(self, can_tp):
        self.tp = can_tp

    def _check_nrc(self, req_sid, response: bytes):
        if not response:
            raise TimeoutError("Không nhận được dữ liệu từ ECU.")
        if response[0] == SID.NEGATIVE_RESPONSE:
            if len(response) >= 3 and response[1] == req_sid:
                nrc_code = response[2]
                nrc_desc = NRC.CODES.get(nrc_code, "Unknown NRC")
                raise UdsException(f"NRC 0x{nrc_code:02X} ({nrc_desc})")
            raise UdsException("Lỗi UDS 0x7F không xác định.")
        if response[0] != (req_sid + SID.POSITIVE_OFFSET):
            raise UdsException(
                f"Sai luồng phản hồi: Đợi 0x{req_sid + SID.POSITIVE_OFFSET:02X}, Nhận 0x{response[0]:02X}"
            )

    def tester_present(self):
        req = bytes([SID.TESTER_PRESENT, 0x00])
        self.tp.send_payload(req)
        res = self.tp.recv_payload()
        self._check_nrc(SID.TESTER_PRESENT, res)
        return res

    def session_control(self, session_type):
        req = bytes([SID.SESSION_CONTROL, session_type])
        self.tp.send_payload(req)
        res = self.tp.recv_payload()
        self._check_nrc(SID.SESSION_CONTROL, res)
        return res

    def ecu_reset(self, reset_type):
        req = bytes([SID.ECU_RESET, reset_type])
        self.tp.send_payload(req)
        res = self.tp.recv_payload()
        self._check_nrc(SID.ECU_RESET, res)
        return res

    def read_did(self, did):
        did_h = (did >> 8) & 0xFF
        did_l = did & 0xFF
        req = bytes([SID.READ_DID, did_h, did_l])
        self.tp.send_payload(req)
        res = self.tp.recv_payload()
        self._check_nrc(SID.READ_DID, res)

        if len(res) < 3:
            raise UdsException(f"Gói phản hồi 0x62 bị cụt (Chỉ nhận được {len(res)} byte).")
        if res[1] != did_h or res[2] != did_l:
            raise UdsException(f"Echo DID sai: Đợi 0x{did:04X}, Nhận 0x{(res[1]<<8)|res[2]:04X}")

        return res[3:]