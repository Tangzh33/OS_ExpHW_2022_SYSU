CFLAGS=-nostdinc -static -Os -c -m32 -g
# CFLAGS += -ffunction-sections和-fdata-sections
# CFLAGS += -Wl,--gc-sections
run:
	@qemu-system-i386 -hda hd.img -serial null -parallel stdio 
qemu_debug:
	@make clean
	@make build
	@qemu-system-i386 -hda hd.img -serial null -parallel stdio -s -S  
gdb_debug:
	@gdb -q -x gdbinit
mbr_build:
	@make clean
	@nasm -g -f elf32 mbr.asm -o mbr.o
	@echo $(CFLAGS)
	@gcc $(CFLAGS) -o my_readsec.o my_readsec.c
	@ld -o mbr.symbol -melf_i386 -N mbr.o my_readsec.o -Ttext 0x7c00
	@ld -o mbr.out 	  -melf_i386 -N mbr.o my_readsec.o -Ttext 0x7c00 
	@objcopy -O binary -j .text mbr.out mbr.bin
	@perl sign.pl mbr.bin
	@wc -c mbr.bin
bootloader_build:
	@nasm -g -f elf32 bootloader.asm -o bootloader.o
	@ld -o bootloader.symbol -melf_i386 -N bootloader.o -Ttext 0x7e00
	@ld -o bootloader.bin    -melf_i386 -N bootloader.o -Ttext 0x7e00 --oformat binary
build:
	@make mbr_build
	@make bootloader_build
	@qemu-img create hd.img 10m
	@dd if=mbr.bin 		  of=hd.img bs=512 count=1 seek=0 conv=notrunc
	@dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc
clean:
	@rm -rf *.bin *.o *.symbol *.img *.out
Tang:
	@make build
	@make run
	@make clean