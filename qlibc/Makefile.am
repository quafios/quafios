include $(top_srcdir)/generic.am

OBJECT_NAME   = libc
DEPS          = $(ALLHFILES) Makefile \
		$(KERNEL_INCLUDE)

%.o:%.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

lib/$(OBJECT_NAME).a: $(OBJ)
	mkdir -p lib
	ar rcs $@ $^

install-exec-local: install-includes install-libraries

uninstall-local: uninstall-includes uninstall-libraries

all-local: make-dirs lib/$(OBJECT_NAME).a

clean-local:
	rm -f $(OBJ) lib/$(OBJECT_NAME).a
	rm -rf lib
