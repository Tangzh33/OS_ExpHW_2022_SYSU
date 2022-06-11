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
mov cx,1        ;setting the parameters for INT09H

mov dl, 2       ;set the colomn
mov dh, 0       ;set the row
mov ah, 02h
int 10h         ;seting the beginning pos is(2,0)


flag_x db 1
flag_y db 1; saving for whether add or sub
;----字符弹射------
loop: ;
    cmp dl, 0
        je boundary_colomn_0
    cmp dl, 79
        je boundary_colomn_80
boundary_colomn_end:
    cmp dh, 0
        je boundary_row_0 
    cmp dh, 23
        je boundary_row_24
boundary_row_end:

    push ecx
    mov ecx, 0
loop_delay:
    cmp ecx, 0x7fff
    jl loop_end_delay
    inc ecx
    jmp loop_delay
loop_end_delay:
    pop ecx

    add dh, [flag_x]
    add dl, [flag_y]
    mov ah, 02h
    int 10h ; move the cursor
    
    inc bl  ;set the color
    mov al, bl
    mov ah, 09h
    int 10h
    jmp loop

loop_end:

boundary_row_0:
    mov byte[flag_x],1
    jmp boundary_row_end
boundary_colomn_0:
    mov byte[flag_y], 1
    jmp boundary_colomn_end
boundary_row_24:
    mov byte[flag_x], -1
    jmp boundary_row_end
boundary_colomn_80:
    mov byte[flag_y], -1
    jmp boundary_colomn_end
zero:

jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa