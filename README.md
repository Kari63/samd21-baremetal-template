# samd21_template1

A bare-metal C++ firmware template for the **ATSAMD21J18A** (SAMD21 Xplained Pro),
written directly against the Atmel CMSIS device headers. It has no ASF, no Harmony,
no MPLAB Melody and no Arduino layer.

The template is a learning project about the Cortex-M0+ and the SAMD21 internals.
Every peripheral is set up register by register, and the hardware pitfalls found
along the way are written down below. Many of them are not in the datasheet.

Version 0.9.0.

## Hardware

- SAMD21 Xplained Pro, ATSAMD21J18A, silicon **revision D**
- External SPI flash replaced with an **SST26VF/WF** series part
- ST7735 128x160 TFT (SPI)
- HD44780 16x2 character LCD on a PCF8574 I2C backpack
- Optional: DHT20 temperature/humidity sensor, DRV8833 motor driver

Pin and peripheral map: [samd21_template1/README.txt](samd21_template1/README.txt).

## What is included

| Area | Content |
| --- | --- |
| Clocks | XOSC32K -> DFLL48M closed loop (48 MHz CPU), OSC8M and DFLL/6 for timers, all in `SystemInit()` |
| SERCOM | USART x2 (ring buffers, interrupts), I2C master, SPI master x2 |
| Timers | TC3..TC7 and TCC0: periodic, event counting, capture, pulse-width (PPW) capture, PWM |
| RTC | Mode 0 and mode 2 (calendar) with alarm, set from a serial command console |
| EIC / EVSYS | Per-pin EXTINT configuration from a pin table; EVSYS channels/users including software events |
| ADC | Event-triggered conversions, results moved by DMAC into ping-pong buffers |
| AC | Analog comparators with adjustable thresholds |
| DMAC | ADC ping-pong, SPI TX for DMA-accelerated TFT fills, memory-to-memory test |
| Displays | ST7735 driver (Adafruit-GFX based, GFXfont support); HD44780 driver with a shadow buffer |
| SPI flash | Two SST26 drivers on purpose: a full command-level driver (`sst26_flash`) and a lightweight self-contained one (`flash_wr`); chip/sector erase, page program, and a "dynamic packer" that streams text to flash |
| `print()` | Small printf-style formatter; the last argument selects the output (TFT, LCD, USART2, USART4, flash) |

Not implemented: DAC, I2S. USB and digital audio are out of scope.

## Design rules

- **Static, compile-time configuration.** Nothing is reconfigured at runtime.
  Features are switched in [`project_config.h`](samd21_template1/include/project_config.h)
  (`USE_xxx`), and dependency errors fail the build.
- **CMSIS headers only**, Atmel style (`GCLK->...`, `.reg` / `.bit`), with the
  `_Pos` / `_Msk` / value macros from the headers. No hard-coded shifts.
- `INTFLAG` and `INTENCLR` are always written with `.reg`. A `.bit` write is a
  read-modify-write: on `INTFLAG` it acknowledges every pending flag, and on
  `INTENCLR` it disables every enabled interrupt.
- Interrupt handlers stay short. Slow work, such as the LCD update over I2C,
  runs in the main loop through soft-IRQ flags.
- Faults are fixed at the root cause. There are no "fix it later" patches and
  no watchdog hiding problems.

## Project layout

```
samd21_template1.atsln          Atmel Studio 7 solution
LICENSE, NOTICE, LICENSES/      licensing
samd21_template1/
  Device_Startup/               vector table, Reset_Handler, SystemInit (clocks), linker scripts
  include/, src/
    hpl/                        peripheral init: SERCOM, timers, RTC, ADC/AC, EIC, GPIO
    hal/                        peripheral drivers: USART, I2C, SPI, timers, DMAC, EVSYS, flash
    drivers/                    devices: TFT, LCD, DHT20, DRV8833
    core/                       print() formatter, soft-IRQ dispatch, command console, systeminit
    api/                        application-level helpers (LCD page, flash text packer)
    app/, callbacks/            application init/run and timer callbacks
  examples/                     one example is built, selected with EXAMPLE_SELECT
  support/                      material for the open Microchip support cases
  .vscode/                      optional VS Code tasks/debug setup (see README.txt)
```

Examples (`EXAMPLE_SELECT` in `project_config.h`): TFT stress test, DMA TFT fills,
EVSYS software event + TC capture (main-loop timing), EIC -> EVSYS -> TC7 pulse
width, DMA RAM-to-RAM test, SST26 flash test, and the SST26 dynamic text packer.

## Building

- **Atmel Studio 7** with **SAMD21_DFP 1.3.395** and CMSIS 5.4.0 (main environment).
  Open `samd21_template1.atsln` and build.
- Also tested with Keil MDK (ARMClang 6.24) and the Microchip SAMD21_DFP 2.0.4 pack.
- The DFP 3.x packs use new header structures (`GCLK_REGS` etc.) and do **not**
  work with this code.
