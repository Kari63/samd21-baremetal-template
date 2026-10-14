#include "formatter.h"
#include "backend.h"
#include <cstdarg>
#include <cstdint>

enum class FmtState : uint8_t {
	Text,
	Percent
};

#define MAX_ARGS 16

// --- Low-level output -------------------------------------------------------

static void out_char(uint8_t backend, char c)
{
	fb_putc(backend, c);
}

static void out_uint(uint8_t backend, uint32_t v, uint8_t width)
{
	char buf[10];
	int i = 0;

	do {
		buf[i++] = char('0' + (v % 10));
		v /= 10;
	} while (v > 0 && i < 10);

	while (i < width)
	buf[i++] = '0';

	while (i--)
	out_char(backend, buf[i]);
}

static void out_int(uint8_t backend, int32_t v, uint8_t width)
{
	if (v < 0) {
		out_char(backend, '-');
		out_uint(backend, uint32_t(-v), width);
		} else {
		out_uint(backend, uint32_t(v), width);
	}
}

static void out_hex(uint8_t backend, uint32_t v, bool upper)
{
	char buf[8];
	int i = 0;

	if (v == 0) {
		out_char(backend, '0');
		return;
	}

	while (v > 0 && i < 8) {
		uint8_t nib = uint8_t(v & 0xF);
		buf[i++] = (nib < 10)
		? char('0' + nib)
		: char((upper ? 'A' : 'a') + (nib - 10));
		v >>= 4;
	}

	while (i--)
	out_char(backend, buf[i]);
}

static void out_bin(uint8_t backend, uint32_t v)
{
	char buf[32];
	int i = 0;

	if (v == 0) {
		out_char(backend, '0');
		return;
	}

	while (v > 0 && i < 32) {
		buf[i++] = char('0' + (v & 1));
		v >>= 1;
	}

	while (i--)
	out_char(backend, buf[i]);
}

static void out_float(uint8_t backend, float v, uint8_t decimals)
{
	// If no number of decimals (width) is given (e.g. %f), the default is 2 decimals
	if (decimals == 0) {
		decimals = 2;
	}

	// Handle the minus sign
	if (v < 0.0f) {
		out_char(backend, '-');
		v = -v;
	}

	// Multiplier for the fractional part (e.g. 2 decimals -> 100)
	uint32_t multiplier = 1;
	for (uint8_t i = 0; i < decimals; i++) {
		multiplier *= 10;
	}

	// Round (+0.5) before converting to an integer
	v += 0.5f / float(multiplier);

	uint32_t integrity = uint32_t(v);
	uint32_t fraction = uint32_t((v - float(integrity)) * float(multiplier));

	// Print the integer part without zero padding
	out_uint(backend, integrity, 0);

	// Print the decimal point
	out_char(backend, '.');

	// Print the fractional part zero padded (width = number of decimals)
	out_uint(backend, fraction, decimals);
}


// --- Main formatter ---------------------------------------------------------

void print(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	uint32_t args_u32[MAX_ARGS];
	int32_t  args_i32[MAX_ARGS];
	const char *args_str[MAX_ARGS];
	char args_char[MAX_ARGS];
	uint32_t args_bin[MAX_ARGS];
	float args_flt[MAX_ARGS];

	int idx_u32 = 0;
	int idx_i32 = 0;
	int idx_str = 0;
	int idx_char = 0;
	int idx_bin = 0;
	int idx_flt = 0;

	// --- First pass: read arguments safely ----------------------------------
	const char *scan = fmt;

	while (*scan) {
		if (*scan == '%') {
			++scan;

			// *** SKIP WIDTH DIGITS ***
			while (*scan >= '0' && *scan <= '9') {
				++scan;
			}

			if (idx_u32 >= MAX_ARGS || idx_i32 >= MAX_ARGS ||
			idx_str >= MAX_ARGS || idx_char >= MAX_ARGS ||
			idx_bin >= MAX_ARGS || idx_flt >= MAX_ARGS)
			{
				va_end(ap);
				return;
			}

			switch (*scan) {
				case 'u':
				case 'x':
				case 'X':
				args_u32[idx_u32++] = va_arg(ap, uint32_t);
				break;

				case 'd':
				args_i32[idx_i32++] = va_arg(ap, int32_t);
				break;

				case 's':
				args_str[idx_str++] = va_arg(ap, const char *);
				break;

				case 'c':
				args_char[idx_char++] = char(va_arg(ap, int));
				break;

				case 'b':
				args_bin[idx_bin++] = va_arg(ap, uint32_t);
				break;

				case 'f':
				// Variadic '...' arguments: float is always promoted to double.
				// Read as double and store in the float array.
				args_flt[idx_flt++] = float(va_arg(ap, double));
				break;

				default:
				break;
			}
		}
		++scan;
	}

	// Backend is always the last variadic argument
	uint8_t backend = uint8_t(va_arg(ap, int));
	va_end(ap);

	// Reset indices for second pass
	idx_u32 = idx_i32 = idx_str = idx_char = idx_bin = idx_flt = 0;

	FmtState st = FmtState::Text;

	// --- Second pass: actual formatting -------------------------------------
	while (*fmt) {

		switch (st) {

			case FmtState::Text:
			if (*fmt == '%') {
				st = FmtState::Percent;
				} else {
				out_char(backend, *fmt);
			}
			++fmt;
			break;

			case FmtState::Percent: {

				uint8_t width = 0;

				// Parse width digits (optional)
				while (*fmt >= '0' && *fmt <= '9') {
					width = width * 10 + (*fmt - '0');
					++fmt;
				}

				switch (*fmt) {

					case 'u':
					out_uint(backend, args_u32[idx_u32++], width);
					break;

					case 'd':
					out_int(backend, args_i32[idx_i32++], width);
					break;

					case 'x':
					out_hex(backend, args_u32[idx_u32++], false);
					break;

					case 'X':
					out_hex(backend, args_u32[idx_u32++], true);
					break;

					case 'b':
					out_bin(backend, args_bin[idx_bin++]);
					break;

					case 'c':
					out_char(backend, args_char[idx_char++]);
					break;

					case 'f':
					out_float(backend, args_flt[idx_flt++], width);
					break;

					case 's': {
						const char *s = args_str[idx_str++];
						if (s) {
							while (*s)
							out_char(backend, *s++);
						}
						break;
					}

					default:
					out_char(backend, '%');
					out_char(backend, *fmt);
					break;
				}

				++fmt;
				st = FmtState::Text;
				break;
			}
		}
	}
}
