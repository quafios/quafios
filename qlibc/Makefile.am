include $(top_srcdir)/generic.am

OBJECT_NAME   = libc
OBJ           = $(shell echo $(CFILES) | sed 's/\.c/\.o/g')
DEPS          = $(ALLHFILES) Makefile \
		$(GEN_INC_DIR)/kernel

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

lib/$(OBJECT_NAME).a: $(OBJ)
	ar rcs $@ $^

install-exec-local: install-includes install-libraries

uninstall-local: uninstall-includes uninstall-libraries

all-local: lib/$(OBJECT_NAME).a \
	   $(GEN_INC_DIR)/$(OBJECT_NAME) \
	   $(GEN_LIB_DIR)/$(OBJECT_NAME)

clean-local: clean-includes clean-libraries
	rm -f $(OBJ) lib/$(OBJECT_NAME).a
