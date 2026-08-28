# Modular ADAS Ultrasonic Radar System

A full-stack embedded engineering showcase. This repository demonstrates a production-grade approach to building an Advanced Driver Assistance System (ADAS), bridging a bare-metal, AUTOSAR-inspired STM32 ECU with a high-performance Qt6 desktop dashboard.

The core philosophy of this project is **Strict Decoupling**: separating hardware abstractions from pure mathematical logic on the MCU, and isolating raw data processing from the UI rendering on the desktop.

---

## System Topology & Data Flow

The system operates on a "Smart Backend - Dumb UI" paradigm, communicating via a custom, lightweight NMEA-style telemetry protocol (`$RADAR`).

```
+-------------------------+                                +-------------------------+
|     STM32 ECU (Core)    |                                |   Qt6 Dashboard (HMI)   |
|-------------------------|                                |-------------------------|
| - Lite-AUTOSAR Stack    |      $RADAR Protocol           | - Event-Driven Parsing  |
| - 1D Kalman Filtering   |   =======================>     | - QSerialPort Async     |
| - Sensor Fusion         |      (UART over DMA)           | - Custom QQuickItem     |
| - Zero-HAL Application  |                                | - 60FPS UI Rendering    |
+-------------------------+                                +-------------------------+
```

## Repository Navigation
Detailed technical documentation, architectural breakdowns, and build instructions are localized within their respective subsystem directories:

### 1. Embedded Firmware (STM32)
The "Brain" of the system. Written in C, featuring a 25ms heartbeat scheduler, hardware-agnostic application layer (Zero-HAL), and pure mathematical DSP (Digital Signal Processing) for ultrasonic noise rejection. Read the [Firmware Architecture Docs](STM32/README.md) here.

### 2. Instrument Cluster HMI (Qt6 / C++)
The "Face" of the system. A cross-platform desktop application acting as a real-time Instrument Cluster. It features automated COM-port detection, hardware-accelerated radar arcs, and dynamic reverse-camera integration. Read the [HMI Architecture & Build Guide](Qt/README.md) here.

## Engineering Highlights
*   **Architectural Discipline:** Strict separation of concerns across the entire stack (MCAL -> ECUAL -> RTE -> AppL -> UI).
*   **Zero-Blocking Telemetry:** Utilizing Direct Memory Access (DMA) on the MCU and Asynchronous OS-level interrupts (`readyRead`) on the PC to ensure 0% CPU blockage during data transmission.
*   **Algorithmic Debouncing:** Replacing traditional `if-else` delay traps with Mathematical Hysteresis Buffers to handle mechanical rotary encoder jitter.

## Future Roadmap (Next Phase)
The architecture is designed to be highly scalable. Upcoming milestones include:

*   [ ] Migration from UART to CAN Bus physical layer.

*   [ ] Implementation of ISO 15765-2 (CAN-TP) for PDU segmentation.

*   [ ] Integration of a Lite DCM (Diagnostic Communication Manager) handling ISO 14229 (UDS) services.

*   [ ] Migration from Bare-metal scheduler to FreeRTOS.