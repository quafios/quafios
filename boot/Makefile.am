include $(top_srcdir)/generic.am

OBJECT_NAME   = loader
OBJECT_PREFIX = $(BOOT_DIR)
LDSCRIPT      = loader.ld
LFLAGS       += -Wl,--oformat=binary -Wl,-T$(LDSCRIPT)
LIBS          =
OBJ           = $(shell echo $(CFILES) | sed 's/\.c/\.o/g')
DEPS          = $(ALLHFILES) Makefile \
		$(GEN_INC_DIR)/kernel

cdboot.o: cdboot.s Makefile
	$(AS) --32 $< -o $@

cdboot: cdboot.o
	$(LD) -m elf_i386 --oformat=binary -Ttext 0x7C00 -e _start $< -o $@

realtest.o: realtest.s Makefile
	$(AS) --32 $< -o $@

realtest: realtest.o realtest.ld
	$(LD) -m elf_i386 -Trealtest.ld $< -o $@

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

loader.bin: $(OBJ) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o $@ $(OBJ) $(LIBS)

install-exec-local:
	$(INSTALL) -D cdboot $(OBJECT_PREFIX)/cdboot
	$(INSTALL) -D loader.bin $(OBJECT_PREFIX)/loader.bin

uninstall-local:
	rm -f $(OBJECT_PREFIX)/cdboot
	rm -f $(OBJECT_PREFIX)/loader.bin
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: cdboot realtest loader.bin

clean-local:
	rm -f cdboot cdboot.o realtest realtest.o $(OBJ) loader.bin
