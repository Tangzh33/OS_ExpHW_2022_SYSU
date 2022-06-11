;-----OS_lab_3 written by Tang-----
;TIPS: BE CARE WITH THE DATA ACCESS METHODS

%include "head.include"
;-----source code: If-----
; if a1 < 12 then
; 	[if_flag] = a1 / 2 + 1
; else if a1 < 24 then
; 	[if_flag] = (24 - a1) * a1
; else
; 	[if_flag] = a1 << 4
; end
your_if:
    mov eax, [a1]     ;initialize the parameter
    cmp eax, 12
    jl if_1         ;a1 < 12
        cmp eax, 24     ;a1 >= 12
        jl if_2         ;a1 < 24; 
            shl eax, 4      ;a1 << 4
            mov [if_flag], eax
    jmp end_if
if_1:
    mov edx, 0      ; Setting the divided-number
    mov ebx, 2
    idiv ebx        ; usage refer to the guidance
    inc eax         ;eax +=1
    mov [if_flag], eax
    jmp end_if
if_2:
    mov ebx, 24
    sub ebx, eax
    imul ebx, eax
    mov [if_flag], ebx
    jmp end_if
end_if:

;-----source code: While-----
; while a2 >= 12 then
; 	call my_random        // my_random将产生一个随机数放到eax中返回
; 	while_flag[a2 - 12] = eax
; 	--a2
; end

your_while:
; put your implementation here
while_loop:
    mov ebx, [a2]
    cmp ebx, 12
    jl end_while ;if a2 < 12 then end while
    ;boundary check

    call my_random

    mov ebx, [a2]
    add ebx, [while_flag]
    sub ebx, 12
    mov [ebx], al 
;calculating the address and save in to the memory

    mov ecx, [a2]
    dec ecx
    mov [a2], ecx ;changing the loop-parameters
    jmp while_loop
end_while:

%include "end.include"

;-----source code: Function-----
; your_function:
; 	for i = 0; string[i] != '\0'; ++i then
; 		pushad
; 		push string[i] to stack
; 		call print_a_char
; 		pop stack
; 		popad
; 	end
; 	return
; end

your_function:
; put your implementation here
    mov ecx, [your_string] ;initialize the para
loop_func:
    cmp byte[ecx], 0 ;byte is for correcting the bug
    je end_loop
    pushad
    mov edx, [ecx]
    push edx
    ; push byte[ecx]
    call print_a_char
    ; pop byte[ecx]
    pop edx
    popad
    inc ecx
    jmp loop_func
end_loop:
    ret
; end_myfunction:
