org 0x7c00
[bits 16]
xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00
mov ax, 1                ; 逻辑扇区号第0~15位
mov cx, 0                ; 逻辑扇区号第16~31位
mov bx, 0x7e00           ; bootloader的加载地址

; mov ax, 3;tang: clear the screen
; int 10h 

load_bootloader:
    call asm_read_hard_disk  ; 读取硬盘
    inc ax
    cmp ax, 5
    jle load_bootloader
jmp 0x0000:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

; chs c = 0, h = 0, s = 2 - 5

asm_read_hard_disk:                           
    mov ah, 02h
    mov al, 05h
    mov ch, 00h
    mov cl, 02h
    mov dh, 00h
    mov dl, 80h
    int 13h    
    ret

times 510 - ($ - $$) db 0
db 0x55, 0xaa