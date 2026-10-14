/*
 * dma.cpp
 *
 * Created: 18.9.2026 13.17.31
 *  Author: Kari63
 */ 
#include "project_config.h"

#if USE_DMA

#include "samd21.h"
#include "dma.h"
#include <stddef.h>
#include "adc_read.h"

volatile uint8_t dma_transfer_complete = 0;
volatile uint8_t dma_transfer_error = 0;
volatile uint32_t dma_tcmpl_count = 0;   //  channel 0 (TC4 trigger) TCMPL hits
volatile uint32_t adc_dma_block_count = 0;
volatile uint32_t adc_dma_error_count = 0;



// Registered callback reports channel 4 TX completion or error.
static dma_spi_complete_cb_t s_spi_complete_cb = NULL;
static dmac_channel_cb_t s_channel_callbacks[DMA_CHANNELS];

static void dmac_adc_callback(uint8_t channel, uint8_t flags);
static void dmac_spi_callback(uint8_t channel, uint8_t flags);


// The DMAC requires 128-bit (16-byte) alignment for BASEADDR, WRBADDR and
// linked descriptors (DESCADDR).
// The descriptors are volatile: the DMAC reads them, so the compiler must not
// reorder or defer the descriptor writes past the channel enable.

static volatile dmac_descriptor_t descriptor_section[DMA_CHANNELS] __attribute__((aligned(16)));
static volatile dmac_descriptor_t writeback_section[DMA_CHANNELS] __attribute__((aligned(16)));
static volatile dmac_descriptor_t adc_ping_pong_descriptor __attribute__((aligned(16)));
static volatile uint16_t adc_dma_buffers[2][ADC_DMA_BUFFER_SAMPLES];
static volatile uint8_t adc_completed_buffer = 0;




 void dmac_init(void)
{
	// Enable the DMAC bus clocks (AHB + APBB)
	PM->AHBMASK.reg  |= PM_AHBMASK_DMAC;
	PM->APBBMASK.reg |= PM_APBBMASK_DMAC;

	// Point the controller to the 16-byte aligned tables
	DMAC->BASEADDR.reg = (uint32_t)descriptor_section;
    DMAC->WRBADDR.reg  = (uint32_t)writeback_section;

	// Global enable, all 4 priority levels enabled
	DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xF);
	dmac_set_channel_callback(ADC_DMA_CHANNEL, dmac_adc_callback);
	dmac_set_channel_callback(SPI_DMA_CHANNEL, dmac_spi_callback);
	NVIC_EnableIRQ(DMAC_IRQn);
}

bool dmac_set_channel_callback(uint8_t channel, dmac_channel_cb_t cb)
{
	if (channel >= DMA_CHANNELS) {
		return false;
	}

	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	s_channel_callbacks[channel] = cb;
	__set_PRIMASK(primask);
	return true;
}

void dmac_build_channel_adc_ping_pong(void)
{
	// ADC RESULT -> buffer A -> buffer B -> buffer A ...
	const uint16_t btctrl = DMAC_BTCTRL_VALID |
		DMAC_BTCTRL_DSTINC |
		DMAC_BTCTRL_BEATSIZE_HWORD |
		DMAC_BTCTRL_BLOCKACT_INT;

	descriptor_section[ADC_DMA_CHANNEL].btctrl = btctrl;
	descriptor_section[ADC_DMA_CHANNEL].btcnt = ADC_DMA_BUFFER_SAMPLES;
	descriptor_section[ADC_DMA_CHANNEL].srcaddr = (uint32_t)&ADC->RESULT.reg;
	descriptor_section[ADC_DMA_CHANNEL].dstaddr =
		(uint32_t)&adc_dma_buffers[0][ADC_DMA_BUFFER_SAMPLES];
	descriptor_section[ADC_DMA_CHANNEL].descaddr = (uint32_t)&adc_ping_pong_descriptor;

	adc_ping_pong_descriptor.btctrl = btctrl;
	adc_ping_pong_descriptor.btcnt = ADC_DMA_BUFFER_SAMPLES;
	adc_ping_pong_descriptor.srcaddr = (uint32_t)&ADC->RESULT.reg;
	adc_ping_pong_descriptor.dstaddr =
		(uint32_t)&adc_dma_buffers[1][ADC_DMA_BUFFER_SAMPLES];
	adc_ping_pong_descriptor.descaddr = (uint32_t)&descriptor_section[ADC_DMA_CHANNEL];
	adc_completed_buffer = 0;
}

