/* usart3.cpp - SERCOM3 USART = console (EDBG virtual COM, PA22/PA23), copy of usart4 */
#include "project_config.h"

#if USE_UART3

#include "samd21.h"
#include "usart3.h"
#include "systick.h"
#include <stdbool.h>


#define RB3_SIZE 256

// Unique names for the SERCOM3 buffers
static volatile uint8_t  rx3_buffer[RB3_SIZE];
static volatile uint32_t rx3_buffer_head = 0;
static volatile uint32_t rx3_buffer_tail = 0;

static volatile uint8_t  tx3_buffer[RB3_SIZE];
static volatile uint32_t tx3_buffer_head = 0;
static volatile uint32_t tx3_buffer_tail = 0;

static volatile bool tx3_busy = false;

void SERCOM3_Handler(void)
{
	uint8_t flags = SERCOM3->USART.INTFLAG.reg;

	/* RX side */
	
	   if (flags &  SERCOM_USART_INTFLAG_RXC)

	{
		(void)SERCOM3->USART.STATUS.reg;
		uint8_t incoming = SERCOM3->USART.DATA.reg;

		uint32_t next = (rx3_buffer_head + 1) & 0xFF;

		if (next != rx3_buffer_tail)
		{
			rx3_buffer[rx3_buffer_head] = incoming;
			rx3_buffer_head = next;
		}
		 usart3_has_data = true;   // signal for the SysTick soft IRQ
	}

	/* TX side */
	if (flags & SERCOM_USART_INTFLAG_DRE)
	{
		   SERCOM3->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;

		if (tx3_buffer_tail != tx3_buffer_head)
		{
			uint8_t byte = tx3_buffer[tx3_buffer_tail];
			tx3_buffer_tail = (tx3_buffer_tail + 1)& 0xFF;
			SERCOM3->USART.DATA.reg = byte;
		}
		else
		{
			tx3_busy = false;
		}  
	
	
	
		
	}
}

void usart3_write_char(uint8_t data)
{
	uint32_t next = (tx3_buffer_head + 1) & 0xFF;
	while (next == tx3_buffer_tail);

	tx3_buffer[tx3_buffer_head] = data;
	tx3_buffer_head = next;

	__disable_irq();
	if (!tx3_busy)
	{
		tx3_busy = true;

		if (tx3_buffer_tail != tx3_buffer_head)
		{
			uint8_t byte = tx3_buffer[tx3_buffer_tail];
			tx3_buffer_tail = (tx3_buffer_tail + 1) & 0xFF;
		SERCOM3->USART.DATA.reg = byte;
		}
		
		
	}
	__enable_irq();
}

void usart3_write_string(const char *s)
{
	while (*s)
	usart3_write_char((uint8_t)*s++);
}

int usart3_read_char(uint8_t *out)
{
	if (rx3_buffer_tail == rx3_buffer_head)
	return 0;

	*out = rx3_buffer[rx3_buffer_tail];
	rx3_buffer_tail = (rx3_buffer_tail + 1) & 0xFF;
	 return 1;
}

void usart3_write_int(int16_t v)
{
	char buf[8];
	int pos = 0;
	int32_t w = v;   // int32: -32768 negates without overflow

	if (w < 0) {
		usart3_write_char('-');
		w = -w;
	}

	// collect the digits in reverse
	do {
		buf[pos++] = '0' + (w % 10);
		w /= 10;
	} while (w > 0);

	// print them in the right order
	while (pos--)
	usart3_write_char(buf[pos]);
}

void usart3_write_hex(uint8_t v)
{
	const char hex_chars[] = "0123456789ABCDEF";

	uint8_t hi = (v >> 4) & 0x0F;
	uint8_t lo = v & 0x0F;

	usart3_write_char(hex_chars[hi]);
	usart3_write_char(hex_chars[lo]);
}


void usart3_write_crlf(void)
{
	usart3_write_char('\r');
	usart3_write_char('\n');
}

#endif /* USE_UART3 */
