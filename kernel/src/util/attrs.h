#pragma once

// To make porting to other compilers easier
#define _attr_packed __attribute__((packed))
#define _attr_noret __attribute__((noreturn))
#define _attr_int __attribute__((interrupt))
#define _attr_unused __attribute__((unused))
#define _attr_pagealigned __attribute__((aligned(0x1000)))
#define _attr_align_stack __attribute__((force_align_arg_pointer))

#define _Alignas alignas