void dmac_arm_channel_adc_trigger(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	DMAC->CHID.reg = DMAC_CHID_ID(ADC_DMA_CHANNEL);
	DMAC->CHCTRLA.reg = 0;

	// ENABLE reads 1 until an ongoing burst has completed
	while (DMAC->CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) {
	}

	DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL | DMAC_CHINTENSET_TERR;
	DMAC->CHCTRLB.reg =
		DMAC_CHCTRLB_LVL(0) |
		DMAC_CHCTRLB_TRIGACT_BEAT |
		// The DMA listens to the ADC RESRDY request: one request moves one sample.
		DMAC_CHCTRLB_TRIGSRC(ADC_DMAC_ID_RESRDY);

	DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;

	__set_PRIMASK(primask);
}



void dmac_arm_channel_spi_trigger(void)
{
	DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMA_CHANNEL);
	DMAC->CHCTRLA.reg = 0;
	DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL | DMAC_CHINTENSET_TERR;
	DMAC->CHCTRLB.reg =
	DMAC_CHCTRLB_LVL(0) |
	DMAC_CHCTRLB_TRIGACT_BEAT |                 // one byte per DRE request
	DMAC_CHCTRLB_TRIGSRC(SERCOM3_DMAC_ID_TX);   // TFT TX, direct peripheral trigger

	DMAC->CHCTRLA.reg = DMAC_CHCTRLA_ENABLE;
}

void dmac_build_channel_color_block(const uint8_t *data, uint16_t len)
{
	descriptor_section[SPI_DMA_CHANNEL].btctrl = DMAC_BTCTRL_VALID |
	DMAC_BTCTRL_SRCINC |
	DMAC_BTCTRL_BEATSIZE_BYTE |
	DMAC_BTCTRL_BLOCKACT_INT;
	descriptor_section[SPI_DMA_CHANNEL].btcnt   = len;
	descriptor_section[SPI_DMA_CHANNEL].srcaddr = (uint32_t)data + len;
	descriptor_section[SPI_DMA_CHANNEL].dstaddr = (uint32_t)&SERCOM3->SPI.DATA.reg;
	descriptor_section[SPI_DMA_CHANNEL].descaddr = 0;
}

void dmac_start_spi_transfer(const uint8_t *data, uint16_t len)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	dmac_build_channel_color_block(data, len);
	dmac_arm_channel_spi_trigger();

	__set_PRIMASK(primask);
}

void dma_spi_set_complete_callback(dma_spi_complete_cb_t cb)
{
	s_spi_complete_cb = cb;
}

static void dmac_adc_callback(uint8_t channel, uint8_t flags)
{
	(void)channel;

	if (flags & DMAC_CHINTFLAG_TERR) {
		dma_transfer_error = 1;
		adc_dma_error_count++;
		dmac_build_channel_adc_ping_pong();
		dmac_arm_channel_adc_trigger();
		return;
	}

	if (flags & DMAC_CHINTFLAG_TCMPL) {
		adc_dma_samples_complete(adc_dma_buffers[adc_completed_buffer],
			ADC_DMA_BUFFER_SAMPLES);
		adc_completed_buffer ^= 1;
		adc_dma_block_count++;
	}
}


static void dmac_spi_callback(uint8_t channel, uint8_t flags)
{
	if (flags & DMAC_CHINTFLAG_TERR) {
		DMAC->CHID.reg = DMAC_CHID_ID(SPI_DMA_CHANNEL);
		DMAC->CHCTRLA.reg = 0;
		if (s_spi_complete_cb) {
			s_spi_complete_cb(false);
		}
		return;
	}

	if (flags & DMAC_CHINTFLAG_TCMPL) {
		if (channel == SPI_DMA_CHANNEL) {
			if (s_spi_complete_cb) {
				s_spi_complete_cb(true);
			}
		}
	}
}

void DMAC_Handler(void)
{
	uint8_t active_channel = DMAC->INTPEND.bit.ID;
	DMAC->CHID.reg = DMAC_CHID_ID(active_channel);
	uint8_t interrupt_flags = DMAC->CHINTFLAG.reg;
	DMAC->CHINTFLAG.reg = interrupt_flags;

	if (active_channel < DMA_CHANNELS && s_channel_callbacks[active_channel]) {
		s_channel_callbacks[active_channel](active_channel, interrupt_flags);
	}
}


#endif /* USE_DMA */
