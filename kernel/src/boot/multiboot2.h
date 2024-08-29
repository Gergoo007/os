#pragma once

#include <util/types.h>
#include <util/attrs.h>

enum {
	MB_TAG_END				= 0,
	MB_TAG_CMDLINE			= 1,
	MB_TAG_BOOTLOADER		= 2,
	MB_TAG_MODULE			= 3,
	MB_TAG_BASIC_MEMINFO	= 4,
	MB_TAG_BOOTDEV			= 5,
	MB_TAG_MMAP				= 6,
	MB_TAG_VBE				= 7,
	MB_TAG_FRAMEBUFFER		= 8,
	MB_TAG_ELF_SECTIONS		= 9,
	MB_TAG_APM	 			= 0,
	MB_TAG_EFI32			= 1,
	MB_TAG_EFI64			= 2,
	MB_TAG_SMBIOS			= 3,
	MB_TAG_ACPI_OLD			= 4,
	MB_TAG_ACPI_NEW			= 5,
	MB_TAG_NETWORK 			= 6,
	MB_TAG_EFI_MMAP			= 7,
	MB_TAG_EFI_BS			= 8,
	MB_TAG_EFI32_IH			= 9,
	MB_TAG_EFI64_IH			= 0,
	MB_TAG_LOAD_BASE_ADDR	= 1,
};

typedef struct _attr_packed {
	u32 type;
	u32 size;
} mb_tag;

#define FB_VADDR 0xffffffffc2000000

void multiboot2_parse(mb_tag* addr);
