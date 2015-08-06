#!/bin/bash

set -x

BUS=`lsusb|grep 'Super Top'|cut -d \  -f 2|cut -d : -f 1|sed 's/^0*//'`
DEV=`lsusb|grep 'Super Top'|cut -d \  -f 4|cut -d : -f 1|sed 's/^0*//'`
CORE="-device usb-host,bus=ehci.0,hostbus=$BUS,hostaddr=$DEV"
FLASH=`if [ -n "$BUS" ]; then echo $CORE; fi`

USBHUB="-usb -device usb-hub,bus=usb-bus.0,id=hub"

USBIMAGE="-drive if=none,id=stick,file=disk.img,format=raw  \
          -device usb-storage,bus=ehci.0,drive=stick"

USBMOUSE="-device usb-mouse,bus=ehci.0,usb_version=1"

USB2="-readconfig $srcdir/scripts/ich9-ehci-uhci.cfg"

USB="$USB2 $USBIMAGE"

if [ $1 = "d" ]; then
	CDROM="-cdrom quafios-2.0.1.iso"
else
	CDROM=""
fi;
SND="-soundhw pcspk"

qemu -enable-kvm $CDROM $USB -boot $1 -m 128
