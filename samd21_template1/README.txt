samd21_template1 - board notes and VS Code setup
=================================================

Board: SAMD21 Xplained Pro, ATSAMD21J18A silicon revision D
EDBG firmware: 0x03.0x25, build 438 (Sep  4 2017 14:48:36)


PERIPHERAL AND PIN MAP
======================

SERCOM4 - USB-UART adapter #1   TX = PB10 (PAD2)  RX = PB11 (PAD3)  BAUD = 115200
SERCOM2 - USB-UART adapter #2   TX = PA10 (PAD2)  RX = PA11 (PAD3)  BAUD = 9600

USART BAUD register values (arithmetic mode, 48 MHz):

| Target Baud Rate | BAUD Value (Decimal) | BAUD Value (Hex) | Real Error (%) | Notes / Stability |
| :--- | :--- | :--- | :--- | :--- |
| **9600** | `65326` | `0xFF2E` | 0.00 % | Verified PuTTY Connection |
| **19200** | `65116` | `0xFE5C` | 0.00 % | Standard Mid-Speed |
| **38400** | `64696` | `0xFCB8` | 0.00 % | Stable Debug Trace |
| **57600** | `64277` | `0xFB15` | 0.00 % | High-Speed Log Output |
| **115200** | `63018` | `0xF62A` | -0.01 % | Fast Terminal Debug |
| **230400** | `60499` | `0xEC53` | -0.01 % | Ultra-Fast Trace |
| **460800** | `55462` | `0xD8A6` | 0.02 % | Requires clean/short lines |
| **921600** | `45388` | `0xB14C` | 0.02 % | Maximum FTC / DMA Speed |

SERCOM0 - I2C master (PCF8574 / LCD / general I2C)
  SDA = PA08 (PAD0)  SCL = PA09 (PAD1)  ~400 kHz

SERCOM3 - SPI master (TFT ST7735, TX only)  12 MHz
SERCOM5 - SPI master (external SST26 flash)  12 MHz

TFT:   MOSI=PA18 SCK=PA19 (SERCOM3 PAD2/PAD3, function D; TX only, no MISO)
       CS=PA17 DC=PB15 RESET=PA16 (GPIO, see tft.h)
Flash: SERCOM5 MOSI=PB22 (PAD2, D) SCK=PB23 (PAD3, D) MISO=PB16 (PAD0, C)
       CS=PA13 (GPIO)

EXTINT: PA15 button

TC3:  MC0 -> EVSYS -> ADC START (internal event; no PA19 output mux)
TC4:  timer output     TC5: timer output
TCC0: PA04 PWM out, PA05 PWM out
ADC:  PA02 / AIN0

AC: PA04 -> COMP0 in use, PA06 -> COMP1 in use

  Usage:
    COMP0 watches a 1.65 V threshold (32 steps of 63)
      ac_set_threshold(0, 32);
    COMP1 watches a 2.5 V threshold (48 steps)
      ac_set_threshold(1, 48);

  AC physical pins (SAMD21J18):
    PA04 -> COMP0 + (positive input / AIN0)
    PA05 -> COMP0 - (negative input / AIN1) -> low threshold or VREF point
    PA06 -> COMP1 + (positive input / AIN2)
    PA07 -> COMP1 - (negative input / AIN3) -> high threshold or VREF point


FORMATTER - USAGE
=================

Backend codes (the last argument of print()):
  1 = TFT
  2 = LCD
  3 = USART2
  4 = USART4
  5 = SPI flash dynamic packer

Basic use:   print("text", backend);   print("Value=%u", value, backend);

Supported conversions:
  %d  signed decimal     %u  unsigned decimal   %f  fixed-point float
  %c  char               %s  string             %x/%X  hex
  %b  binary             %p  pointer

Examples:
  print("Hello TFT", 1);  print("Temp=%u C", temp, 2);  print("Boot OK\n", 3);


EVSYS: TC4 DMA AND TC6 SOFTWARE EVENT
=====================================

Routing in use (SAMD21J18A, rev D):

  EVSYS channel 1: TC4 MC0 -> DMAC channel 0 and TC5 EVU
  EVSYS channel 3: SWEVT   -> TC6 EVU

Channel indexes in the CHANNEL register are zero based. In the CHANNEL field
of the USER register the value 0 means "no routing"; therefore channel 1 is
encoded as 2 and channel 3 as 4.

