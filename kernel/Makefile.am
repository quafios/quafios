include $(top_srcdir)/generic.am

OBJECT_NAME   = kernel
OBJECT_PREFIX = $(BOOT_DIR)
LDSCRIPT      = $(CSD)/ld/arch32.ld
LFLAGS       += -Wl,--oformat=binary -Wl,-T$(LDSCRIPT) -Wl,--build-id=none
LIBS          = -lgcc
DEPS          = $(ALLHFILES) Makefile

%.o:%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_NAME).bin: $(OBJ) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o $@ $(OBJ) $(LIBS)

install-exec-local: install-includes install-libraries
	$(INSTALL) -D $(OBJECT_NAME).bin $(OBJECT_PREFIX)/$(OBJECT_NAME).bin

uninstall-local: uninstall-includes uninstall-libraries
	rm -f $(OBJECT_PREFIX)/$(OBJECT_NAME)
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: make-dirs $(OBJECT_NAME).bin

clean-local:
	rm -f $(OBJ) fonts/*.o $(OBJECT_NAME).bin
