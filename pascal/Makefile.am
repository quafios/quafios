include $(top_srcdir)/generic.am

OBJECT_NAME   = pascal
OBJECT_PREFIX = $(BIN_DIR)
LIBS          = -lgui -lc -lgcc
DEPS          = $(ALLHFILES) Makefile \
                $(KERNEL_INCLUDE) \
		$(LIBC_INCLUDE) \
		$(LIBC_LIB) \
		$(LIBGUI_INCLUDE) \
		$(LIBGUI_LIB)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_NAME): $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

install-exec-local:
	$(INSTALL) -D $(OBJECT_NAME) $(OBJECT_PREFIX)/$(OBJECT_NAME)

uninstall-local:
	rm -f $(OBJECT_PREFIX)/$(OBJECT_NAME)
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: $(OBJECT_NAME)

clean-local:
	rm -f $(OBJ) $(OBJECT_NAME)
