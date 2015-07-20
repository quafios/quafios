include $(top_srcdir)/generic.am

OBJECT_NAME   = libgui
DEPS          = $(ALLHFILES) Makefile \
                $(KERNEL_INCLUDE) \
		$(LIBC_INCLUDE) \
		$(LIBC_LIB)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

lib/$(OBJECT_NAME).a: $(OBJ)
	mkdir -p lib
	ar rcs $@ $^

install-exec-local: install-includes install-libraries

uninstall-local: uninstall-includes uninstall-libraries

all-local: lib/$(OBJECT_NAME).a

clean-local:
	rm -f $(OBJ) lib/$(OBJECT_NAME).a
	rm -rf lib
