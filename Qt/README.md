# Parking Assist HMI Dashboard (Qt6 / C++)
A high-performance, event-driven HMI desktop application built with Qt6. This project serves as the **Instrument Cluster** for an STM32-based ADAS Ultrasonic Radar system. 

## Key Features
*   **Plug & Play:** Auto-detects STM32 UART connections via OS hardware registry (no hardcoded COM ports).
*   **Real-time Rendering:** 60FPS hardware-accelerated radar arcs using custom C++ `QQuickPaintedItem`.
*   **Dumb UI - Smart Backend:** Zero raw data processing on the UI layer, dedicating 100% resources to rendering.
*   **Auto-Reverse Camera:** Seamlessly integrates laptop webcams, triggered dynamically by hardware gear shifts (R-Gear).

## Build & Run Instructions
**Prerequisites:**
*   Qt 6.5 or higher (with `qtdeclarative`, `qtserialport`, and `qtmultimedia` modules).
*   CMake 3.16+ & C++17 standard compiler.

**Steps:**
1. Clone the repository.
2. Open `CMakeLists.txt` via Qt Creator.
3. Plug in the STM32 Radar ECU via USB.
4. Build and Run. The App will auto-detect the COM port and begin real-time telemetry rendering.

## Technical Documentation
Dive into the parsing logic, data pipelines, and strict C++/QML decoupling principles here: 
**[HMI Architecture Design](HMI_Architecture.md)**