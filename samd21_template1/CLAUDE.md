# CLAUDE.md - samd21_template1

Answer the user in Finnish. Code comments, strings and documentation in the
repository are written in English.

The user is an experienced embedded developer, so the basics do not need
explaining. The project is a learning tool (Cortex-M0+ and SAMD21 internals):
go through the details and ask rather than offer a quick minimal solution.

## Target
- SAMD21J18A, SAMD21 Xplained Pro (silicon revision D, known errata apply)
- QSPI flash replaced on the board: SST26VF/WF series
- Variant: ADC with DMA (RESRDY triggers a DMAC channel, ping-pong buffer,
  ADC IRQ off). The parallel project samd21_template is the DMA-free
  reference (ADC with an interrupt).

## Tools
- Main environment: Atmel Studio 7, project name samd21_template1
  (DFP: Atmel SAMD21_DFP 1.3.395)
- Also tested with Keil (ARMClang V6.24) and the Microchip SAMD21_DFP 2.0.4 pack.
  The new headers of the DFP 3.x packs (GCLK_REGS etc.) do NOT fit this project.
- Do not run a build or change project files (.cppproj, .atsln) unless asked.

## Git workflow
- Repo: Kari63/samd21_template1 (private), root at solution level
  (.atsln + project folder). .gitignore is in the repository root.
- Claude pushes directly to main. The user does Fetch + Pull in VS Code.
- The user pushes their own commits before a request; Claude fetches a fresh
  main before editing.
- New file/folder or a .cppproj change: the user runs Rebuild in Atmel Studio
  before Claude commits (Debug/Makefile gets updated).
- .vscode/launch.json is local only (.gitignore); changes that belong in the
  repository go to .vscode/launch.example.json.
- New source files: use "Author: Kari63" in the header (Atmel Studio inserts
  the Windows user name).

## Principles
- Bare metal, no ASF, Harmony or MPLAB Melody
- Atmel CMSIS headers only (Atmel style: GCLK->..., .reg and .bit).
  Bit fields (.bit) are allowed and used. No hard-coded shifts:
  use the _Pos/_Msk and value macros from the headers.
- Exception: INTFLAG and INTENCLR are always written with .reg. A .bit write
  is a read-modify-write: on INTFLAG it acknowledges all pending flags, on
  INTENCLR it disables all enabled interrupts.
- All own code is .cpp. Only the vendor startup and system_samd21.c are .c.
  Do not suggest extern "C" wrappers by reflex.
- Industrial/automotive style: static, compile-time configuration, no
  runtime reconfiguration. No Arduino-style solutions.
- The C++ class layer (api/) is a later stage, not yet.
- No "fix it first" patches and no watchdog to hide faults: find the root cause.

## Architecture
- Clocks and NVIC: SystemInit (system_samd21.c), not on the main side
- Layers: hpl/ (device-level drivers and peripheral inits: SERCOM, timers,
  RTC, AC, ADC) -> hal/ (timerX, i2c, spi ...) -> drv/ -> (later) api/
- eic_init() is called from systeminit; per line eic_pin_config(GpioPin, sense,
  flags, cb) in the application (app_init). The EXTINT line comes from eic_extint_map.h.
- board_pins.h: named pins; gpio_pins.h: PORTA/PORTB/PA()/PB() helpers
- print() formatter: src/core/formatter/ (formatter.cpp + backend.cpp).
  The last variadic argument selects the backend (TFT, I2C LCD, USART2,
  USART4, SPI flash). The build must also work with USE_TFT 0.
- evsys.cpp: evsys_channel_configure(channel, generator_id, path) and
  evsys_user_configure(user_id, channel); the USER.CHANNEL +1 trick is hidden inside.
- The SPI bus is shared between the TFT and the flash.
- LCD (HD44780 + PCF8574): own driver with a shadow buffer; LCD_update() runs
  from the main loop (lcd_softirq), not from the TC4 interrupt.

## Known pitfalls (verified on hardware)
- EVSYS CHANNEL register: select the channel with an 8-bit write and write
  SWEVT separately as 16 bits. A 32-bit read-modify-write leaks the EVGEN/PATH
  bits of the previous channel into the new channel.
- SWEVT: needs the sync or resync path (not async), EDGSEL rising, its own
  GCLK for the channel, CTRL.GCLKREQ only for the duration of the SWEVT write.
- TC/TCC do not work as EVSYS users on the SYNC/RESYNC path (official erratum)
  -> async for TC users.
- The TC COUNT must be read through READREQ synchronization, otherwise the
  value is frozen.
- The SERCOM TXC flag stays set regardless of INTENSET: clear it first, then
  enable the interrupt.
- An interrupt handler must always clear INTFLAG (otherwise interrupt storm).
- DAC: official erratum on the EMPTY flag after waking from standby.
- DMA pays off only for large transfers (fillScreen/fillRect/V/HLine);
  on short lines the setup cost eats the gain.
- QSPI SST26VF/WF unofficial quirks: GBULK needs WREN, WRR writes SR1+SR2,
  WIP does not rise immediately after an erase (~25 ms delay), sector erase is
  4 kB aligned, page program does not cross a 256-byte boundary, SR2 may
  restore the protection, SFDP does not match, the JEDEC ID does not tell the
  variant apart, DPD exit takes 1-2 ms.

## Clocks and measurements
- GCLK2 = OSC8M (8 MHz), TC prescaler DIV1024 -> 128 us tick
- Because of OSC8M, ~2 % systematic difference to the SysTick-based delay_ms()
- SERCOM SPI TFT: 12 MHz (BAUD=1)

## Status
- Done and tested: SERCOM (SPI, I2C, USART), RTC mode 0/2, QSPI flash,
  ADC, AC, TC/TCC, EIC, EVSYS, DMAC
- Missing: DAC, I2S. USB and digital audio are out of scope.
- Open: rare ADC freeze after adding ADC DMA
  (the DMAC channel order matters, Microchip tickets open).
