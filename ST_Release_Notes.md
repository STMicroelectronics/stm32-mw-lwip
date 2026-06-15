


# Release Note for LwIP SW Pack


# Purpose

STM32Cube enables developers to achieve design success. With a comprehensive suite of professional development tools and embedded software components, STM32Cube allows developers to differentiate products, streamline design cycles, and reduce costs. STM32Cube ecosystem supports all design steps, including selection, configuration, development, debugging, programming, and monitoring.

The STM32Cube embedded software offer provides ready-to-use software components that can be added to a project. It includes STM32 peripheral driver APIs with two levels of abstraction, middleware, board drivers, and examples. There are several distribution channels, including the STM32CubeMX2 tool, the ST website, and GitHub. All embedded software comes with enhanced online documentation, with flowcharts and user sequences.

**LwIP** is a small independent implementation of the TCP/IP protocol suite. It is deployed across selected STM32 series.

# Documentation

- More documentation is available at [LwIP SW Pack online documentation](https://dev.st.com/stm32cube-docs/mw-lwip/latest/en/index.html).

- Reference documentation for [LwIP open source software](https://nongnu.org/lwip).

  The reference documentation sources are available under `doc/doxygen` directory and can be generated with [doxygen](https://doxygen.nl/) tool with configuration file `doc/doxygen/lwip.Doxyfile`.


# Update History

<label for="collapse-section-2.1.0" aria-hidden="true">__2.1.0 / 12-June-2026__</label>
<div>

## Main changes

- Add configuration and code generation in CubeMX2.
- Add Common and RTOS port components.
- This pack is based on LwIP V2.2.1 version.

## Contents

- Support for STM32 Ethernet interface.

## Known Limitations

- None.

## Development toolchains and compilers

- IAR Embedded Workbench for ARM (EWARM) toolchain V9.60.3 + ST-LINK
- MDK-ARM Keil uVision V5.42
- STM32CubeIDE for Visual Studio Code (GCC13 compiler)
- STM32CubeMX2 1.0.1

## Supported devices and boards

- STM32C5 Series.

## Backward compatibility

- None

## Dependencies

- STM32C5xx HAL Drivers V2.1.0
- FreeRTOS SW Pack V2.1.0
- LAN8742 Part Drivers V2.0.1

</div>

<label for="collapse-section-2.0.1" aria-hidden="true">__2.0.1 / 11-April-2026__</label>
<div>

## Main changes

- Maintenance release of LwIP SW Pack.

  This pack is based on LwIP V2.2.1 version.

## Contents

- Support for STM32 Ethernet interface.

## Known Limitations

- None.

## Development toolchains and compilers

- IAR Embedded Workbench for ARM (EWARM) toolchain V9.60.3 + ST-LINK
- MDK-ARM Keil uVision V5.42
- STM32CubeIDE for Visual Studio Code (GCC13 compiler)

## Supported devices and boards

- STM32C5 Series.
- STM32V8 Series.

## Backward compatibility

- None

## Dependencies

- STM32C5xx HAL Drivers V2.0.0
- LAN8742 Part Drivers V2.0.0
- FreeRTOS SW Pack V2.0.0

</div>

<label for="collapse-section-2.0.0" aria-hidden="true">__2.0.0 / 13-March-2026__</label>
<div>

## Main changes

- First Official release of LwIP SW Pack.

  This pack is based on LwIP V2.2.1 version.

## Contents

- Support for STM32 Ethernet interface.

## Known Limitations

- None.

## Development toolchains and compilers

- IAR Embedded Workbench for ARM (EWARM) toolchain V9.60.3 + ST-LINK
- MDK-ARM Keil uVision V5.42
- STM32CubeIDE for Visual Studio Code (GCC13 compiler)

## Supported devices and boards

- STM32C5 Series.

## Backward compatibility

- None

## Dependencies

- STM32C5xx HAL Drivers V2.0.0
- LAN8742 Part Drivers V2.0.0
- FreeRTOS SW Pack V2.0.0

</div>





For complete documentation on **STM32 Microcontrollers**,
visit: [www.st.com/stm32](http://www.st.com/stm32)