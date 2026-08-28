# ADAS Radar ECU (STM32 Firmware)

This repository contains the core embedded firmware for the ADAS Ultrasonic Radar System, running on the STM32 platform. The architecture strictly enforces the **Zero-HAL** principle at the Application Layer, completely decoupling business logic from hardware dependencies.

## Core Technical Features
*   **Kalman Filtering 1D:** Pure mathematical noise reduction eliminating hardware-induced sensor fluctuations.
*   **Sensor Fusion:** Mathematical interpolation to generate virtual center-point sensors, covering physical blind spots.
*   **Software Hysteresis Buffer:** Algorithmic debouncing for Rotary Encoder inputs, mapping precise mechanical ticks to rigid state-machine gears (P-R-N-D).
*   **DMA-driven Comm:** Non-blocking UART transmission of `$RADAR` NMEA strings to the HMI via Direct Memory Access.

## Technical Documentation
Dive into the engineering details of the system architecture:

1. [Architecture Layers & Directory Structure](docs/01_Architecture_Layers.md) - *Details the AUTOSAR-lite folder structure and layer responsibilities.*
2. [System Data Flow Model](docs/02_System_Data_Flow.md) - *Visualizes the top-to-bottom data pipeline from Hardware IT to the AppL.*
3. [Interrupt Handling Sequence](docs/03_Interrupt_Handling_Sequence.md) - *Explains the Function Pointer mechanisms used to decouple MCAL and ECUAL during asynchronous events.*

## Environment
*   **MCU:** STM32 Nucleo-F446RE
*   **Scheduler:** Bare-metal 25ms Task with Wait-For-Interrupt (WFI) low-power mode. (Ready for FreeRTOS migration).
*   **Toolchain:** STM32CubeIDE / GCC.