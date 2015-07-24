include $(top_srcdir)/generic.am

OBJECT_NAME   = coreutils
OBJECT_PREFIX = $(BIN_DIR)
LIBS          = -lc -lgcc
DEPS          = $(ALLHFILES) Makefile \
                $(KERNEL_INCLUDE) \
		$(LIBC_INCLUDE) \
		$(LIBC_LIB)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

umount: umount.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

mount: mount.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

rmdir: rmdir.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

mkdir: mkdir.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

rm: rm.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

unlink: unlink.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

link: link.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

mknod: mknod.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

dir: dir.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

ls: ls.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

cat: cat.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

readsect: readsect.o
	$(CC) $(LFLAGS) -o $@ $< $(LIBS)

install-exec-local:
	$(INSTALL) -D umount       $(OBJECT_PREFIX)/umount
	$(INSTALL) -D mount        $(OBJECT_PREFIX)/mount
	$(INSTALL) -D rmdir        $(OBJECT_PREFIX)/rmdir
	$(INSTALL) -D mkdir        $(OBJECT_PREFIX)/mkdir
	$(INSTALL) -D rm           $(OBJECT_PREFIX)/rm
	$(INSTALL) -D unlink       $(OBJECT_PREFIX)/unlink
	$(INSTALL) -D link         $(OBJECT_PREFIX)/link
	$(INSTALL) -D mknod        $(OBJECT_PREFIX)/mknod
	$(INSTALL) -D dir          $(OBJECT_PREFIX)/dir
	$(INSTALL) -D ls           $(OBJECT_PREFIX)/ls
	$(INSTALL) -D cat          $(OBJECT_PREFIX)/cat
	$(INSTALL) -D readsect     $(OBJECT_PREFIX)/readsect
	$(INSTALL) -D $(CSD)/free  $(OBJECT_PREFIX)/free
	$(INSTALL) -D $(CSD)/lsdev $(OBJECT_PREFIX)/lsdev

uninstall-local:
	rm -f $(OBJECT_PREFIX)/umount
	rm -f $(OBJECT_PREFIX)/mount
	rm -f $(OBJECT_PREFIX)/rmdir
	rm -f $(OBJECT_PREFIX)/mkdir
	rm -f $(OBJECT_PREFIX)/rm
	rm -f $(OBJECT_PREFIX)/unlink
	rm -f $(OBJECT_PREFIX)/link
	rm -f $(OBJECT_PREFIX)/mknod
	rm -f $(OBJECT_PREFIX)/dir
	rm -f $(OBJECT_PREFIX)/ls
	rm -f $(OBJECT_PREFIX)/cat
	rm -f $(OBJECT_PREFIX)/readsect
	rm -f $(OBJECT_PREFIX)/free
	rm -f $(OBJECT_PREFIX)/lsdev
	- $(call REMOVE_EMPTY_DIR, $(prefix))

all-local: umount mount rmdir mkdir rm unlink link mknod dir ls cat readsect

clean-local:
	rm -f $(OBJ) umount mount rmdir mkdir rm unlink link mknod dir ls \
	      cat readsect
