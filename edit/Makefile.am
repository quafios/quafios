include $(top_srcdir)/generic.am

OBJECT_NAME   = edit
OBJECT_PREFIX = $(BIN_DIR)
LIBS          = -lc -lgcc
DEPS          = $(ALLHFILES) Makefile \
                $(KERNEL_INCLUDE) \
		$(LIBC_INCLUDE) \
		$(LIBC_LIB)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

edit: $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

view: $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

install-exec-local:
	$(INSTALL) -D edit $(OBJECT_PREFIX)/edit
	$(INSTALL) -D view $(OBJECT_PREFIX)/view

uninstall-local:
	rm -f $(OBJECT_PREFIX)/edit
	rm -f $(OBJECT_PREFIX)/view
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: edit view

clean-local:
	rm -f $(OBJ) edit view
