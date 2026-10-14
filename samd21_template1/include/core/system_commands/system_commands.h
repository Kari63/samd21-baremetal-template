/*
 * system_commands.h
 *
 * Created: 29.8.2026 13.54.50
 *  Author: Kari63
 */ 
#pragma once
#include <stdint.h>

void rtc_set_time(uint8_t h, uint8_t m, uint8_t s);
void serial_feed_char(char c);
void process_command(const char *cmd);
void command_softirq(void);
void command_console_banner(void);

