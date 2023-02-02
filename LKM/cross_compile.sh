#!/bin/bash

sudo pkill qemu-system-x86
make ARCH=x86_64 CROSS_COMPILE=x86_64-linux-gnu- && sudo cp krwx.ko ~/hacking/kernel/page-tables/rootfs/rootfs/krwx.ko
