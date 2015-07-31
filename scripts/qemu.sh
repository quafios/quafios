#!/bin/bash

set -x

BUS=`lsusb|grep 'Super Top'|cut -d \  -f 2|cut -d : -f 1|sed 's/^0*//'`
DEV=`lsusb|grep 'Super Top'|cut -d \  -f 4|cut -d : -f 1|sed 's/^0*//'`
CORE="-device usb-host,bus=ehci.0,hostbus=$BUS,hostaddr=$DEV"
FLASH=`if [ -n "$BUS" ]; then echo $CORE; fi`

USB="-readconfig $srcdir/scripts/ich9-ehci-uhci.cfg $FLASH"

HDA="-drive file=disk.img,format=raw"
if [ $1 = "d" ]; then
	CDROM="-cdrom quafios-2.0.1.iso"
else
	CDROM=""
fi;
SND="-soundhw pcspk"

qemu -enable-kvm $CDROM $HDA $USB -boot $1 -m 128
