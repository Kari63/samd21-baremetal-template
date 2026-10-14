

#include "project_config.h"

#include "backend.h"
#include "tft.h"
#include "lcdpcf8574.h"
#include "usart2.h"
#include "usart4.h"
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
			TFT_drawChar(cx, cy, ch);
			SPI_endWrite();

			// --- GFXfont compatible advance (WITHOUT scaling) ---
			if (gfxFont)
			{
				if (ch >= gfxFont->first && ch <= gfxFont->last)
				{
					const GFXglyph *glyph = &gfxFont->glyph[ch - gfxFont->first];
					TFT_setCursor(cx + glyph->xAdvance, cy);
				}
				else
				{
					// fallback monospace
					TFT_setCursor(cx + 6, cy);
				}
			}
			else
			{
				// default font (5x7)
				TFT_setCursor(cx + 6, cy);
			}
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
			if (c == '\n')
			{
				usart2_write_string("\r\n");
			}
			else
			{
				usart2_write_char(static_cast< uint8_t>(c));
			}
		}
		#endif
		break;

		case 4: // === USART4 ===
		#if USE_UART4
		{
			if (c == '\n')
			{
				usart4_write_string("\r\n");
			}
			else
			{
				usart4_write_char(static_cast< uint8_t>(c));
			}
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
