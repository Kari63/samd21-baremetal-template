/*
 * system_commands.cpp
 *
 * MODE2-based RTC command console
 * SETTIME hh:mm:ss
 * SETDATE dd.mm.yy
 * I2CSCAN            addresses 0x08..0x77 that ACK on SERCOM0 (USE_I2C)
 * BMP280             temperature and pressure (USE_BMP280)
 * DHT20              temperature and humidity (USE_DHT20)
 */

#include "samd21.h"
#include "project_config.h"
#include "system_commands.h"
#include "formatter.h"
#include "usart3.h"
#include "rtc_mode2_init.h"
#if USE_I2C
#include "i2c_0.h"
#endif
#if USE_BMP280
#include "bmp280.h"
#endif
#if USE_DHT20
#include "dht20.h"
#endif

// ------------------------------------------------------------
// MODE2: RTC time set (hh:mm:ss → CLOCK sec/min/hour)
// ------------------------------------------------------------
void rtc_set_time(uint8_t h, uint8_t m, uint8_t s)
{
	// stop the RTC
	RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	uint32_t clk = RTC->MODE2.CLOCK.reg;

	// clear SECOND, MINUTE, HOUR
	clk &= ~((0x3Fu << 0) | (0x3Fu << 6) | (0x1Fu << 12));

	// set the new values
	clk |= ((uint32_t)s << 0);
	clk |= ((uint32_t)m << 6);
	clk |= ((uint32_t)h << 12);

	RTC->MODE2.CLOCK.reg = clk;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}
	rtc_read_sync_continuous();             // the ENABLE write cleared RCONT

	print("Time set to %02u:%02u:%02u\r\n", h, m, s, 4);
}

// ------------------------------------------------------------
// MODE2: RTC date set (dd.mm.yy → CLOCK day/month/year)
// ------------------------------------------------------------
void rtc_set_date(uint8_t d, uint8_t m, uint8_t y)
{
	// stop the RTC
	RTC->MODE2.CTRL.reg &= ~RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	uint32_t clk = RTC->MODE2.CLOCK.reg;

	// clear DAY, MONTH, YEAR
	clk &= ~((0x1Fu << 17) | (0x0Fu << 22) | (0x3Fu << 26));

	// set the new values
	clk |= ((uint32_t)d << 17);
	clk |= ((uint32_t)m << 22);
	clk |= ((uint32_t)y << 26);

	RTC->MODE2.CLOCK.reg = clk;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}
	rtc_read_sync_continuous();             // the ENABLE write cleared RCONT

	print("Date set to %02u.%02u.%02u\r\n", d, m, y, 4);
}


// ------------------------------------------------------------
// Command buffer
// ------------------------------------------------------------
static char   cmd_buf[32];
static uint8_t cmd_pos = 0;

// ------------------------------------------------------------
// starts_with
// ------------------------------------------------------------
static bool starts_with(const char *s, const char *p)
{
    while (*p) {
        if (*s++ != *p++) return false;
    }
    return true;
}

// ------------------------------------------------------------
// Time parsing: "hh:mm:ss"
// ------------------------------------------------------------
static bool two_digits(const char *t, uint8_t *out)
{
    if (t[0] < '0' || t[0] > '9' || t[1] < '0' || t[1] > '9') {
        return false;
    }
    *out = (uint8_t)((t[0] - '0') * 10 + (t[1] - '0'));
    return true;
}

static void parse_time(const char *t)
{
    uint8_t hh, mm, ss;

    // Checked before the RTC: format, digits, line length and ranges
    if (!two_digits(&t[0], &hh) || t[2] != ':' ||
        !two_digits(&t[3], &mm) || t[5] != ':' ||
        !two_digits(&t[6], &ss) || t[8] != '\0' ||
        hh > 23 || mm > 59 || ss > 59) {
        print("Usage: SETTIME hh:mm:ss\r\n", 4);
        return;
    }
    rtc_set_time(hh, mm, ss);
}

// ------------------------------------------------------------
// Date parsing: "dd.mm.yy"
// ------------------------------------------------------------
static void parse_date(const char *t)
{
    uint8_t dd, mm, yy;

    // MODE2 YEAR field is 6 bits: yy 0..63
    if (!two_digits(&t[0], &dd) || t[2] != '.' ||
        !two_digits(&t[3], &mm) || t[5] != '.' ||
        !two_digits(&t[6], &yy) || t[8] != '\0' ||
        dd < 1 || dd > 31 || mm < 1 || mm > 12 || yy > 63) {
        print("Usage: SETDATE dd.mm.yy\r\n", 4);
        return;
    }
    rtc_set_date(dd, mm, yy);
}

#if USE_I2C

// ------------------------------------------------------------
// Known devices on the template's I2C bus (7-bit addresses)
// ------------------------------------------------------------
static const char *i2c_name(uint8_t addr)
{
	switch (addr) {
	case 0x18:
	case 0x19: return "LIS3DHTR accelerometer";
	case 0x20: case 0x21: case 0x22: case 0x23:
	case 0x24: case 0x25: case 0x26:
	case 0x27: return "PCF8574 (LCD backpack)";
	case 0x28: return "EDBG (Xplained Pro debugger)";
	case 0x38: return "DHT20";
	case 0x3C:
	case 0x3D: return "SSD1306 OLED";
	case 0x76:
	case 0x77: return "BMP280 / BME280";
	default:   return "";
	}
}