A software event has no peripheral EVGEN source. Its channel is set to
EVGEN_NONE, and a SWEVT write produces the event on the selected EVSYS
channel. TC6 EVU is connected to the same channel through the USER register.

SAMD21 SWEVT restrictions:

  - A software event does not work on the asynchronous path. Use the
    synchronous or resynchronized path; the synchronous path is used here.
  - EDGSEL must be set to rising edge.
  - A synchronous channel needs its own EVSYS channel clock. Here GCLK0 is
    connected to channel 3 (GCLK_CLKCTRL_ID_EVSYS_3).
  - EVSYS_CTRL_GCLKREQ is set for the duration of the SWEVT write and cleared
    after it.


VS CODE CONFIGURATION (.vscode/)
================================

VS Code is used as an editor next to Atmel Studio 7: build, flash and debug
use the Atmel Studio toolchain. Files:

  tasks.json            Build (make all), Clean and Flash (atprogram)
  launch.example.json   Debug: OpenOCD + arm-none-eabi-gdb, EDBG/SWD
  c_cpp_properties.json IntelliSense: include paths, defines, compiler
  settings.json         Task buttons in the status bar (refer to the tasks.json labels)

launch.json contains machine-specific absolute paths and is not in the
repository (.gitignore). Copy launch.example.json to launch.json and fix the
paths marked below.

Prerequisite: Debug/Makefile is created only by an Atmel Studio build, and
Debug/ is in .gitignore. In a fresh clone or a new project, run one build in
Atmel Studio first, otherwise the VS Code build task cannot find the Makefile.


Project and machine specific items
----------------------------------

tasks.json
  -f ...\Debug\samd21_template1.hex  project name
  -d ATSAMD21J18A                     device
  -s ATML2130062700000156             EDBG serial number, depends on the board
                                      (find it with: atprogram.exe list)
  Other paths use the ${workspaceFolder} variable.

launch.json (copied from launch.example.json)
  program: ${workspaceFolder}\Debug\samd21_template1.elf  project name
  customLaunchSetupCommands -> "file ..."                 ABSOLUTE PATH, replace
                                                          C:/path/to/ with your folder
  debugServerPath (OpenOCD)                               ${env:USERPROFILE}\... matches
                                                          a WinGet xPack OpenOCD install
  miDebuggerPath (gdb)                                    installation specific
  debugServerArgs: target/at91samdXX.cfg                  whole SAM D/L/C family

c_cpp_properties.json
  defines: __SAMD21J18A__                                 device
  includePath: .../SAMD21_DFP/1.3.395/samd21a/include     DFP; the same for the
                                                          whole D21 series, changes
                                                          with the family (D11, D51)
  compilerPath, CMSIS path                                installation specific

settings.json
  No project specific content.


Pitfall: the "file" command in launch.json
------------------------------------------

The GDB "file" command loads the symbols from the .elf file. The path must be
absolute and written with forward slashes:

  "text": "file \"C:/Users/<user>/.../samd21_template1/Debug/samd21_template1.elf\""

Tried and found not to work:
  - Relative path (file Debug/samd21_template1.elf): GDB is not started in the
    project folder (the "cwd" of launch.json applies to the target program,
    not to GDB) -> "No such file or directory".
  - ${workspaceFolder}: expands with backslashes on Windows (C:\Users\...),
    and GDB treats the backslashes as escape characters -> broken path.

The path therefore always has to be fixed by hand when the project is copied
or moved.


Using the project as a base for a new one
-----------------------------------------

1. Copy the .vscode/ files into the new project (launch.example.json -> launch.json).
2. Find & replace: samd21_template1 -> name of the new project.
3. Fix the absolute path of the "file" command in launch.json to match the
   folder of the new project (find & replace only changes the name, not the
   folder path).
4. If the device changes: ATSAMD21J18A and __SAMD21J18A__, and the DFP path
   if needed.
5. If the debug board changes: the -s serial number in tasks.json.
6. Run the first build in Atmel Studio so that Debug/Makefile is created.


License
-------

Own code: Apache-2.0 (LICENSE in the repository root).
Third-party components and their licenses are listed in NOTICE, the license
texts are in the LICENSES/ folder:
- Device_Startup/: Microchip, Apache-2.0
- TFT driver (tft.h, tft.cpp): Adafruit-GFX (BSD) and Adafruit-ST7735 (MIT)
- Free*.h fonts: GNU FreeFont, GPL-3.0-or-later + font exception
