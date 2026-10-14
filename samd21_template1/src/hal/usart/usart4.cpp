#include "project_config.h"

#if USE_UART4

#include "samd21.h"
#include "usart4.h"
#include "systick.h"
#include <stdbool.h>


#define RB4_SIZE 256

// Unique names for the SERCOM4 buffers
static volatile uint8_t  rx4_buffer[RB4_SIZE];
static volatile uint32_t rx4_buffer_head = 0;
static volatile uint32_t rx4_buffer_tail = 0;

static volatile uint8_t  tx4_buffer[RB4_SIZE];
static volatile uint32_t tx4_buffer_head = 0;
static volatile uint32_t tx4_buffer_tail = 0;

static volatile bool tx4_busy = false;

void SERCOM4_Handler(void)
{
	uint8_t flags = SERCOM4->USART.INTFLAG.reg;

	/* RX side */
	
	   if (flags &  SERCOM_USART_INTFLAG_RXC)

	{
		(void)SERCOM4->USART.STATUS.reg;
		uint8_t incoming = SERCOM4->USART.DATA.reg;

		uint32_t next = (rx4_buffer_head + 1) & 0xFF;

		if (next != rx4_buffer_tail)
		{
			rx4_buffer[rx4_buffer_head] = incoming;
			rx4_buffer_head = next;
		}
		 usart4_has_data = true;   // signal for the SysTick soft IRQ
	}

	/* TX side */
	if (flags & SERCOM_USART_INTFLAG_DRE)
	{
		   SERCOM4->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;

		if (tx4_buffer_tail != tx4_buffer_head)
		{
			uint8_t byte = tx4_buffer[tx4_buffer_tail];
			tx4_buffer_tail = (tx4_buffer_tail + 1)& 0xFF;
			SERCOM4->USART.DATA.reg = byte;
		}
		else
		{
			tx4_busy = false;
		}  
	
	
	
		
	}
}

void usart4_write_char(uint8_t data)
{
	uint32_t next = (tx4_buffer_head + 1) & 0xFF;
	while (next == tx4_buffer_tail);

	tx4_buffer[tx4_buffer_head] = data;
	tx4_buffer_head = next;

	__disable_irq();
	if (!tx4_busy)
	{
		tx4_busy = true;

		if (tx4_buffer_tail != tx4_buffer_head)
		{
			uint8_t byte = tx4_buffer[tx4_buffer_tail];
			tx4_buffer_tail = (tx4_buffer_tail + 1) & 0xFF;
		SERCOM4->USART.DATA.reg = byte;
		}
		
		
	}
	__enable_irq();
}

void usart4_write_string(const char *s)
{
	while (*s)
	usart4_write_char((uint8_t)*s++);
}

int usart4_read_char(uint8_t *out)
{
	if (rx4_buffer_tail == rx4_buffer_head)
	return 0;

	*out = rx4_buffer[rx4_buffer_tail];
	rx4_buffer_tail = (rx4_buffer_tail + 1) & 0xFF;
	 return 1;
}

void usart4_write_int(int16_t v)
{
	char buf[8];
	int pos = 0;

	if (v < 0) {
		usart4_write_char('-');
		v = -v;
	}

	// collect the digits in reverse
	do {
		buf[pos++] = '0' + (v % 10);
		v /= 10;
	} while (v > 0);

	// print them in the right order
	while (pos--)
	usart4_write_char(buf[pos]);
}

void usart4_write_hex(uint8_t v)
{
	const char hex_chars[] = "0123456789ABCDEF";

	uint8_t hi = (v >> 4) & 0x0F;
	uint8_t lo = v & 0x0F;

	usart4_write_char(hex_chars[hi]);
	usart4_write_char(hex_chars[lo]);
}


void usart4_write_crlf(void)
{
	usart4_write_char('\r');
	usart4_write_char('\n');
}

#endif /* USE_UART4 */
