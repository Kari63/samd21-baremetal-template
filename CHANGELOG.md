# Changelog

All notable changes to this template. Versions follow
`TEMPLATE_VERSION_*` in `samd21_template1/include/project_config.h`.

## [0.10.0] - 2026-10-20

### Changed (board wiring / configuration)

- **Serial console moved to the EDBG virtual COM port.** USART3 = SERCOM3 on
  PA22 (TX) / PA23 (RX), function C, 115200 8N1. This is the COM port of the
  Xplained Pro USB debug cable, so no separate USB-UART adapter is needed.
  The terminal must assert DTR (PuTTY does by default). The console was on
  USART4 (SERCOM4, PB10/PB11) before.
- **TFT SPI moved from SERCOM3 to SERCOM1** on the same pins (PA18 MOSI /
  PA19 SCK, now function C instead of D), to free SERCOM3 for the console.
  No rewiring is needed. Files: `sercom3_init.cpp` -> `sercom1_init.cpp`,
  `spi3` -> `spi1`; the DMAC trigger is `SERCOM1_DMAC_ID_TX`.
- USART4 stays as a driver (`USE_UART4`), but it no longer wakes the console.
- RTC: the ALARM0 interrupt is removed. Its `SYNCBUSY` wait in the handler
  blocked the CPU ~6 ms once per second. `CLOCK` is now read in continuous
  read-synchronization mode without waiting.

### Added

- I2C transfer API (`i2c_write`, `i2c_read`, `i2c_write_read`) with a timeout
  on every wait: a missing sensor no longer hangs the MCU. NACK, bus error,
  arbitration lost and timeout return `false`.
- LCD: a whole update goes as one I2C frame by DMA (DMAC channel 6, SERCOM0 TX,
  `ADDR.LENEN`, `USE_I2C_DMA`). Only the changed characters are sent.
- BMP280 pressure/temperature driver (integer compensation, one burst read).
- DHT20 driver rewritten as a non-blocking state machine; `dht20_read()`
  returns the latest result.
- Console commands: `HELP`, `I2CSCAN` (names known devices, 0x28 = the EDBG),
  `BMP280`, `DHT20`. The banner shows the template name, variant, version
  and build stamp.
- Examples: `EXAMPLE_TFT_CLOCK_ADC` (clock + ADC voltage and bar) and
  `EXAMPLE_TFT_WEATHER` (clock, DHT20, BMP280). The TFT stress example starts
  with a text and rotation test, run as a state machine so that the main loop
  keeps running.
- TFT 5x7 font: `.` `:` `-` `%` `/` `+` (they were drawn as `0`).

### Fixed

- RTC: setting the time or the date (`CTRL.ENABLE` = 0 and 1) cleared
  `READREQ.RCONT`; after that every `CLOCK` read stalled the CPU bus ~5 ms.
  RCONT is set again after the enable (`rtc_read_sync_continuous()`).
- TFT: clipping on all edges in `fillRect` (blocking and DMA), the internal
  line functions, `fillRoundRect` and the bitmap functions. A negative start
  or size used to give a wrong address window or an almost endless loop.
- TFT: `setRotation` swapped `_width` / `_height` the wrong way round
  (rotation 1 left a 32 px stripe).
- TFT: text is always opaque (the whole character cell is written), so new
  text covers the old one without `fillRect` and without flicker. The cursor
  step includes the font size (size 2 text overlapped).
- Console: `Unknown command` went to USART2; `SETTIME` / `SETDATE` values are
  checked before they reach the RTC (format, digits, length, ranges, mode 2
  YEAR 0..63); Enter is not echoed after the answer.
- `usart2/usart4_write_int`: -32768 no longer overflows.
- Formatter backends: `\n` -> `\r\n` only when the previous character was not
  `\r` (no more `\r\r\n`).
- `backend.h`: the `usart4_write_char` declaration had a wrong parameter type
  (a second, undefined C++ overload).

### Removed

- Unused diagnostic switches without an implementation (`DEBUG_PRINT`,
  `PRINT_RESET_CAUSE`, `HARDFAULT_DUMP`, `WDT_ENABLE`, `DEBUG_PINS`).

### Still open

- The ADC/DMAC stall is still being investigated with Microchip support
  (`samd21_template1/support/`).

## [0.9.0] - 2026-10-14

Initial public release.
