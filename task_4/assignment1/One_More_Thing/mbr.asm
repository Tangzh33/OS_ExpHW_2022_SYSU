; org 0x7c00
%include "boot.inc"
[bits 16]

mov ax, 3;tang: clear the screen
int 10h
extern myreadsec

xor ax, ax ; eax = 0
; 初始化段寄存器, 段地址全部设为0
mov ds, ax
mov ss, ax
mov es, ax
mov fs, ax
mov gs, ax

; 初始化栈指针
mov sp, 0x7c00

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

[bits 32]           
protect_mode_begin:                              

mov eax, DATA_SELECTOR                     ;加载数据段(0..4GB)选择子
mov ds, eax
mov es, eax
mov eax, STACK_SELECTOR
mov ss, eax
mov eax, VIDEO_SELECTOR
mov gs, eax

; call debug_32
mov eax, 1                ; 逻辑扇区号
mov ebx, 0x7e00           ; bootloader的加载地址
load_bootloader:
    call asm_read_hard_disk  ; 读取硬盘
    inc eax
    cmp eax, 5
    jle load_bootloader
call debug_32
jmp dword CODE_SELECTOR:0x7e00        ; 跳转到bootloader

jmp $ ; 死循环

debug:
    pushad
    mov ax, 0xb800
    mov gs, ax ; 初始化段地址：显存显示
    mov ah, 0x47 ;设置颜色参数
    mov cx, bootloader_tag_end - bootloader_tag ;设置循环参数
    xor bx, bx ;显存段内偏移地址, bx初始化为0
    mov si, bootloader_tag ;显示字符串内容
    output_bootloader_tag:
        mov al, [si] ;循环输入字符串
        mov word[gs:bx], ax ;写入显存
        inc si
        add bx,2
        loop output_bootloader_tag
    popad
    ret

debug_32:
    pushad
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
    popad
    ret

asm_read_hard_disk: 
    pushad                  
    push eax
    push ebx
    call myreadsec
    add esp, 8
    popad
    add ebx, 200h
    ret


pgdt dw 0
     dd GDT_START_ADDRESS
bootloader_tag db 'Debug INFO'
bootloader_tag_end:
name_tag db '[Protect Mode]  TangZhe'
name_tag_end: