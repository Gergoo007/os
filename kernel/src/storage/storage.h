#pragma once

#include <util/attrs.h>
#include <util/types.h>

enum {
	STG_MSD = 0x00,
	STG_ATA = 0x01,
	STG_ATAPI = 0x02,
};

// Bitmapek
_attr_unused static u32 storage_ids[] = {
	0,
	0,
	0,
};

_attr_unused static char* storage_types[] = {
	"msd",
	"ata",
	"atapi",
};

typedef struct stg_dev {
	void* port;
	u64 size; // Byte, 512 többszöröse

	u8 num;
	u8 type;

	char serial[20];
	char firmware[8];
	char model[40];
} stg_dev;

stg_dev storage_parse(const char* s);
char* storage_id(stg_dev* drive);
void storage_register(
	u8 _type, u64 size, char* fw, char* model, char* serial, void* port);
