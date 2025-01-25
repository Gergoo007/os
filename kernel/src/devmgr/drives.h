#pragma once

#include <storage/ahci/ahci.h>

#define GUID_ESP (u32[]){ 0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B }
#define GUID_BDP (u32[]){ 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7 }
#define GUID_OWN (u32[]){ 0xCAFEBABE, 0x0556, 0x0762, 'n',  'e',  'p',  't',  'u',  'n',  'o',  's'  }

#define CHECK(p0, p1, p2, r, g, en) \
	if ((p0) == (g)[0] && \
	(p1) == (g)[1] && \
	(p2) == (g)[2]) { \
		failed = 0; \
		for (u8 i = 0; i < 8; i++) { \
			if ((r)[i] != (g)[3+i]) { failed = 1; break; } \
		} \
		if (!failed) return en; \
	}

#define CHECK_GUID(p0, p1, p2, r) \
	CHECK(p0, p1, p2, r, GUID_ESP, PART_ESP); \
	CHECK(p0, p1, p2, r, GUID_BDP, PART_BDP); \
	CHECK(p0, p1, p2, r, GUID_OWN, PART_OWN); \

// TODO: Nem biztos hogy nincsenek kihagyások a táblába (ez kilép az elsőnél)
#define FOREACH_PART(gpt, iter) \
	for (u32 iter = 0; iter < h->num_entries && *(u64*)h->entries[i].part_type; iter++)

enum PART_TYPES {
	PART_UNKNOWN = 0,
	PART_ESP,
	PART_BDP,
	PART_OWN,
};

typedef struct partition {
	u8 type;
	u8 fstype;
	char name[72];
	u64 startlba;
	u64 endlba;
	dev_drive* drive;
	union {
		void* cache;
		struct fat32_cache* f32c;
	};
} partition;

typedef struct _attr_packed part_table {
	u32 num_parts;
	partition parts[0];
} part_table;

enum PART_TYPES guid_to_enum(u8* guid);
void drive_init(dev_drive* d);
void drive_read(dev_drive* d, u64 start, u64 sectors, void* into);
void drive_write();
