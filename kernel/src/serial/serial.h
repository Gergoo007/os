#pragma once

#include <util/types.h>

#include <util/printf.h>
#define sprintk(fmt, ...) printf(sputc, sputs, fmt, ##__VA_ARGS__)

void outb(u8 b, u16 port);
void outw(u16 b, u16 port);
void outl(u32 b, u16 port);

u8 inb(u16 port);
u16 inw(u16 port);
u32 inl(u16 port);

u8 sputc(const char* c);
void sputs(char* str);
