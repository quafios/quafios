include $(top_srcdir)/generic.am

OBJECT_NAME   = kernel
OBJECT_PREFIX = $(BOOT_DIR)
LDSCRIPT      = $(CSD)/ld/arch32.ld
LFLAGS       += -Wl,-T$(LDSCRIPT) -Wl,--build-id=none
BLFLAGS       = $(LFLAGS) -Wl,--oformat=binary
LIBS          = -lgcc
DEPS          = $(ALLHFILES) Makefile

%.o:%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_NAME).bin: $(OBJ) $(LDSCRIPT)
	$(CC) $(BLFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJECT_NAME).elf: $(OBJ) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o $(OBJECT_NAME).elf $(OBJ) $(LIBS)

$(OBJECT_NAME).dump: $(OBJECT_NAME).elf
	objdump -D $< > $@

install-exec-local: install-includes install-libraries
	$(INSTALL) -D $(OBJECT_NAME).bin $(OBJECT_PREFIX)/$(OBJECT_NAME).bin

uninstall-local: uninstall-includes uninstall-libraries
	rm -f $(OBJECT_PREFIX)/$(OBJECT_NAME)
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: make-dirs $(OBJECT_NAME).bin $(OBJECT_NAME).dump

clean-local:
	rm -f $(OBJ) fonts/*.o $(OBJECT_NAME).*
