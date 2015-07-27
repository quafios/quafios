ENTRY(main)

SECTIONS {

	. = 0x0100000;

	.mboot  : {*(.mboot )}
	.text   : {*(.text  )}
	.rodata : {*(.rodata)}
	.data   : {*(.data  )}
	.bss    : {*(.bss   )}

}
