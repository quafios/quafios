#!/bin/bash

echo
echo "+-----------------------------------+"
echo "|   Quafios Interactive Installer   |"
echo "+-----------------------------------+"
echo
echo Please insert path to device file of the disk \
     where you want to install Quafios \(ex: /dev/sda\):
read FILENAME
if [ ! -e "$FILENAME" ]; then
    echo "Error: File not found.";
    exit -1;
fi
if [ ! -w "$FILENAME" ]; then
    echo "Error: File doesn't have write permissions! (not root?)";
    exit -1;
fi
if [ -z "`fdisk -l "$FILENAME" 2>/dev/null | grep type | grep dos`" ]; then
    echo "Error: Quafios only supports MBR partition tables."
    exit -1
fi
echo
echo Listing of partitions by fdisk utility:
echo ----------------------------------------
fdisk -l "$FILENAME"
echo Enter number of system partition where you want to install \
     Quafios \(only primary partitions are supported\):
read PART
if [ `expr "$PART" : '[0-9]\+$'` == "0" ]; then
    echo "Error: Invalid input!";
    exit -1;
fi
PARTENTRY_E="`fdisk -l "$FILENAME" 2>/dev/null | grep -e ^$FILENAME$PART`"
PARTENTRY=${PARTENTRY_E//\*}
if [ -z "$PARTENTRY" ]; then
    echo "Error: Partition not found.";
    exit -1;
fi
if [ $PART -lt 1 -o $PART -gt 4 ]; then
    echo "Error: Only primary partitions are supported.";
    exit -1;
fi
echo
STARTSECT=`echo "$PARTENTRY" | awk '{ print $2; }'`
ENDSECT=`echo "$PARTENTRY" | awk '{ print $3; }'`
BLOCKCNT=`expr \( $ENDSECT - $STARTSECT + 1 \) / 2`
START=`expr $STARTSECT \* 512`
SIZE=`expr $BLOCKCNT \* 1024`
echo Partition: "$FILENAME"$PART
echo Offset: $START
echo Size: $SIZE
echo
echo Partition will be formatted and Quafios will be installed on \
     it. ALL DATA ON THE PARTITION WILL BE ERASED!! Note also that \
     THERE IS NO WARRANTY FOR THIS PROGRAM.
printf "Continue? [Y/n] "
read REPLY
if [ -n "$REPLY" -a "$REPLY" != "Y" -a "$REPLY" != "y" ]; then
    echo "Aborted by user.";
    exit -1;
fi;
tools/mkdiskfs disk "$FILENAME" `uuidgen` $START $SIZE
dd if=boot/vbr.bin of="$FILENAME" bs=512 count=2 \
   seek=$STARTSECT conv=notrunc  &> /dev/null
echo
echo Quafios boot loader will now be installed on your disk. If you \
     choose to proceed, this is will totally alter your current bootstrap \
     program by installing Quafios bootsector to the MBR, and will also \
     set Quafios partition as active. This step is optional\; you can \
     use another bootloader \(like GRUB\) to boot from Quafios partition \
     \(using chainloader command\).
printf "Continue? [Y/n] "
read REPLY
if [ -z "$REPLY" -o "$REPLY" == "Y" -o "$REPLY" == "y" ]; then
    dd if=boot/mbr.bin of="$FILENAME" bs=446 count=1 conv=notrunc &> /dev/null
    for i in {1..4}; do
        ENTRY="`fdisk -l "$FILENAME" 2>/dev/null | grep -e ^$FILENAME$i`"
        if [ -n "$ENTRY" ]; then
            BOOTABLE=$(expr "`echo "$ENTRY" | awk '{ print $2; }'`" == "*")
            if [ "$i" == "$PART" -a "$BOOTABLE" == "0" -o \
                 "$i" != "$PART" -a "$BOOTABLE" == "1" ]; then
                echo -e "a\n$i\nw\n" | fdisk "$FILENAME" 2>/dev/null 1>&2
            fi;
        fi;
    done;
    echo
    echo Bootloader installed successfully and partition is set active.
    echo
fi;
echo "Thank you for installing Quafios on your disk.";
