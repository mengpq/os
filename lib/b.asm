extern milli_delay

[section .data]
	x dw 1                      ; 当前字符显示位置的行号,0~24
	y dw 64                     ; 当前字符显示位置的列号,0~79
	rdlu db 1                   ; 当前画框的方向, 1-向右,2-向下,3-向左,4-向上
	char db 'A'                 ; 当前显示字符

[section .text]
global PROCESSB

PROCESSB:
	mov byte[rdlu], 1             ; 当前画框的方向, 1-向右,2-向下,3-向左,4-向上
    mov word[char],'A'
	
loop1:
    call boxing
	mov eax,10
	push eax
	call milli_delay
	add esp,4
	jmp loop1
		
boxing:
	
right:
    mov al,byte[rdlu]           ;左-->右 
	cmp al,1
	jnz down
	mov ax,word[y]               ;最后一列?
	cmp ax, 78 
	jz r2d
	inc byte[y]
	jmp show
r2d:
    mov byte[rdlu],2           ;改为向下
	inc byte[x]
	jmp show
	
down:
    mov al,byte[rdlu]           ;向下 
	cmp al,2
	jnz left
	mov ax,word[x]               ;最后一行?
	cmp ax, 11
	jz d2l
	inc byte[x]
	jmp show
d2l:
    mov byte[rdlu],3           ;改为向左
	dec byte[y]
	jmp show

left:
    mov al,byte[rdlu]           ;向左 
	cmp al,3
	jnz up
	mov ax,word[y]               ;最左一列?
	cmp ax, 64 
	jz l2u
	dec byte[y]
	jmp show
l2u:
    mov byte[rdlu],4           ;改为向 上
	dec byte[x]
	jmp show
	
up:
    mov al,byte[rdlu]           ;向上 
	cmp al,4
	jnz end
	mov ax,word[x]               ;最上一行?
	cmp ax, 1
	jz u2r
	dec byte[x]
	jmp show
u2r:
    mov byte[rdlu],1           ;改为向右
	inc byte[y]
	mov al,byte[char]
	cmp al,'Z'
	jz returntoa
	inc byte[char]
	jmp show
	
returntoa:
    mov byte[char],'A' 
	jmp show

show:	
    xor ax,ax                      ; 计算当前字符的显存地址 gs:((80*x+y)*2)
    mov ax,word[x]
	mov bx,80                  ; (80*x
	mul bx
	add ax,word[y]             ; (80*x+y)
	mov bx,2
	mul bx                     ; ((80*x+y)*2)
	mov bp,ax
	mov ah,0Fh		   ; 0000：黑底、1111：亮白字（默认值为07h）
	mov al,byte[char]	   ; AL = 显示字符值（默认值为20h=空格符）
	mov word[gs:bp],ax  	   ;   显示字符的ASCII码值

	
end:
	ret


    jmp $	
