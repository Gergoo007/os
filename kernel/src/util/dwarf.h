#pragma once

#include <util/types.h>
#include <util/attrs.h>

typedef struct _attr_packed {
	u32 length;
	u16 version;
	u64 header_length;
	u8 min_instruction_length;
	u8 default_is_stmt;
	i8 line_base;
	u8 line_range;
	u8 opcode_base;
	u8 std_opcode_lengths[12];
} dbg_line_hdr;
