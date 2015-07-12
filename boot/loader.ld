ENTRY(main)

SECTIONS {

	. = 0x0008000;

	.text   : {*(.text.main) *(.text)}
	.rodata : {*(.rodata            )}
	.data   : {*(.data              )}
	.bss    : {*(.bss COMMON        )}

}
