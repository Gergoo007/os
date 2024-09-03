OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS {
	. = 0x401000;

	.text : ALIGN(0x1000) {
		*(.*text*)
	}

	.data : ALIGN(0x1000) {
		*(.data)
		*(.rodata)
	}

	.bss : ALIGN(0x1000) {
		*(.bss)
	}

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
	}
}
