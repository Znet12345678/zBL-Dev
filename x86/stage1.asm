bits 16
org 0x7c00
jmp a
db 0x1f
db 0xaf
db 0x0f
db 0x9f
a:mov ah,0x0e
mov al,'.'
int 0x10
fda:mov ax,ds
mov es,ax
mov bx,0x1000
mov ah,2
mov al,1
mov ch,0
mov cl,2
mov dh,0
mov dl,0
int 0x13
cmp ah,0
jz cmp1
fdb:mov ah,0x0e
mov al,'<'
int 0x10
mov dl,1
mov ah,2
mov al,1
int 0x13
cmp ah,0
jz cmp1
hda:mov ah,0x0e
mov al,'>'
int 0x10
mov dl,0x80
mov ah,2
mov al,1
int 0x13
cmp ah,0
jz cmp1
hdb:
mov ah,0x0e
mov al,'='
int 0x10
mov dl,0x81
int 0x13
cmp ah,0
jz cmp1
err:mov ah,0x0e
mov al,'e'
int 0x10
hng:jmp hng
cmp1:cmp byte [0x1000],0x0f
jz cmp2
jmp l
l:cmp dl,0
jz fdb
cmp dl,1
jz hda
cmp dl,0x80
jz hdb
mov ah,0x0e
mov al,':'
int 0x10
jz err
cmp2:
cmp byte [0x1001],0x1f
jz cmp3
jmp l
cmp3:
cmp byte [0x1002],0xaa
jz ldr
jmp l
ldr:mov ah,0x0e
mov al,'!'
int 0x10
mov [0x9000],dl
mov [0x500],dl
jmp 0:0x1003
times 510 - ($ - $$) db 0
dw 0xaa55