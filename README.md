# 🚘 ADAS Ultrasonic Radar ECU & Qt6 HMI

An advanced, AUTOSAR-inspired embedded radar system built on the STM32F446RE platform. This project demonstrates a production-ready approach to embedded software architecture, featuring strict hardware decoupling (Zero-HAL), mathematical signal processing, and a modern C++ desktop companion app.

## 🌟 Key Features
*   **Zero-HAL Architecture:** Strict separation of concerns (MCAL, EAL, RTE, AppL). The application layer contains absolute zero hardware-specific code, ensuring seamless migration to RTOS or different MCU vendors.
*   **Advanced Signal Processing:** 
    *   Hardware timer-based Input Capture for microsecond-level accuracy.
    *   3-Stage Pipeline: Outlier Rejection -> Median Filter -> **1D Kalman Filter** for ultra-smooth distance tracking.
    *   **Sensor Fusion:** Virtual center-point calculation interpolating blind spots between physical corners.
*   **Robust HMI Input:** Rotary Encoder integration featuring a custom **Software Hysteresis Buffer** to completely eliminate mechanical bounce and jitter during gear shifting (P-R-N-D).
*   **Non-Blocking Comm:** Direct Memory Access (DMA) driven UART transmission using a custom NMEA-style payload (`$RADAR`) to ensure 0% CPU blocking during data telemetry.
*   **Qt6 C++ Dashboard:** A CMake-based Qt6 desktop application utilizing `QSerialPort` and Event-Driven parsing for real-time data visualization.

## 🧰 Hardware Topology
*   **MCU:** STM32 Nucleo-F446RE (Running at 3.3V logic).
*   **Sensors:** 4x HC-SR04+ Ultrasonic Sensors (Mounted at FL, FR, RL, RR positions). Powered via 3.3V, utilizing STM32 5V-Tolerant (FT) pins for safe Echo capture without voltage dividers.
*   **Actuators/Inputs:** 
    *   1x Rotary Encoder (Gear selector) ergonomically placed to prevent sensor interference.
    *   1x Active Buzzer for dynamic proximity alerts (Variable tick-rates based on danger zones).

## 🧩 Software Architecture Stack
The firmware is designed using a Lite-AUTOSAR methodology:
1.  **AppL (Application Layer):** Contains `Swc_AdasWarning` (Kalman logic, Buzzing rules, Gear logic). Pure C/Math, hardware agnostic.
2.  **RTE (Runtime Environment):** Global data warehouse isolating sensor acquisition from algorithm execution.
3.  **EAL (ECU Abstraction Layer):** Wraps raw MCAL ticks into logical units (e.g., converting Timer Deltas to Mechanical Steps). 
4.  **Services:** UART payload packing & transmission.
5.  **MCAL (Microcontroller Abstraction):** STM32 HAL, NVIC, DMA, and Timer register configurations.

## ⏱️ Task Scheduling
Currently operating on a lightweight Bare-metal OS:
*   **25ms Heartbeat Scheduler:** Triggered via Basic Timer (TIM6).
*   **Low-Power State:** CPU enters `Wait-For-Interrupt (WFI)` mode during idle cycles to maximize energy efficiency.
*   *Ready for RTOS migration.*

## 💻 Building the Qt6 HMI
The desktop application is built with modern C++17 and CMake.
```bash
cd Qt_RadarHMI
mkdir build && cd build
cmake ..
cmake --build .