/*
 * system_commands.cpp
 *
 * MODE2-based RTC command console
 * SETTIME hh:mm:ss
 * SETDATE dd.mm.yy
 */

#include "samd21.h"
#include "system_commands.h"
#include "formatter.h"
#include "usart4.h"

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

	// --- Set the next alarm exactly +1 second from the new time ---
	uint32_t next_alarm = (s + 1) % 60;
	RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND = next_alarm;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

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

	// Save the current seconds BEFORE clearing, so we know where we are
	uint32_t current_current_seconds = clk & 0x3F;

	// clear DAY, MONTH, YEAR
	clk &= ~((0x1Fu << 17) | (0x0Fu << 22) | (0x3Fu << 26));

	// set the new values
	clk |= ((uint32_t)d << 17);
	clk |= ((uint32_t)m << 22);
	clk |= ((uint32_t)y << 26);

	RTC->MODE2.CLOCK.reg = clk;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	// --- The seconds were not touched, so the alarm must be the current second + 1 ---
	uint32_t next_alarm = (current_current_seconds + 1) % 60;
	RTC->MODE2.Mode2Alarm[0].ALARM.bit.SECOND = next_alarm;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

	RTC->MODE2.CTRL.reg |= RTC_MODE2_CTRL_ENABLE;
	while (RTC->MODE2.STATUS.bit.SYNCBUSY) {}

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
static void parse_time(const char *t)
{
    uint8_t hh = (t[0] - '0') * 10 + (t[1] - '0');
    uint8_t mm = (t[3] - '0') * 10 + (t[4] - '0');
    uint8_t ss = (t[6] - '0') * 10 + (t[7] - '0');

    rtc_set_time(hh, mm, ss);
}

// ------------------------------------------------------------
// Date parsing: "dd.mm.yy"
// ------------------------------------------------------------
static void parse_date(const char *t)
{
    uint8_t dd = (t[0] - '0') * 10 + (t[1] - '0');
    uint8_t mm = (t[3] - '0') * 10 + (t[4] - '0');
    uint8_t yy = (t[6] - '0') * 10 + (t[7] - '0');

    rtc_set_date(dd, mm, yy);
}

// ------------------------------------------------------------
// Command processing
// ------------------------------------------------------------
void process_command(const char *cmd)
{
    if (starts_with(cmd, "SETTIME ")) {
        parse_time(cmd + 8);
        return;
    }

    if (starts_with(cmd, "SETDATE ")) {
        parse_date(cmd + 8);
        return;
    }

    print("Unknown command: %s\r\n", cmd, 3);
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
// Poll USART4 and feed console
// ------------------------------------------------------------
void command_softirq(void)
{
	uint8_t c;

	while (usart4_read_char(&c)) {
		serial_feed_char(c);
		print("%c", c, 4);
	}
}

// ------------------------------------------------------------
// Console banner
// ------------------------------------------------------------
void command_console_banner(void)
{
    print("\r\n", 4);
    print("MonstR Command Console\r\n", 4);
    print("Commands:\r\n", 4);
    print("  SETTIME hh:mm:ss\r\n", 4);
    print("  SETDATE dd.mm.yy\r\n", 4);
    print("\r\n", 4);


}