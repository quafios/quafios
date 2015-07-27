include $(top_srcdir)/generic.am

OBJECT_NAME    = loader
OBJECT_PREFIX  = $(BOOT_DIR)
LOADER_LD      = $(CSD)/loader/loader.ld
ISOLIVE_LD     = $(CSD)/isolive/isolive.ld
MULTIBOOT_LD   = $(CSD)/multiboot/multiboot.ld
LOADER_LFLAGS  = $(LFLAGS) -Wl,--oformat=binary -Wl,-T$(LOADER_LD)
ISOLIVE_LFLAGS = $(LFLAGS) -Wl,--oformat=binary -Wl,-T$(ISOLIVE_LD)
LOADER_SRC     = $(LOADER_OBJ) $(COMMON_OBJ)
ISOLIVE_SRC    = isolive/eltorito.o $(ISOLIVE_OBJ) $(COMMON_OBJ)
LIBS           =
DEPS           = $(ALLHFILES) Makefile \
		 $(KERNEL_INCLUDE)

LOADER_OBJ = \
$(shell cd $(CSD) && find ./loader -name "*.c" 2>/dev/null | sed 's/\.c/\.o/g')

COMMON_OBJ = \
$(shell cd $(CSD) && find ./common -name "*.c" 2>/dev/null | sed 's/\.c/\.o/g')

ISOLIVE_OBJ = \
$(shell cd $(CSD) && find ./isolive -name "*.c" 2>/dev/null | sed 's/\.c/\.o/g')

%.o: %.c $(DEPS)
	mkdir -p isolive
	mkdir -p loader
	mkdir -p common
	$(CC) $(CFLAGS) -c -o $@ $<

diskboot/mbr.o: diskboot/mbr.s Makefile
	mkdir -p diskboot
	$(AS) --32 $< -o $@

mbr.bin: diskboot/mbr.o
	$(LD) -m elf_i386 --oformat=binary -Ttext 0x0600 -e start $< -o $@

diskboot/vbr.o: diskboot/vbr.s Makefile
	mkdir -p diskboot
	$(AS) --32 $< -o $@

vbr.bin: diskboot/vbr.o
	$(LD) -m elf_i386 --oformat=binary -Ttext 0x7C00 -e start $< -o $@

isolive/eltorito.o: isolive/eltorito.s Makefile
	mkdir -p isolive
	$(AS) --32 $< -o $@

isolive.bin: $(ISOLIVE_SRC) $(ISOLIVE_LD)
	$(CC) $(ISOLIVE_LFLAGS) -o $@ $(ISOLIVE_SRC) $(LIBS)

loader.bin: $(LOADER_SRC) $(LOADER_LD)
	$(CC) $(LOADER_LFLAGS) -o $@ $(LOADER_SRC) $(LIBS)

multiboot/multiboot.o: multiboot/multiboot.s Makefile
	mkdir -p multiboot
	$(AS) --32 $< -o $@

multiboot.bin: multiboot/multiboot.o $(MULTIBOOT_LD)
	$(LD) -m elf_i386 -T$(MULTIBOOT_LD) $< -o $@

install-exec-local:
	$(INSTALL) -D loader.bin $(OBJECT_PREFIX)/loader.bin

uninstall-local:
	rm -f $(OBJECT_PREFIX)/loader.bin
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: mbr.bin vbr.bin isolive.bin loader.bin multiboot.bin

clean-local:
	rm -f diskboot/mbr.o mbr.bin
	rm -f diskboot/vbr.o vbr.bin
	rm -f isolive/eltorito.o isolive.bin
	rm -f loader.bin
	rm -f multiboot/multiboot.o multiboot.bin
	rm -f $(LOADER_OBJ)
	rm -f $(ISOLIVE_OBJ)
	rm -f $(COMMON_OBJ)
