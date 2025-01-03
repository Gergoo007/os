ENTRY("mod_main")

SECTIONS {
	. = SIZEOF_HEADERS;

	IMAGE_START = .;
	.text : {
		*(.text)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}
	IMAGE_END = .;

	/DISCARD/ : {
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)

		*(.dyn*)
		*(.interp)
		*(.*hash*)
	}
}
