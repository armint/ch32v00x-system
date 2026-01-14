# Agent Instructions for ch32v00x-system

## Purpose
This library provides the Hardware Abstraction Layer (HAL), startup code, and peripheral drivers for WCH CH32V00X and some CH32V20x microcontrollers.

## Key Features
- **Startup**: Linker scripts and startup code (`startup.cpp`, `vectors_*.c`).
- **Peripherals**: Drivers for ADC, DMA, EXTI, GPIO, I2C, USART, and Timers.
- **System**: Delay functions, interrupt handling, and hardware initialization.

## Guidelines
- Peripheral drivers are located in `src/peripherals/`.
- When modifying or adding peripheral support, ensure it matches the hardware specification of the targeted WCH MCUs.
- Use the `Interrupt` utility in `src/utils/` for managing runtime interrupt connections.
- Be mindful of the limited resources (Flash/RAM) on these microcontrollers.
