class SID:
    SESSION_CONTROL = 0x10
    ECU_RESET = 0x11
    CLEAR_DTC = 0x14
    READ_DTC = 0x19
    READ_DID = 0x22
    SECURITY_ACCESS = 0x27
    WRITE_DID = 0x2E
    ROUTINE_CONTROL = 0x31
    TESTER_PRESENT = 0x3E
    
    POSITIVE_OFFSET = 0x40
    NEGATIVE_RESPONSE = 0x7F
    SUPPRESS_POS_RSP_BIT = 0x80

class Session:
    DEFAULT = 0x01
    PROGRAMMING = 0x02
    EXTENDED = 0x03

class ResetType:
    HARD = 0x01
    KEY_OFF_ON = 0x02
    SOFT = 0x03

class DID:
    DEFAULT_PARAM_TEST = 0x0100  # 4B: Đọc/ghi tham số EEPROM AT24C02
    RADAR_HW_RAW       = 0x0101  # 5B: 4 mắt Raw (FL, FR, RL, RR) + Raw ErrMask
    RADAR_LOGIC_DIST   = 0x0102  # 7B: 6 mắt Logic (FL, FC, FR, RL, RC, RR) + Logic ErrMask
    RADAR_SYS_STATUS   = 0x0103  # 5B: Gear, Warn, Error, Session, SecLevel
    RADAR_FULL_TELEM   = 0x0104  # 11B: Gear, 6 mắt Logic, Warn, Error, Session, SecLevel

class NRC:
    CODES = {
        0x10: "General Reject",
        0x11: "Service Not Supported",
        0x12: "Sub-function Not Supported",
        0x13: "Incorrect Message Length Or Invalid Format",
        0x22: "Conditions Not Corrected",
        0x31: "Request Out Of Range",
        0x33: "Security Access Denied",
        0x35: "Invalid Key",
        0x36: "Exceed Number Of Attempts",
        0x72: "General Programming Failure",
        0x7F: "Service Not Supported In Active Session",
    }