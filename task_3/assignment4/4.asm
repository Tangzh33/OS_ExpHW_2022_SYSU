;-----OS_lab_3 written by Tang-----
;NOTICE: calculation between different 
;       size of reg needs to be careful
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

;------------------
;print name section
mov ah,0x0e
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, ' ';
int 10h
mov al, '2';
int 10h
mov al, '0'
int 10h
mov al, '3'
int 10h
mov al, '3'
int 10h
mov al, '7'
int 10h
mov al, '1'
int 10h
mov al, '1'
int 10h
mov al, '1'
int 10h
mov al, ' '
int 10h
mov al, 'T'
int 10h
mov al, 'a'
int 10h
mov al, 'n'
int 10h
mov al, 'g'
int 10h
mov al, 'Z'
int 10h
mov al, 'h'
int 10h
mov al, 'e'
int 10h

mov bl,0x47     ;set the color
mov bh,0

mov dl, 2       ;set the colomn
mov dh, 0       ;set the row
mov ah, 02h
int 10h         ;seting the beginning pos is(2,0)


flag_r dw 1
flag_c dw 1; saving for whether add or sub
row dw 0
col dw 2
;----字符弹射------
loop: ;
    mov cx, [row]
    mov dx, [col]; initialize the register

;------------------------------------
;boundary check
    push ax
    cmp dx, 0
        je boundary_colomn_0
    cmp dx, 79
        je boundary_colomn_80
boundary_colomn_end:
    cmp cx, 0
        je boundary_row_0 
    cmp cx, 23
        je boundary_row_24
boundary_row_end:
    pop ax

;------------------------------------
; move the cursor
    add cx, [flag_r]
    add dx, [flag_c]
    mov ah, dl
    mov al, cl
    and al, 0x7
    add al, 0x30
    mov [row], cx; update to the memory
    mov [col], dx

;------------------------------------
; print the cursor
    imul bx, cx, 80
    add bx, dx ; cor = 80 * row + col
    imul bx, 2    
    mov [gs:bx], ax

;-----------------------------------
; print the character symmetrically
    mov bx, 24
    sub bx, cx
    mov cx, bx
    mov bx, 80
    sub bx, dx
    mov dx, bx

    imul bx, cx, 80
    add bx, dx ; cor = 80 * row + col
    imul bx, 2    
    mov [gs:bx], ax
; -----------------------
; this section is used for delay
;therefore it can print the character slower
    push cx
    mov cx, 0x7fff
loop_delay_1:   
    push cx     
    mov cx, 0x007f
    loop_delay_2:
        loop loop_delay_2
    pop cx
    loop loop_delay_1
    pop cx

    jmp loop


boundary_row_0:
    mov ax, 1
    mov [flag_r], ax
    jmp boundary_row_end
boundary_colomn_0:
    mov ax, 1
    mov [flag_c], ax
    jmp boundary_colomn_end
boundary_row_24:
    mov ax, -1
    mov [flag_r], ax
    jmp boundary_row_end
boundary_colomn_80:
    mov ax, -1
    mov [flag_c], ax
    jmp boundary_colomn_end
loop_end:

jmp $ ; 死循环

times 510 - ($ - $$) db 0
db 0x55, 0xaa