#! /bin/bash
set disassembly-flavor intel
target remote:1234
b *0x7c00 # break at the beginning
c
layout asm # break before and after getting info about the cursor
b *0x7c2d # break before and after getting info about the cursor
b *0x7c2f # break before and after getting info about the cursor
c
info registers #check the results
c
info registers #check the results