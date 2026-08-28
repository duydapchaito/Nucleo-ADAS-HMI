# 1. Cấu trúc Phân tầng Kiến trúc (Architecture Layers)

Dự án áp dụng tư duy phân tầng lấy cảm hứng từ AUTOSAR (Lite-AUTOSAR), hướng tới mục tiêu **Zero-HAL** tại tầng ứng dụng. Toàn bộ mã nguồn được quy hoạch thành các module độc lập:

```
Project_Root/
├── Core/                      (Code do CubeMX sinh ra, chứa main.c "rỗng" để gọi Boot)
│
├── MCAL/                      (Microcontroller Abstraction Layer)
│   ├── Inc/ 
│   │   ├── Mcal_Timer.h       (Bọc cấu hình đếm xung, Delay)
│   │   ├── Mcal_Dio.h         (Bọc GPIO)
│   │   └── Mcal_Uart.h        (Bọc phần truyền nhận vật lý)
│   └── Src/ ...
│
├── ECUAL/                     (ECU Abstraction Layer - IoHwAb)
│   ├── Inc/
│   │   ├── Eal_Buzzer.h     (Định nghĩa còi kêu/tắt dựa trên Mcal_Dio)
│   │   ├── Eal_RadarHw.h    (Quản lý phát xung Trigger và nhận ngắt Echo)
│   │   └── Eal_Encoder.h    (Quản lý đọc Delta đếm xung vật lý)
│   └── Src/ ...
│
├── Services/                  (Basic Software - Tầng Dịch vụ Hệ thống)
│   ├── Inc/
│   │   └── Com_Uart_If.h      (Giao tiếp UART mức logic - Dọn đường cho CanIf/UDS)
│   └── Src/ ...
│
├── CDD_MathLibs/              (Complex Device Drivers / Pure Math - Tầng tính toán)
│   ├── Inc/
│   │   ├── Lib_Filter.h       (Thuật toán Kalman, EMA - C thuần 100%, độc lập phần cứng)
│   │   └── Lib_Fusion.h       (Toán học nội suy góc chéo)
│   └── Src/ ...
│
├── RTE/                       (Runtime Environment - Môi trường Môi giới)
│   ├── Inc/
│   │   └── Rte_Radar.h        (Chứa Shadow Buffers, Cờ đồng bộ 2 pha quét, trạng thái toàn cục)
│   └── Src/ ...
│
└── AppL/                      (Application Layer - Khối nghiệp vụ cốt lõi)
    ├── Inc/
    │   ├── Swc_AdasWarning.h  (Software Component: Rút data từ RTE, tính toán ngữ cảnh, kích hoạt Buzzer)
    │   └── Swc_HmiRouter.h    (Software Component: Rút data từ RTE, ép chuỗi, đẩy xuống BSW_Com)
    └── Src/ ...
```