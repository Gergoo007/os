#pragma once

#include <util/attrs.h>
#include <util/types.h>

typedef struct _attr_packed ustar_hdr {
	char name[100];
	u64 mode;
	u64 uid;
	u64 gid;
	u8 size[12];
	u8 modtime[12];
	u64 checksum;
	u8 type;
	char linkedfilename[100];
	char ustar[6]; // "ustar"
	u16 ustar_version;
	u64 uname;
	u64 gname;
	u64 devmajor;
	u64 devminor;
	char filename_prefix[155];
} ustar_hdr;

typedef struct tar_file {
	void* content;
	u32 size;
} tar_file;

tar_file tar_read_file(ustar_hdr* archive, char* name);
