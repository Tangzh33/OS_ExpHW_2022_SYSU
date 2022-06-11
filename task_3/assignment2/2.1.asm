;-----OS_lab_3 written by Tang-----
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
mov ax, 0xb800
mov gs, ax

mov ax, 3;tang: clear the screen
int 10h

;----设置光标------
mov ah, 02h ;tang: Set the mode
mov bx, 00h ;set the page
mov dh, 2   ;set the row
mov dl, 0   ;set the colomn
int 10h

;----获取光标------
mov dh, 00h
mov dl, 00h ;clear before returning related info 
mov ah, 03h ;tang: Set the mode
mov bx, 00h ;set the page
int 10h

jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa