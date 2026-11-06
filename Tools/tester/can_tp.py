import time

class CanTp:
    def __init__(self, send_func, recv_func, req_id=0x7E0, res_id=0x7E8, flush_func=None):
        self._send_can_frame = send_func
        self._recv_can_frame = recv_func
        self._flush_rx = flush_func
        self.req_id = req_id
        self.res_id = res_id

    def _clear_stale_frames(self):
        if self._flush_rx:
            self._flush_rx()
        else:
            while self._recv_can_frame() is not None:
                pass

    def send_payload(self, payload: bytes):
        # Dọn sạch mọi frame rác tồn đọng từ phiên lỗi trước đó
        self._clear_stale_frames()
        payload_len = len(payload)
        
        # 1. Single Frame (SF)
        if payload_len <= 7:
            can_data = bytearray([payload_len]) + bytearray(payload)
            can_data += bytearray([0x00] * (8 - len(can_data)))
            self._send_can_frame(self.req_id, can_data)
            return

        # 2. Multi-frame: Bắn First Frame (FF)
        ff_header = bytearray([0x10 | ((payload_len >> 8) & 0x0F), payload_len & 0xFF])
        ff_data = ff_header + bytearray(payload[:6])
        self._send_can_frame(self.req_id, ff_data)

        # Chờ Flow Control (FC) từ ECU
        start_wait_fc = time.time()
        fc_received = False
        while (time.time() - start_wait_fc) < 1.0:
            msg = self._recv_can_frame()
            if msg and msg['id'] == self.res_id:
                if (msg['data'][0] >> 4) == 0x3:
                    fc_received = True
                    break
            time.sleep(0.005)

        if not fc_received:
            raise TimeoutError("CanTp TX Timeout: ECU không trả lời Flow Control (0x30).")

        # Bắn Consecutive Frames (CF)
        idx = 6
        sn = 1
        while idx < payload_len:
            chunk = payload[idx : idx + 7]
            cf_header = bytearray([0x20 | (sn & 0x0F)])
            cf_data = cf_header + bytearray(chunk)
            cf_data += bytearray([0x00] * (8 - len(cf_data)))
            
            self._send_can_frame(self.req_id, cf_data)
            idx += len(chunk)
            sn = (sn + 1) & 0x0F
            time.sleep(0.01)

    def recv_payload(self, timeout=2.0) -> bytes:
        start_time = time.time()
        rx_buffer = bytearray()
        expected_len = 0
        next_sn = 1

        while (time.time() - start_time) < timeout:
            msg = self._recv_can_frame()
            while msg:
                if msg['id'] == self.res_id:
                    data = msg['data']
                    pci_type = data[0] >> 4

                    # Nhận Single Frame (SF)
                    if pci_type == 0x0:
                        uds_len = data[0] & 0x0F
                        return bytes(data[1 : 1 + uds_len])

                    # Nhận First Frame (FF)
                    elif pci_type == 0x1:
                        expected_len = ((data[0] & 0x0F) << 8) | data[1]
                        rx_buffer = bytearray(data[2:8])
                        next_sn = 1
                        
                        fc_frame = bytearray([0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00])
                        self._send_can_frame(self.req_id, fc_frame)

                    # Nhận Consecutive Frame (CF)
                    elif pci_type == 0x2:
                        sn = data[0] & 0x0F
                        if sn == next_sn:
                            next_sn = (next_sn + 1) & 0x0F
                            remain = expected_len - len(rx_buffer)
                            chunk_size = min(remain, 7)
                            rx_buffer += data[1 : 1 + chunk_size]

                            if len(rx_buffer) >= expected_len:
                                return bytes(rx_buffer)

                msg = self._recv_can_frame()
            time.sleep(0.005)

        raise TimeoutError("CanTp RX Timeout: Quá thời gian chờ ECU phản hồi.")