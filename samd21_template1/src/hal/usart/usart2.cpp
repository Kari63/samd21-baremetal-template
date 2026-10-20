#include "project_config.h"

#if USE_UART2

#include "samd21.h"
#include "usart2.h"

#define RB2_SIZE 256

// Unique names for the SERCOM2 buffers
static volatile uint8_t  rx2_buffer[RB2_SIZE];
static volatile uint32_t rx2_buffer_head = 0;
static volatile uint32_t rx2_buffer_tail = 0;

static volatile uint8_t  tx2_buffer[RB2_SIZE];
static volatile uint32_t tx2_buffer_head = 0;
static volatile uint32_t tx2_buffer_tail = 0;

static volatile bool tx2_busy = false;

void SERCOM2_Handler(void)
{
	uint8_t flags = SERCOM2->USART.INTFLAG.reg;

	/* RX side */
	if (flags & SERCOM_USART_INTFLAG_RXC)
	{
		(void)SERCOM2->USART.STATUS.reg;
		uint8_t incoming = SERCOM2->USART.DATA.reg;

		uint32_t next = (rx2_buffer_head + 1)  & 0xFF;
		if (next != rx2_buffer_tail)
		{
			rx2_buffer[rx2_buffer_head] = incoming;
			rx2_buffer_head = next;
		}
	}

	/* TX side */
	if (flags & SERCOM_USART_INTFLAG_DRE)
	{
		SERCOM2->USART.INTFLAG.reg = SERCOM_USART_INTFLAG_TXC;

		if (tx2_buffer_tail != tx2_buffer_head)
		{
			uint8_t byte = tx2_buffer[tx2_buffer_tail];
			tx2_buffer_tail = (tx2_buffer_tail + 1)  & 0xFF;
			SERCOM2->USART.DATA.reg = byte;
		}
		else
		{
			tx2_busy = false;
		}
	}
}

void usart2_write_char(uint8_t data)
{
	uint32_t next = (tx2_buffer_head + 1) & 0xFF;
	while (next == tx2_buffer_tail);

	tx2_buffer[tx2_buffer_head] = data;
	tx2_buffer_head = next;

	__disable_irq();
	if (!tx2_busy)
	{
		tx2_busy = true;

		if (tx2_buffer_tail != tx2_buffer_head)
		{
			uint8_t byte = tx2_buffer[tx2_buffer_tail];
			tx2_buffer_tail = (tx2_buffer_tail + 1) & 0xFF;
			SERCOM2->USART.DATA.reg = byte;
		}
	}
	__enable_irq();
}

void usart2_write_string(const char *s)
{
	while (*s)
	usart2_write_char((uint8_t)*s++); // the ++ is needed, otherwise the loop never ends
}

int usart2_read_char(uint8_t *out)
{
	if (rx2_buffer_tail == rx2_buffer_head)
	return 0;

	*out = rx2_buffer[rx2_buffer_tail];
	rx2_buffer_tail = (rx2_buffer_tail + 1)  & 0xFF;
	return 1;
}


void usart2_write_int(int16_t v)
{
	char buf[8];
	int pos = 0;
	int32_t w = v;   // int32: -32768 negates without overflow

	if (w < 0) {
		usart2_write_char('-');
		w = -w;
	}

	// collect the digits in reverse
	do {
		buf[pos++] = '0' + (w % 10);
		w /= 10;
	} while (w > 0);

	// print them in the right order
	while (pos--)
	usart2_write_char(buf[pos]);
}

void usart2_write_hex(uint8_t v)
{
	const char hex_chars[] = "0123456789ABCDEF";

	uint8_t hi = (v >> 4) & 0x0F;
	uint8_t lo = v & 0x0F;

	usart2_write_char(hex_chars[hi]);
	usart2_write_char(hex_chars[lo]);
}


void usart2_write_crlf(void)
{
	usart2_write_char('\r');
	usart2_write_char('\n');
}

#endif /* USE_UART2 */
