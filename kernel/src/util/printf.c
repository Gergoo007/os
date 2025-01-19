#include <util/printf.h>
#include <util/types.h>
#include <util/string.h>

void vprintf(u8 (*putc)(const char* c),
	void (*puts)(char* str),
	const char* fmt,
	va_list l
) {
	while (*fmt) {
		if (*fmt == '%') {
			fmt++;

			switch (*fmt) {
				case 'c':
					char a = (char) va_arg(l, u32);
					putc(&a);
					break;

				case 's': {
					char* s = va_arg(l, char*);
					if (s)
						puts(s);
					else
						puts("(null)");
					break;
				}

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
							putc(str++);
						}
					} else {
						num = num * 10 + (*fmt - '0');
						char* str = va_arg(l, char*);
						while (num--) {
							putc(str++);
						}
						fmt++;
					}

					break;
				}

				case 'l': {
					fmt++;
					if (*fmt == 'c') {
						wchar arg = va_arg(l, u32);
						if (arg > 128) arg = 0;
						putc((char*)&arg);
					} else if (*fmt == 's') {
						wchar* arg = va_arg(l, wchar*);
						while (*arg) {
							wchar c = *arg;
							if (c > 128) c = 0;
							putc((char*)&c);
							arg++;
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
										putc(str++);
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
			fmt++;
		} else {
			fmt += putc(fmt);
		}
	}
}

void printf(u8 (*putc)(const char* c),
	void (*puts)(char* str),
	const char* fmt, ...
) {
	va_list l;
	va_start(l, fmt);
	vprintf(putc, puts, fmt, l);
	va_end(l);
}