- VS Code can be used as an editor with build/flash/debug tasks. See
  [README.txt](samd21_template1/README.txt) for the setup and its pitfalls.

### Porting to CMake / another build system

No Makefile or CMake file is included; Atmel Studio generates `Debug/Makefile`
from the project. The settings used by the Debug build (GCC 6.3.1, Atmel
toolchain):

- **Sources:** all `.c` / `.cpp` files listed in `samd21_template1/samd21_template1.cppproj`
  (`<Compile Include=...>`), i.e. everything under `src/`, `examples/`,
  `Device_Startup/` and `main.cpp`.
- **Include paths:** `include/` and **every subfolder of it** (headers are
  included by file name only), plus the DFP `SAMD21_DFP/1.3.395/samd21a/include`
  and CMSIS `5.4.0/CMSIS/Core/Include`.
- **Defines:** `__SAMD21J18A__`, `DEBUG`.
- **Compiler:** `-mcpu=cortex-m0plus -mthumb -Og -g3 -Wall -ffunction-sections
  -fno-rtti -fno-exceptions -mlong-calls` (Release uses `-Os`).
- **Linker:** `-mcpu=cortex-m0plus -mthumb -Wl,--gc-sections -lm
  -L Device_Startup -T samd21j18a_flash.ld`.
- Feature selection is done in `include/project_config.h`; all sources are
  always compiled. Most modules are guarded by their `USE_xxx` switch, and
  unused code is removed by `--gc-sections`.

## Hardware-verified pitfalls

These were all found on real hardware (rev D). They are the most useful part
of this repository for anyone else working with the SAMD21.

**EVSYS**
- Select the `CHANNEL` register channel with an **8-bit** write, and write `SWEVT`
  separately as 16 bits. A 32-bit read-modify-write leaks the EVGEN/PATH bits
  of the previously selected channel into the new one.
- Software events (`SWEVT`) need the synchronous or resynchronized path (not
  asynchronous), EDGSEL rising, a GCLK of their own for the channel, and
  `CTRL.GCLKREQ` set only while writing `SWEVT`.
- `USER.CHANNEL` is the channel number **+1** (0 = not connected). This is hidden
  inside `evsys_user_configure()`.
- TC/TCC do not work as event users on the SYNC/RESYNC path (official erratum),
  so TC users use the asynchronous path.

**TC / SERCOM / interrupts**
- TC `COUNT` and `CC` must be read through `READREQ`, otherwise a frozen value is
  returned (the registers are in another clock domain).
- The SERCOM `TXC` flag stays set regardless of `INTENSET`: clear it first, then
  enable the interrupt.
- An interrupt handler must always clear `INTFLAG`, otherwise you get an interrupt storm.
- DMAC priority levels (`CHCTRLB.LVL`) only arbitrate between DMA channels. They
  have nothing to do with NVIC priorities: the DMAC interrupt still waits behind
  any other handler at the same NVIC priority.

**DMAC**
- Descriptor and write-back tables must be 16-byte aligned.
- Erratum 1.7.2: while a channel with linked descriptors is active, a channel
  with a **lower** number must not be enabled.
- DMA pays off only for large transfers (fillScreen / fillRect / long lines);
  for short transfers the setup cost eats the gain.

**SST26VF/WF SPI flash (unofficial quirks)**
- `GBULK` (global block unlock) needs `WREN` first.
- `WRR` writes SR1 **and** SR2, and it locks the blocks again, so `GBULK` is needed after it.
- `WIP` does not rise immediately after an erase. Wait ~25 ms before polling.
- Sector erase is 4 kB aligned, and a page program must not cross a 256-byte boundary.
- SR2 may bring the protection back. SFDP does not match the datasheet, and the
  JEDEC ID does not tell the VF and WF variants apart.
- Exiting deep power-down takes 1-2 ms.

**HD44780 over PCF8574**
- The PCF8574 updates its outputs at every data-byte ACK, so a whole LCD byte
  (both nibbles and the E pulses) fits in one I2C frame. The frame gaps also
  cover the HD44780 execution time, which is checked at compile time. With a
  shadow buffer only the changed characters are sent, so a typical update takes
  well under a millisecond.

## Open issues

A rare **ADC/DMAC stall** after adding ADC DMA is being investigated with
Microchip support. The DMAC channel order matters. The material sent to
Microchip is in [`samd21_template1/support/`](samd21_template1/support/).

## License

Own code: **Apache-2.0** ([LICENSE](LICENSE)). Third-party parts are listed in
[NOTICE](NOTICE), with the license texts in [LICENSES/](LICENSES/):

- `Device_Startup/` startup and linker scripts: Microchip, Apache-2.0
- TFT driver: based on Adafruit-GFX (BSD) and Adafruit-ST7735 (MIT)
- `Free*.h` fonts: GNU FreeFont, GPL-3.0-or-later with font exception
