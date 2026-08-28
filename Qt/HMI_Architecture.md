# HMI Architecture Design

Designed with the **"Dumb UI - Smart Backend"** philosophy, this application handles zero raw data processing, dedicating 100% of its resources to real-time telemetry parsing and fluid UI rendering.

## 1. Directory Structure (C++ / QML Decoupling)
The project strictly enforces separation of concerns between the logic backend and the presentation frontend:

```
Qt_ParkingSensor/
├── C++_Backend/
│   ├── main.cpp                 (Entry point, registers QML types & context properties)
│   ├── serialcontroller.h/cpp   (UART parsing, auto-detect, signals emitter)
│   └── SensorSurCar.h/cpp       (Custom QQuickPaintedItem for radar arcs)
│
└── QML_Frontend/
    ├── Main.qml                 (Root window, layout orchestration)
    ├── LeftPanel.qml            (Active vehicle state & passive IVI components)
    ├── DashboardView.qml        (Center cluster, CameraView, Gear display)
    └── ParkingSensorPanel.qml   (Maps C++ distance properties to radar UI)
```

## 2. Core Architecture Philosophy
*   **Decoupled Frontend/Backend:** The UI (QML) and Logic (C++) are strictly separated. QML acts purely as a presentation layer, bound to C++ backend signals via `Q_PROPERTY`.
*   **Event-Driven Serial Communication:** Instead of CPU-blocking polling, the `QSerialPort` leverages OS-level interrupts (`readyRead` signals) to fetch data only when the DMA from the STM32 fires a complete payload.
*   **Mockup Isolation (Cluster vs IVI):**
    *   **Real-time Cluster (Active):** Gear Shifting (P-R-N-D) and Ultrasonic Radar Arcs are safety-critical UI elements mapped directly to live hardware data.
    *   **IVI Elements (Passive):** Music Player, Weather, and TPMS are static mockup components (dummy data) designed to simulate a complete infotainment environment without consuming parsing resources.

## 3. Data Pipeline (The `$RADAR` Protocol)
The system expects a clean, pre-filtered, and pre-calculated NMEA-style string from the STM32 ECU.
*   **Format:** `$RADAR:<Gear>,<FL>,<FC>,<FR>,<RL>,<RC>,<RR>\n`
*   **Example:** `$RADAR:D,120,45,200,-1,-1,-1\n`

**Parsing Mechanism** (`serialcontroller.cpp`):
*   **Hysteresis Buffer:** A `QByteArray` silently accumulates incoming fragmented bytes from the UART stream.
*   **Newline Trigger:** Parsing strictly occurs only when a `\n` character is detected, ensuring zero app crashes from partial serial frames.
*   **Blind-spot Masking:** Negative values (e.g., `-1`) transmitted during specific gears (like forward sensors disabled during Reverse) are automatically translated by the UI to hide the respective radar arcs.

## 4. Key Technical Implementations
### Auto-Detect COM Port
Eliminated hardcoded COM ports. The system utilizes `QSerialPortInfo` at startup to scan the OS hardware registry, searching for manufacturer footprints (`STLink`, `CP210`, `CH340`). It auto-connects to the radar ECU upon detection—true Plug & Play.

### Custom C++ QML Item (`SensorSurCar.cpp`)
Standard QML components lack the performance for dynamic, multi-arc radar rendering. We implemented a custom `QQuickPaintedItem`:
*   Overridden `paint()` event using `QPainter`.
*   Dynamically calculates arc radiuses, angles, and color gradients (Green/Yellow/Red) based directly on the distance integers (cm) pushed from the C++ Backend.
*   Hardware-accelerated anti-aliasing for smooth 60FPS rendering.