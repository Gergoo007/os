#include <storage/storage.h>

#include <mm/vmm.h>
#include <util/string.h>
#include <util/mem.h>
#include <sysinfo.h>

stg_dev storage_parse(const char* s) {
	stg_dev ret;
	u8 fivechar = 0;
	if (!strncmp(s, storage_types[0], 3)) {
		ret.type = STG_MSD;
	} else if (!strncmp(s, storage_types[2], 5)) {
		fivechar = 1;
		ret.type = STG_ATAPI;
	} else if (!strncmp(s, storage_types[1], 3)) {
		ret.type = STG_ATA;
	} else {
		ret.type = -1;
	}

	ret.num = str_to_uint(s + (fivechar ? 5 : 3));

	return ret;
}

char* storage_id(stg_dev* drive) {
	char* name = vmm_alloc();
	u8 len = drive->type == STG_ATAPI ? 5 : 3;
	memcpy(storage_types[drive->type], name, len);
	uintn_to_str(drive->num, &name[len], 2);

	return name;
}

void storage_register(
	u8 _type, u64 size, char* fw,
	char* model, char* serial, void* port
) {
	drives[num_drives].type = _type;
	drives[num_drives].size = size;
	u8 next_free = 0;
	for (; next_free < 32; next_free++)
		if (!(storage_ids[_type] & (1 << next_free))) break;

	storage_ids[_type] |= (1 << next_free);

	if (serial) {
		memcpy(serial, drives[num_drives].serial, 20);
		drives[num_drives].serial[19] = 0;
	} else
		memcpy("(unknown)", drives[num_drives].serial, 20);

	if (fw) {
		memcpy(fw, drives[num_drives].firmware, 8);
		drives[num_drives].firmware[7] = 0;
	} else
		memcpy("(unknown)", drives[num_drives].firmware, 8);

	if (model) {
		memcpy(model, drives[num_drives].model, 40);
		drives[num_drives].model[39] = 0;
	} else
		memcpy("(unknown)", drives[num_drives].model, 40);

	drives[num_drives].num = next_free;
	drives[num_drives].port = port;
	num_drives++;
}
