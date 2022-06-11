run:
	@qemu-system-i386 -hda hd.img -serial null -parallel stdio 
debug_qemu:
	@make clean
	@make build
	@qemu-system-i386 -s -S -hda hd.img -serial null -parallel stdio &
debug_gdb:
	@gdb -q -x gdbinit
build:
	@nasm -g -f elf32 mbr.asm -o mbr.o
	@ld -o mbr.symbol -melf_i386 -N mbr.o -Ttext 0x7c00
	@ld -o mbr.bin -melf_i386 -N mbr.o -Ttext 0x7c00 --oformat binary

	@nasm -g -f elf32 bootloader.asm -o bootloader.o
	@ld -o bootloader.symbol -melf_i386 -N bootloader.o -Ttext 0x7e00
	@ld -o bootloader.bin -melf_i386 -N bootloader.o -Ttext 0x7e00 --oformat binary

	@qemu-img create hd.img 10m
	@dd if=mbr.bin of=hd.img bs=512 count=1 seek=0 conv=notrunc
	@dd if=bootloader.bin of=hd.img bs=512 count=5 seek=1 conv=notrunc
clean:
	@rm -fr *.bin *.o *.symbol *.img
Tang:
	@make clean
	@make build
	@make run
	@make clean