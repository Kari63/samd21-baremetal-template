SAMD21 DMAC source review and evidence package
Prepared: 2026-10-05
Device: ATSAMD21J18A, user-reported silicon revision D

START HERE
SUPPORT_REQUEST.txt is the English support request.
EVIDENCE.txt contains the observed descriptor/register values, known-failing
sequence and limitations of the evidence.
source/ contains byte-for-byte copies of selected original workspace sources.
SOURCE_MANIFEST.csv records relative paths, byte counts and SHA-256 hashes.

SCOPE
This is a focused source review package, NOT a standalone buildable reproducer.
The selected files reference other application/driver headers and services.
The complete original Atmel Studio project can be supplied separately if needed.
No downloaded SDK/device-pack code, credentials, debugger serial number, or
current ELF/HEX binaries are bundled. Current binaries would not represent all
historical failure variants. No firmware build or flash is performed to create
this package.

SOURCE VERSION
The included ADC_DMA_CHANNEL constant is 1. Functions retaining "channel6" in
their names and diagnostics retaining "ch6" labels now select channel 1 via
ADC_DMA_CHANNEL. This is a legacy naming issue, not an actual ch6 selection.

The current SPI normal path has NO SWRST and NO CHCTRLA=0 disable writes.
It retains flag clearing, descriptor construction, DMB, PRIMASK protection and
RX-before-TX enable order. This is the latest experiment, with no confirmed
long-run result at preparation time. See EVIDENCE.txt for the immediately
preceding known-failing version, which still performed normal-path disable
writes. Error-path disables and ADC TERR recovery remain in the original code.

SOURCE READING ORDER
1. include/hal/dma/dma.h: channel constant, descriptor layout, callbacks.
2. src/hal/dma/dma.cpp: descriptor tables, ADC links, SPI setup and DMAC ISR.
3. src/hpl/adc/adc_init.cpp and src/hpl/timer/tc3_init.cpp.
4. src/hal/evsys/evsys.cpp: evsys_route_tc3_to_adc().
5. src/hpl/sercom/sercom5_init.cpp and src/hal/devif/spi5.cpp.
6. src/drivers/tft/dma_tft.cpp: block scheduling and diagnostic snapshots.
7. src/drivers/tft/tft.cpp: SPI_endWrite() and TFT_setAddrWindow().
8. examples/support/tft_tests.cpp: app2() is the actual stress workload.
9. main.cpp, src/core/systeminit.cpp, src/app/app_run.cpp and project_config.h.
10. Device_Startup/system_samd21.c: clock configuration.

The original full stress function is retained in tft_tests.cpp but app_run()
calls app2(). EXAMPLE_SELECT=0, so example_init() does not arm the ch0 RAM test.
USE_DMA_RAM_TEST=1 alone does not mean that DMA channel 0 is active.
Flash/QSPI testing is disabled in the included configuration.

DIAGNOSTIC CAUTIONS
- Deep TFT channel snapshots are meaningful only when
  tft_dma_stall_snapshot_valid=1. An ADC-only stall does not populate them.
- PRIMASK does not stop DMA; a multiword live read may not be atomic.
- CHID is restored after manual selected-channel inspection before resuming.
- Do not read ADC RESULT or SPI DATA merely to inspect a failure: these reads
  consume peripheral data and can change the observed state.
- Source comments include historical debugging notes. Actual code/register
  values take precedence over unverified explanations in those comments.

REFERENCES
Microchip DS80000760G, section 1.7.2, page 15:
https://ww1.microchip.com/downloads/en/DeviceDoc/SAM-D21DA1-Family-Silicon-Errata-and-Data-Sheet-Clarification-DS80000760G.pdf
The document's G suffix is its document revision, not the silicon revision.
The latest published document revision has not been established here.