// ------------------------------------------------------------
// I2CSCAN: address-only write (n = 0) to 0x08..0x77, ACK = device.
// A running LCD DMA frame is waited for by i2c_write (i2c_dma_wait).
// ------------------------------------------------------------
static void i2c_scan(void)
{
	uint8_t found = 0;

	for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
		if (i2c_write(addr, nullptr, 0)) {
			// %X has no width in the formatter: leading zero by hand
			print("  0x%s%X  %s\r\n", (addr < 0x10) ? "0" : "", (uint32_t)addr, i2c_name(addr), 4);
			found++;
		}
	}
	print("%u device(s)\r\n", (uint32_t)found, 4);
}

#endif /* USE_I2C */

#if USE_BMP280 || USE_DHT20

// ------------------------------------------------------------
// "T=-3.25 C" from 0.01 degC (the formatter has no signed fixed point)
// ------------------------------------------------------------
static void print_temp_c100(int32_t c100)
{
	const bool     neg = c100 < 0;
	const uint32_t t   = (uint32_t)(neg ? -c100 : c100);
	print("T=%s%u.%02u C", neg ? "-" : "", t / 100u, t % 100u, 4);
}

#endif

#if USE_BMP280

// ------------------------------------------------------------
// BMP280: latest measurement, "T=-3.25 C  P=1013.25 hPa"
// ------------------------------------------------------------
static void bmp280_show(void)
{
	BMP280_Data_t d;
	if (!bmp280_read(&d)) {
		print("BMP280: no data (not found at init or I2C error)\r\n", 4);
		return;
	}
	print_temp_c100(d.temp_c100);
	print("  P=%u.%02u hPa\r\n", d.press_pa / 100u, d.press_pa % 100u, 4);
}

#endif /* USE_BMP280 */

#if USE_DHT20

// ------------------------------------------------------------
// DHT20: latest measurement (no bus traffic), "T=21.53 C  RH=45.67 %"
// ------------------------------------------------------------
static void dht20_show(void)
{
	DHT20_Data_t d;
	if (!dht20_read(&d)) {
		print("DHT20: no data yet (missing, not calibrated or errors)\r\n", 4);
		return;
	}
	print_temp_c100(d.temp_c100);
	print("  RH=%u.%02u %s\r\n", d.rh_c100 / 100u, d.rh_c100 % 100u, "%", 4);   /* no %% in the formatter */
}

#endif /* USE_DHT20 */

// ------------------------------------------------------------
// Command list (banner and HELP)
// ------------------------------------------------------------
static void command_help(void)
{
    print("Commands:\r\n", 4);
    print("  SETTIME hh:mm:ss\r\n", 4);
    print("  SETDATE dd.mm.yy\r\n", 4);
#if USE_I2C
    print("  I2CSCAN\r\n", 4);
#endif
#if USE_BMP280
    print("  BMP280\r\n", 4);
#endif
#if USE_DHT20
    print("  DHT20\r\n", 4);
#endif
    print("  HELP\r\n", 4);
}

// ------------------------------------------------------------
// Command processing
// ------------------------------------------------------------
void process_command(const char *cmd)
{
    if (starts_with(cmd, "HELP")) {
        command_help();
        return;
    }
#if USE_I2C
    if (starts_with(cmd, "I2CSCAN")) {
        i2c_scan();
        return;
    }
#endif
#if USE_BMP280
    if (starts_with(cmd, "BMP280")) {
        bmp280_show();
        return;
    }
#endif
#if USE_DHT20
    if (starts_with(cmd, "DHT20")) {
        dht20_show();
        return;
    }
#endif

    if (starts_with(cmd, "SETTIME ")) {
        parse_time(cmd + 8);
        return;
    }

    if (starts_with(cmd, "SETDATE ")) {
        parse_date(cmd + 8);
        return;
    }

    print("Unknown command: %s\r\n", cmd, 4);   // console = 4 (was 3 = USART2)
}

// ------------------------------------------------------------
// Feed chars into command buffer
// ------------------------------------------------------------
void serial_feed_char(char c)
{
    if (c == '\r' || c == '\n') {
        print("\r\n", 4);
        cmd_buf[cmd_pos] = 0;

        if (cmd_pos > 0) {
            process_command(cmd_buf);
        }

        cmd_pos = 0;
        return;
    }

    if (cmd_pos < sizeof(cmd_buf) - 1) {
        cmd_buf[cmd_pos++] = c;
    }
}

// ------------------------------------------------------------
// Poll USART3 (EDBG virtual COM) and feed console
// ------------------------------------------------------------
void command_softirq(void)
{
	uint8_t c;

	while (usart3_read_char(&c)) {
		serial_feed_char(c);
		if (c != '\r' && c != '\n') {
			print("%c", c, 4);   // echo; Enter is answered by serial_feed_char
		}
	}
}

// ------------------------------------------------------------
// Console banner
// ------------------------------------------------------------
void command_console_banner(void)
{
    print("\r\n", 4);
#if PRINT_BOOT_BANNER
    // Rebuild refreshes the build stamp; a plain Build may leave it stale
    print("%s %s (%s) %s %s\r\n", TEMPLATE_NAME, TEMPLATE_VERSION_STRING,
          TEMPLATE_VARIANT, __DATE__, __TIME__, 4);
#endif
    print("MonstR Command Console\r\n", 4);
    command_help();
    print("\r\n", 4);


}