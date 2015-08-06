
include generic.am

SUBDIRS= kernel boot qlibc init rash qash coreutils calc edit \
         media gui winman qonqueror qonsole cpanel quafshot launcher \
         pascal tools

all-local:
	rm -rf $(top_srcdir)/autom4*
	rm -rf $(top_builddir)/autom4*

clean-local:
	rm -rf $(top_builddir)/disk
	rm -rf $(top_builddir)/iso
	rm -f $(top_builddir)/disk.img
	rm -f $(top_builddir)/quafios-2.0.1.iso
	- $(call REMOVE_EMPTY_DIR, $(top_builddir))

install-exec-local:
	mkdir -p ${prefix}/dev
	mkdir -p ${prefix}/tmp
	mkdir -p ${prefix}/mnt
	mkdir -p ${prefix}/run
	mkdir -p ${prefix}/sys

uninstall-local:
	- rm -d ${prefix}

setup:
	make install prefix=`pwd`/disk
	$(top_srcdir)/scripts/setup.sh

diskimg:
	make install prefix=`pwd`/disk
	mkdir -p `pwd`/iso
	dd if=/dev/zero of=disk.img bs=1M count=8 &> /dev/null
	echo -e "o\nn\np\n1\n2048\n\na\nw\n" | fdisk disk.img > /dev/null
	dd if=boot/mbr.bin of=disk.img bs=446 count=1 conv=notrunc \
	   &> /dev/null
	tools/mkdiskfs disk disk.img $(shell uuidgen) 1048576
	dd if=boot/vbr.bin of=disk.img bs=512 count=2 \
	   seek=2048 conv=notrunc  &> /dev/null

isolive: diskimg
	gzip disk.img -c > iso/ramdisk.gz
	cp boot/isolive.bin iso/isolive.bin
	mkisofs -R -b isolive.bin -boot-info-table \
		-boot-load-size 4 -no-emul-boot -graft-points \
		-input-charset utf-8 -o quafios-2.0.1.iso iso/

qemu-iso: isolive
	srcdir=$(top_srcdir) $(top_srcdir)/scripts/qemu.sh d

qemu-disk: diskimg
	srcdir=$(top_srcdir) $(top_srcdir)/scripts/qemu.sh c

xen-iso: isolive
	srcdir=$(top_srcdir) $(top_srcdir)/scripts/xen.sh
