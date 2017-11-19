[bits 16]
[org 0x1000]
db 0x0f,0x1f,0xaa
jmp end
mov ah,0x0e
mov al,'.'
int 0x10
dap:db 0x10
db 0
db 2
dw 16
dw 0x8000
dw 0
dd 4
dd 0
end:mov ah,0x0e
mov al,'.'
int 0x10
mov ah,0x02
mov ch,0
mov al,58
mov cl,3
mov dh,0
mov dl,[0x9000]
mov bx,0x7e00
int 0x13
cmp ah,0
jz _ldr
jmp _err
_ldr:mov ah,0
mov al,0x13
int 0x10
mov ah,0x0e
mov al,'.'
int 0x10
jmp goprotected
jmp 0x8000
mov ah,0x0e
mov al,'G'
int 0x10
mov al,'O'
int 0x10
mov al,'I'
int 0x10
mov al,'N'
int 0x10
mov al,'G'
int 0x10
mov al,' '
int 0x10
mov al,'P'
int 0x10
mov al,'R'
int 0x10
mov al,'O'
int 0x10
mov al,'T'
int 0x10
mov al,'E'
int 0x10
mov al,'C'
int 0x10
mov al,'T'
int 0x10
mov al,'E'
int 0x10
mov al,'D'
int 0x10
_err:
mov ah,0x0e
mov al,'E'
int 0x10
_hng:jmp _hng
_check:cmp byte[0x7DF7],0x01
jz _check2
;mov ah,0x0e
;mov al,'0'
;int 0x10
jmp _err
_check2:cmp byte[0x7DF8],0x0f
jz _check3
mov ah,0x0e
mov al,'1'
int 0x10
jmp _err
_check3:mov ah,0x0e
mov al,'.'
int 0x10
cmp byte[0x7DF9],0xff
jz goprotected
mov ah,0x0e
mov al,'2'
int 0x10
jmp _err
load:;;mov ax,3
;int 0x10
jmp 0x07e03
goprotected:
;mov ah,0
;mov al,0x14
;int 0x10
;mov ax,3
;int 0x10
mov ah,0
mov al,0x3
int 0x10
cli
mov word [0x000],code
lgdt [gdtr]
sgdt [0x0000]
;lidt [idt]
mov eax,cr0
or eax,1
mov cr0,eax
mov ebx,0xb8000
;mov al,'.'
;mov [fs:ebx],al
jmp (code - null):finish
bits 32
finish:mov eax,0x10
mov ds,eax
mov es,eax
mov fs,eax
mov gs,eax
mov ss,eax
mov esp,0x1000
mov ebx,0xb8002
mov al,'!'
mov [fs:ebx],al
jmp 0x7e00
hng:jmp hng
idt:dw 0
dd 0
gdt:
null:dd 0
dd 0
code:dw 0xffff
dw 0
db 0
db 10011010b
db 11001111b
db 0
data:dw 0xffff
dw 0
db 0
db 10010010b
db 11001111b
db 0
gdt_end:
gdtr:dw gdt_end - null - 1
dd gdt

desc:dw gdt_end - gdt - 1
dd gdt

.hng:hlt
jmp .hng
times 512 - ($ - $$) db 0