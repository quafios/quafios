include $(top_srcdir)/generic.am

OBJECT_NAME   = kernel
OBJECT_PREFIX = $(BOOT_DIR)
LDSCRIPT      = ld/arch32.ld
LFLAGS       += -Wl,--oformat=binary -Wl,-T$(LDSCRIPT) -Wl,--build-id=none
LIBS          = -lgcc
OBJ           = $(shell echo $(CFILES) | sed 's/\.c/\.o/g')
DEPS          = $(ALLHFILES) Makefile

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

fonts/font8x16.o: fonts/font8x16.fon
	objcopy -I binary -O elf32-i386 -B i386  \
	--redefine-sym _binary_fonts_font8x16_fon_start=_font8x16_start \
	$< $@

fonts/font10x24.o: fonts/font10x24.fon
	objcopy -I binary -O elf32-i386 -B i386  \
	--redefine-sym _binary_fonts_font10x24_fon_start=_font10x24_start \
	$< $@

fonts: fonts/font8x16.o fonts/font10x24.o

$(OBJECT_NAME).bin: fonts $(OBJ) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o $@ $(OBJ) $(LIBS) fonts/font8x16.o fonts/font10x24.o

install-exec-local: install-includes install-libraries
	$(INSTALL) -D $(OBJECT_NAME).bin $(OBJECT_PREFIX)/$(OBJECT_NAME).bin

uninstall-local: uninstall-includes uninstall-libraries
	rm -f $(OBJECT_PREFIX)/$(OBJECT_NAME)
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: $(OBJECT_NAME).bin \
	   $(GEN_INC_DIR)/$(OBJECT_NAME) \
	   $(GEN_LIB_DIR)/$(OBJECT_NAME)

clean-local: clean-includes clean-libraries
	rm -f $(OBJ) fonts/*.o $(OBJECT_NAME).bin
