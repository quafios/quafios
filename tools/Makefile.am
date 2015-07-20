include $(top_srcdir)/generic.am

MKDISKFS_OBJ  = mkdiskfs
DEPS          = $(CFILES) $(ALLHFILES) Makefile \
                $(KERNEL_INCLUDE)

$(MKDISKFS_OBJ): $(DEPS)
	$(CC) -pedantic -o $@ $(CFILES)

install-exec-local:

uninstall-local:

all-local: $(MKDISKFS_OBJ)

clean-local:
	rm -f $(MKDISKFS_OBJ)
