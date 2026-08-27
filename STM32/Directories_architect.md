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
│   │   ├── Eal_RadarHw.h    (Chỉ quản lý việc phát xung Trigger và nhận ngắt Echo)
│   │   └── Eal_Encoder.h    (Quản lý đọc Delta đếm xung vật lý)
│   └── Src/ ...
│
├── Services/                       (Basic Software - Tầng Dịch vụ Hệ thống)
│   ├── Inc/
│   │   └── Com_Uart_If.h      (Giao tiếp UART mức logic, sau này đổi thành CanIf, CanTp, Dcm/UDS ở đây)
│   └── Src/ ...
│
├── CDD_MathLibs/              (Complex Device Drivers / Pure Math - Tầng tính toán)
│   ├── Inc/
│   │   ├── Lib_Filter.h       (Thuật toán Kalman, EMA - C thuần 100%, không biết phần cứng là gì)
│   │   └── Lib_Fusion.h       (Toán học nội suy góc chéo)
│   └── Src/ ...
│
├── RTE/                       (Runtime Environment - Môi trường thực thi & Định tuyến)
│   ├── Inc/
│   │   └── Rte_Radar.h        (Chứa Shadow Buffers, Cờ đồng bộ 2 pha quét, biến trạng thái)
│   └── Src/ ...
│
└── AppL/                      (Application Layer - Khối nghiệp vụ cốt lõi)
    ├── Inc/
    │   ├── Swc_AdasWarning.h  (Software Component: Rút data từ RTE, tính toán ngữ cảnh, ra lệnh cho EcuAb_Buzzer)
    │   └── Swc_HmiRouter.h    (Software Component: Rút data từ RTE, ép chuỗi, đẩy xuống BSW_Com)
    └── Src/ ...
```