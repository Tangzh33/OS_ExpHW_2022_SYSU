org 0x7e00
[bits 16]

; mov ax, 3;tang: clear the screen
; int 10h 
; 为什么中断清屏失效？ 因为已经是32位了吗？


mov ax, 0xb800
mov gs, ax ; 初始化段地址：显存显示
mov ah, 0x47 ;设置颜色参数
mov ecx, bootloader_tag_end - bootloader_tag ;设置循环参数
xor ebx, ebx ;显存段内偏移地址, bx初始化为0
mov esi, bootloader_tag ;显示字符串内容
output_bootloader_tag:
    mov al, [esi] ;循环输入字符串
    mov word[gs:bx], ax ;写入显存
    inc esi
    add ebx,2
    loop output_bootloader_tag
jmp $ ; 死循环

bootloader_tag db '[LBA28]TangZh 20337111'
bootloader_tag_end: