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

mov bl,0x47     ;set the color
mov bh,0
mov cx,1        ;setting the parameters for INT10H

mov dl, 4
mov dh, 7
mov ah, 02h
int 10h         ;seting the beginning pos is(4,7)

;----回显------
loop: ;tang: loop to replay the keyboard's data
    mov ah, 00h; set the int 16h mode
    int 16h ;the type-in info will return to al register
    
    inc dl
    mov ah, 02h
    int 10h ; move the cursor
    
    mov ah, 09h
    int 10h
    jmp loop


jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa