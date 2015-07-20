#!/bin/bash

set -x

BUS=`lsusb|grep 'Super Top'|cut -d \  -f 2|cut -d : -f 1|sed 's/^0*//'`
DEV=`lsusb|grep 'Super Top'|cut -d \  -f 4|cut -d : -f 1|sed 's/^0*//'`
CORE="-device usb-host,hostbus=$BUS,hostaddr=$DEV"
FLASH=`if [ -n "$BUS" ]; then echo $CORE; fi`

HDA="-drive file=disk.img,format=raw"
CDROM="-cdrom quafios-2.0.1.iso"
SND="-soundhw pcspk"

qemu -enable-kvm $CDROM $HDA -usb $FLASH -boot d -m 128
