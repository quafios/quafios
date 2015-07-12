ENTRY(main)

SECTIONS {

	. = 0x00100000;

	kernel_physical_start = .;

	.text   : {*(.text.main .text)}
	.rodata : {*(.rodata         )}
	.data   : {*(.data           )}
	.bss    : {*(.bss COMMON     )}

	kernel_physical_end = . ;

}
