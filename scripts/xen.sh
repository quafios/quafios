#!/bin/bash

sudo modprobe xen-pciback
sudo xl pci-assignable-add 00:1d.0
sudo xl create $srcdir/scripts/hvm_domu_quafios.cfg
vncviewer localhost:5901
sudo xl destroy HVM_domU_quafios
