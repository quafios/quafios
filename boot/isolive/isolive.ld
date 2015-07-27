ENTRY(main)

SECTIONS {

	. = 0x0001000;

	.text   : {*(.text.eltorito) *(.text)}
	.rodata : {*(.rodata                )}
	.data   : {*(.data                  )}
	.bss    : {*(.bss COMMON            )}

}
