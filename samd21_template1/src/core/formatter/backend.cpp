

#include "project_config.h"

#include "backend.h"
#include "tft.h"
#include "lcdpcf8574.h"
#include "usart2.h"
#include "usart3.h"
#include "samd21.h"
#include "dyn_flash_formatter_write.h"

// --- TFT BACKEND ---



void fb_putc(uint8_t backend, char c)
{
	switch (backend)
	{
		
		case 1: // === TFT ===
		#if USE_TFT
		{
			if (c == '\n')
			{
				SPI_startWrite();
				TFT_gotoNextLine(cursorLineStartX);
				SPI_endWrite();
				return;
			}

			uint8_t cx = TFT_getCursorX();
			uint8_t cy = TFT_getCursorY();
			uint8_t ch = static_cast<uint8_t>(c);

			SPI_startWrite();
			uint8_t w = TFT_drawChar(cx, cy, ch);
			SPI_endWrite();

			// drawChar returns the advance including the font size
			// (default font 6 x size, GFXfont xAdvance x size).
			// 0 = character not in the GFXfont: fallback monospace step
			TFT_setCursor(cx + (w ? w : 6), cy);
		}
		#endif
		break;


		case 2: // === I2C LCD ===
		#if USE_LCD 
		{
			if (c == '\n')
			{
				lcd_gotoxy(0, 1);
			}
			else
			{
				lcd_putc(c);
			}
		}
		#endif
		break;

		case 3: // === USART2 ===
		#if USE_UART2
		{
			// "\n" -> "\r\n", an existing "\r\n" stays (no "\r\r\n")
			static char prev = 0;
			if (c == '\n' && prev != '\r')
			{
				usart2_write_char('\r');
			}
			usart2_write_char(static_cast< uint8_t>(c));
			prev = c;
		}
		#endif
		break;

		case 4: // === USART3 = EDBG virtual COM ===
		#if USE_UART3
		{
			// "\n" -> "\r\n", an existing "\r\n" stays (no "\r\r\n")
			static char prev = 0;
			if (c == '\n' && prev != '\r')
			{
				usart3_write_char('\r');
			}
			usart3_write_char(static_cast< uint8_t>(c));
			prev = c;
		}
		#endif
		break;
		
		case 5: // === SPI FLASH DYNAMIC PACKER ===
		#if USE_QSPI_FLASH
		{
		   flash_dyn_packer_putc(static_cast<uint8_t>(c));
		}
		
		#endif
		break;
		// ======================================

		default:
		break;
	}
}
