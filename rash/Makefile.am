include $(top_srcdir)/generic.am

OBJECT_NAME   = rash
OBJECT_PREFIX = $(BIN_DIR)
LIBS          = -lc -lgcc
OBJ           = $(shell echo $(CFILES) | sed 's/\.c/\.o/g')
DEPS          = $(ALLHFILES) Makefile \
		$(GEN_INC_DIR)/kernel \
		$(GEN_INC_DIR)/libc \
		$(GEN_LIB_DIR)/libc

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJECT_NAME): $(OBJ)
	$(CC) $(LFLAGS) -o $@ $^ $(LIBS)

install-exec-local:
	$(INSTALL) -D $(OBJECT_NAME) $(OBJECT_PREFIX)/$(OBJECT_NAME)
	$(INSTALL) -D $(OBJECT_NAME) $(OBJECT_PREFIX)/sh

uninstall-local:
	rm -f $(OBJECT_PREFIX)/$(OBJECT_NAME)
	rm -f $(OBJECT_PREFIX)/sh
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: $(OBJECT_NAME)

clean-local:
	rm -f $(OBJ) $(OBJECT_NAME)
