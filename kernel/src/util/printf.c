#include <util/printf.h>
#include <util/types.h>
#include <util/string.h>

void vprintf(void (*putc)(char c),
	void (*puts)(char* str),
	const char* fmt,
	va_list l
) {
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;

			switch (*fmt) {
				case 'c':
					putc((char) va_arg(l, u32));
					break;

				case 's':
					puts(va_arg(l, char*));
					break;

				case 'x': {
					char num[17];
					hex_to_str(va_arg(l, u64), num);
					puts(num);
					break;
				}

				case 'p': {
					char num[17];
					hexn_to_str(va_arg(l, u64), num, 16);
					puts(num);
					break;
				}

				case 'd': {
					char num[64];
					int_to_str(va_arg(l, i32), num);
					puts(num);
					break;
				}

				case '.': {
					fmt++;
					u8 num = *fmt - '0';
					fmt++;

					if (*fmt == 's') {
						char* str = va_arg(l, char*);
						while (num--) {
							putc(*(str++));
						}
					}

					break;
				}

				default: {
					if (*fmt >= '0' && *fmt <= '9') {
						fmt++;
						if (*fmt >= '0' && *fmt <= '9') {
							fmt++;

							u32 num = (*(fmt-1) - '0') + (*(fmt-2) -'0') * 10;
							switch (*fmt) {
								case 's': {
									char* str = va_arg(l, char*);
									while (num--)
										putc(*(str++));
									break;
								}

								case 'x': {
									char num2[17];
									hexn_to_str(va_arg(l, u64), num2, num);
									puts(num2);
									break;
								}
							}
						}
					}
				}
			}
		} else {
			putc(*fmt);
		}
		fmt++;
	}
}

void printf(void (*putc)(char c),
	void (*puts)(char* str),
	const char* fmt, ...
) {
	va_list l;
	va_start(l, fmt);
	vprintf(putc, puts, fmt, l);
	va_end(l);
}
