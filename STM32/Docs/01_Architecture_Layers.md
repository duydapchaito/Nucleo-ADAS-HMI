# 1. Cấu trúc Phân tầng Kiến trúc (Architecture Layers)

Dự án áp dụng tư duy phân tầng lấy cảm hứng từ AUTOSAR (Lite-AUTOSAR), hướng tới mục tiêu **Zero-HAL** tại tầng ứng dụng. Toàn bộ mã nguồn được quy hoạch thành các module độc lập:

```
Radar_Core_ECU/
├── MCAL/
│   ├── Mcal_Dio.h / .c          [GIỮ NGUYÊN] - Đọc/ghi chân GPIO (Trigger, Buzzer)
│   ├── Mcal_Uart.h / .c         [GIỮ NGUYÊN] - Truyền UART2 DMA
│   ├── Mcal_Pwr.h / .c          [CẬP NHẬT]   - Đổi cờ sang PWR_SLEEPENTRY_WFI
│   ├── Mcal_Timer.h / .c        [CẬP NHẬT]   - TIM1 (Capture + DelayUs), TIM2 (Encoder)
│   └── Mcal_Can.h / .c          [FILE MỚI]   - CAN1 Driver (Filter, TX Mailbox, RX FIFO0 ISR)
│
├── EAL/
│   ├── Eal_RadarHw.h / .c       [GIỮ NGUYÊN] - Bắn Trigger & tính độ rộng xung 4 mắt thật
│   ├── Eal_Encoder.h / .c       [GIỮ NGUYÊN] - Tính độ lệch xung xoay cần số
│   ├── Eal_Buzzer.h / .c        [GIỮ NGUYÊN] - Điều khiển còi cảnh báo
│   └── Eal_CanIf.h / .c         [FILE MỚI]   - Ánh xạ PDU ID <-> CAN ID (0x300, 0x7E8)
│
├── CDD_MathLibs/
│   ├── Cdd_Filter.h / .c        [CẬP NHẬT]   - Đổi Ticks -> cm, lọc nhiễu EMA & bắt lỗi đứt dây 4 mắt thật
│   └── Cdd_Fusion.h / .c        [CẬP NHẬT]   - Hợp thể 4 mắt thật -> 6 mắt đồng đẳng + đóng gói 6-bit ErrorMask
│
├── Services/
│   ├── Os.h / .c                [FILE MỚI]   - Quản lý FreeRTOS Tasks, Mutex, Queue, WFI, điều phối EAL->CDD->RTE
│   └── Com_Manager.h / .c       [ĐỔI TÊN]    - (Từ Com_Uart_If) Đóng gói CAN 8-Byte (0x300) & UART NMEA
│
├── RTE/
│   └── Rte_Radar.h / .c         [CẬP NHẬT]   - Trạm trung chuyển Thread-Safe (Biết 4 HW Ticks & 6 Sensor Distances)
│
└── AppL/
    └── Swc_Radar.h / .c         [CẬP NHẬT]   - (Boss cuối) Đọc mảng 6 cảm biến đồng đẳng, chẩn đoán Gear, Buzzer, gọi Com
```