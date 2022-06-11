[bits 32]

global asm_hello_world

asm_hello_world:
    pushad
    mov ecx, name_tag_end - name_tag
    mov ebx, 58
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
name_tag db 'TangZhe 20337111'
name_tag_end: