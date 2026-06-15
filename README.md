# LwIP Software Pack

![tag](https://img.shields.io/badge/tag-2.1.0-brightgreen.svg)
[![release note](https://img.shields.io/badge/release_note-view_html-gold.svg)](https://htmlpreview.github.io/?https://github.com/STMicroelectronics/stm32-mw-lwip/blob/hal2/ST_Release_Notes.html)

## Overview

LwIP (Lightweight IP) is a small independent implementation of the TCP/IP **network protocol** suite, targeted at embedded systems with small RAM and Flash memory.

A general overview of LwIP is available in the [README](README) file.

ST provides adaptation layers for LwIP to run on some STM32 microcontrollers:

- RTOS support (FreeRTOS)
- STM32 network interface (Ethernet)

## Description and Usage

LwIP documentation is available at [http://nongnu.org/lwip/](http://nongnu.org/lwip/). 

The sources for this documentation are available locally under `doc/doxygen` directory and can be generated with [doxygen](https://doxygen.nl/) tool with configuration file `doc/doxygen/lwip.Doxyfile`.

When updating from a previous LwIP release, check [UPGRADING](./UPGRADING) file.
