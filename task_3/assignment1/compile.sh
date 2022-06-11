#! /bin/bash
NAME=1.1
nasm -f bin $NAME.asm -o $NAME.bin
qemu-img create hd.img 10m
dd if=$NAME.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
qemu-system-i386 -hda hd.img -serial null -parallel stdio 
