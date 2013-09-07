
ENTRY(startup)

MEMORY {
	FLASH  (RX) : ORIGIN = 0x08000000       , LENGTH = 15k
	CONFIG (RW) : ORIGIN = 0x08000000 + 15k , LENGTH = 1k
	RAM    (RW) : ORIGIN = 0x20000000       , LENGTH = 4k
}

SECTIONS {
	.text : {
		KEEP(*(.isr_vector))

		. = ALIGN(4);

		*(.text .text.*)
		*(.rodata .rodata.*)

		. = ALIGN(4);

		_data_flash_start = .;
	} > FLASH

	.bss : {
		. = ALIGN(4);
		_bss_start = .;

		*(.bss)
		*(COMMON)

		. = ALIGN(4);
		_bss_end = .;
	} > RAM

	.data : AT(_data_flash_start) {
		. = ALIGN(4);
		_data_start = .;

		*(.data)

		. = ALIGN(4);
		_data_end = .;
	} > RAM

	_data_length = _data_end - _data_start;
	_data_flash_end = _data_flash_start + _data_length;

	.config : {
		*(.config)
	} > CONFIG

	_stack_end = ORIGIN(RAM) + LENGTH(RAM);
}
