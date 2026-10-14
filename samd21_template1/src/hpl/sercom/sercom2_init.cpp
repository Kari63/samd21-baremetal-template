#include "samd21.h"
#include "sercom_init.h"
//#include "sercom2_init.h"

void sercom2_init(void)
{
    /* 0. Disable before configuration */
    SERCOM2->USART.CTRLA.bit.ENABLE = 0;
    while (SERCOM2->USART.SYNCBUSY.bit.ENABLE);

    /* 1. APBCMASK: SERCOM2 power-domain */
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM2;
	while (GCLK->STATUS.bit.SYNCBUSY); 

    /* 2. GCLK: SERCOM2 core clock */
    GCLK->CLKCTRL.reg = (uint16_t)(
        GCLK_CLKCTRL_ID(SERCOM2_GCLK_ID_CORE) |
        GCLK_CLKCTRL_GEN_GCLK0 |
        GCLK_CLKCTRL_CLKEN
    );
	 while (GCLK->STATUS.bit.SYNCBUSY); 

    /* 3. SWRST */
    SERCOM2->USART.CTRLA.bit.SWRST = 1;
    while (SERCOM2->USART.SYNCBUSY.bit.SWRST);

    /* 4. PINMUX: PA10 (TX, PAD2) and PA11 (RX, PAD3), function D (SERCOM-ALT)
       PMUX[n] holds two pins: n = pin >> 1, even pin -> PMUXE, odd pin -> PMUXO
         PA10 (even) -> PMUX[5].PMUXE
         PA11 (odd)  -> PMUX[5].PMUXO
       Which pin is TX or RX is chosen by TXPO/RXPO in CTRLA, not by the PMUX.
    */
    PORT->Group[0].PINCFG[10].bit.PMUXEN = 1;   // PA10
    PORT->Group[0].PINCFG[11].bit.PMUXEN = 1;   // PA11

    PORT->Group[0].PMUX[10 >> 1].reg =
        PORT_PMUX_PMUXE_D |   // PA10 → TX (PAD2)
        PORT_PMUX_PMUXO_D;    // PA11 → RX (PAD3)

    /* 5. CTRLA: MODE + TXPO/RXPO */
    SERCOM2->USART.CTRLA.reg =
        SERCOM_USART_CTRLA_MODE_USART_INT_CLK |
        SERCOM_USART_CTRLA_RXPO(3) |   // RX = PAD3 = PA11
        SERCOM_USART_CTRLA_TXPO(1) |   // TX = PAD2 = PA10
        SERCOM_USART_CTRLA_DORD;

    /* 6. CTRLB: RXEN + TXEN */
    SERCOM2->USART.CTRLB.reg =
        SERCOM_USART_CTRLB_RXEN |
        SERCOM_USART_CTRLB_TXEN;

    while (SERCOM2->USART.SYNCBUSY.bit.CTRLB);

    /* 7. BAUD: 9600 @ 48 MHz */
    SERCOM2->USART.BAUD.reg = 65326;

    /* 8. ENABLE */
    SERCOM2->USART.CTRLA.bit.ENABLE = 1;
    while (SERCOM2->USART.SYNCBUSY.bit.ENABLE);

    /* 9. IRQ: RXC + TXC */
    SERCOM2->USART.INTENSET.reg =
        SERCOM_USART_INTENSET_RXC |
        SERCOM_USART_INTENSET_TXC;

    NVIC_EnableIRQ(SERCOM2_IRQn);
    NVIC_SetPriority(SERCOM2_IRQn, 2);
}
