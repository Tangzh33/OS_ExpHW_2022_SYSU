%include "boot.inc"
[bits 16]

; ---------clear the screen--------------
mov ax, 3;tang: clear the screen
int 10h

mov ax, 0xb800
mov gs, ax
mov ah, 0x47 ;Tang: My Color
mov ecx, num_tag_end - num_tag
mov ebx, 60
mov esi, num_tag
output_num_tag:
    mov al, [esi]
    mov word[gs:bx], ax
    inc esi
    add ebx,2
    loop output_num_tag

;空描述符
mov dword [GDT_START_ADDRESS+0x00],0x00
mov dword [GDT_START_ADDRESS+0x04],0x00  

;创建描述符，这是一个数据段，对应0~4GB的线性地址空间
mov dword [GDT_START_ADDRESS+0x08],0x0000ffff    ; 基地址为0，段界限为0xFFFFF
mov dword [GDT_START_ADDRESS+0x0c],0x00cf9200    ; 粒度为4KB，存储器段描述符 

;建立保护模式下的堆栈段描述符      
mov dword [GDT_START_ADDRESS+0x10],0x00000000    ; 基地址为0x00000000，界限0x0 
mov dword [GDT_START_ADDRESS+0x14],0x00409600    ; 粒度为1个字节

;建立保护模式下的显存描述符   
mov dword [GDT_START_ADDRESS+0x18],0x80007fff    ; 基地址为0x000B8000，界限0x07FFF 
mov dword [GDT_START_ADDRESS+0x1c],0x0040920b    ; 粒度为字节

;创建保护模式下平坦模式代码段描述符
mov dword [GDT_START_ADDRESS+0x20],0x0000ffff    ; 基地址为0，段界限为0xFFFFF
mov dword [GDT_START_ADDRESS+0x24],0x00cf9800    ; 粒度为4kb，代码段描述符 

;初始化描述符表寄存器GDTR
mov word [pgdt], 39      ;描述符表的界限   
lgdt [pgdt]
      
in al,0x92                         ;南桥芯片内的端口 
or al,0000_0010B
out 0x92,al                        ;打开A20

cli                                ;中断机制尚未工作
mov eax,cr0
or eax,1
mov cr0,eax                        ;设置PE位
      
;以下进入保护模式
jmp dword CODE_SELECTOR:protect_mode_begin

;16位的描述符选择子：32位偏移
;清流水线并串行化处理器
[bits 32]           
protect_mode_begin:                              

mov eax, DATA_SELECTOR                     ;加载数据段(0..4GB)选择子
mov ds, eax
mov es, eax
mov eax, STACK_SELECTOR
mov ss, eax
mov eax, VIDEO_SELECTOR
mov gs, eax

mov ecx, name_tag_end - name_tag
mov ebx, 80 * 2 + 58
mov esi, name_tag
mov ah, 0x47 ;Tang: My Color
output_name_tag:
    mov al, [esi]
    mov word[gs:ebx], ax
    add ebx, 2
    inc esi
    loop output_name_tag


; ------字符弹射-------------

mov bl,0x47     ;set the color
mov bh,0


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
    mov word[gs:bx], ax
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

pgdt dw 0
     dd GDT_START_ADDRESS

num_tag db '[Real Mode] 20337111'
num_tag_end:

name_tag db '[Protect Mode] TangZhe'
name_tag_end: