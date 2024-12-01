#pragma once

#include <util/attrs.h>
#include <util/types.h>

typedef struct _attr_packed gpt_hdr {
	union _attr_packed {
		struct _attr_packed {
			u64 sign;
			u32 rev;
			u32 hdr_size;
			u32 crc32;
			u32 : 32;
			u64 hdr_lba; // Erre a headerre mutat?
			u64 alt_hdr_lba;
			u64 first_usable;
			u64 last_usable;
			u8 disk_guid[16];
			u64 entries_lba;
			u32 num_entries;
			u32 entry_size;
			u32 table_crc32;
		};
		u8 lba1[512];
	};

	union _attr_packed {
		struct _attr_packed {
			u8 part_type[16];
			u8 part_guid[16];
			u64 start_lba;
			u64 end_lba;
			u64 attrs;
			u8 name[72];
		} entries[0];
		u8 lba2[512];
	};
} gpt_hdr;
