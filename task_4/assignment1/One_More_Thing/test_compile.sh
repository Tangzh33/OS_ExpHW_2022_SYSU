#! /bin/bash
nasm -g -f elf32 mbr.asm -o mbr.o
gcc -O2 -c -m32 -o my_readsec.o my_readsec.c

ld mbr.o my_readsec.o -o mbr.symbol -melf_i386 -N -Ttext 0x7c00
# ld -o mbr.bin -melf_i386 -N mbr.o my_readsec.o -Ttext 0x7c00 --oformat binary
rm -r *.o *.bin