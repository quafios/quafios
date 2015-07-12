
include generic.am

SUBDIRS= kernel boot qlibc init udev shell rash coreutils calc edit \
         media gui winman qonqueror qonsole cpanel quafshot launcher \
         pascal format

all-local:
	rm -rf autom4*

clean-local:
	rm -rf $(top_srcdir)/include
	rm -rf $(top_srcdir)/lib
	rm -rf $(top_srcdir)/disk
	rm -rf $(top_srcdir)/iso
	rm -f $(top_srcdir)/disk.img
	rm -f $(top_srcdir)/quafios-1.0.2.iso

install-exec-local:
	mkdir -p ${prefix}/dev
	mkdir -p ${prefix}/tmp
	mkdir -p ${prefix}/mnt
	mkdir -p ${prefix}/run
	mkdir -p ${prefix}/sys

uninstall-local:
	- rm -d ${prefix}

isolive:
	make install prefix=`pwd`/disk
	mkdir -p `pwd`/iso
	dd if=/dev/zero of=disk.img bs=1M count=8 &> /dev/null
	format/format disk disk.img
	gzip -c disk.img > iso/ramdisk.gz
	cat disk/boot/cdboot disk/boot/loader.bin > iso/isoboot.bin
	mkisofs -R -b isoboot.bin -boot-info-table \
		-boot-load-size 4 -no-emul-boot -graft-points \
		-input-charset utf-8 -o quafios-1.0.2.iso iso/

qemu: isolive
	qemu -cdrom quafios-1.0.2.iso
