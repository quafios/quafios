include $(top_srcdir)/generic.am

OBJECT_NAME   = format
OBJECT_PREFIX = $(BIN_DIR)
DEPS          = $(CFILES) $(ALLHFILES) Makefile \
		$(GEN_INC_DIR)/kernel

$(OBJECT_NAME): $(DEPS)
	$(CC) -pedantic -o $@ $(CFILES)

install-exec-local:

uninstall-local:

all-local: $(OBJECT_NAME)

clean-local:
	rm -f $(OBJECT_NAME)
