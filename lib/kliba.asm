
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;			       klib.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;							Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

; 导入全局变量
extern	disp_pos
extern  disp_int
extern  memcpy


[SECTION .text]

; 导出函数
global	disp_str
global	disp_color_str
global	out_byte
global	in_byte
global	enable_irq
global	disable_irq
global	enable_int
global	disable_int
global  read_mem_byte
global  read_mem_int
global  write_mem_byte
global 	write_mem_int
global  readelf


; ========================================================================
;		   void disp_str(char * info);
; ========================================================================
disp_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	cmp edi,100
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	cmp edi,4000
	jle .4
	mov edi,0
.4:
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

; ========================================================================
;		   void disp_color_str(char * info, int color);
; ========================================================================
disp_color_str:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12]	; color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi

	pop	ebp
	ret

; ========================================================================
;		   void out_byte(u16 port, u8 value);
; ========================================================================
out_byte:
	mov	edx, [esp + 4]		; port
	mov	al, [esp + 4 + 4]	; value
	out	dx, al
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;		   u8 in_byte(u16 port);
; ========================================================================
in_byte:
	mov	edx, [esp + 4]		; port
	xor	eax, eax
	in	al, dx
	nop	; 一点延迟
	nop
	ret

; ========================================================================
;		   void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
;	}
disable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, 1
	rol	ah, cl			; ah = (1 << (irq % 8))
	cmp	cl, 8
	jae	disable_8		; disable irq >= 8 at the slave 8259
disable_0:
	in	al, INT_M_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_M_CTLMASK, al	; set bit at master 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
disable_8:
	in	al, INT_S_CTLMASK
	test	al, ah
	jnz	dis_already		; already disabled?
	or	al, ah
	out	INT_S_CTLMASK, al	; set bit at slave 8259
	popf
	mov	eax, 1			; disabled by this function
	ret
dis_already:
	popf
	xor	eax, eax		; already disabled
	ret

; ========================================================================
;		   void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;	if(irq < 8){
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;	}
;	else{
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;	}
;
enable_irq:
	mov	ecx, [esp + 4]		; irq
	pushf
	cli
	mov	ah, ~1
	rol	ah, cl			; ah = ~(1 << (irq % 8))
	cmp	cl, 8
	jae	enable_8		; enable irq >= 8 at the slave 8259
enable_0:
	in	al, INT_M_CTLMASK
	and	al, ah
	out	INT_M_CTLMASK, al	; clear bit at master 8259
	popf
	ret
enable_8:
	in	al, INT_S_CTLMASK
	and	al, ah
	out	INT_S_CTLMASK, al	; clear bit at slave 8259
	popf
	ret


; ========================================================================
;		   write_mem_byte 
; ========================================================================
write_mem_byte:
	push ebp
	push ebx
	mov ebp,esp
	mov eax,[ebp + 12]
	mov ebx,eax
	mov al,[ebp + 16]
	mov [ds:ebx],al

	pop ebx
	pop ebp
	ret

; ========================================================================
;		   write_mem_int 
; ========================================================================
write_mem_int:
	push ebp
	push ebx
	mov ebp,esp
	mov ebx,[ebp + 12]
	mov eax,[ebp + 16]
	mov [ds:ebx],eax
	call disp_int
	pop ebx
	pop ebp
	ret
; ========================================================================
;		   read_mem_byte 
; ========================================================================
read_mem_byte:
	push edx
	xor edx,edx
	mov dl,[ds:eax]
	mov eax,edx
	pop edx
	ret

; ========================================================================
;		   read_mem_int 
; ========================================================================
read_mem_int:
	push edx
	xor edx,edx
	mov edx,[ds:eax]
	mov eax,edx
	pop edx
	ret


readelf:
	push 	ebp
	mov 	ebp,esp
	xor		esi, esi
	mov 	eax,[ebp+8]
	mov 	cx,word[eax+2Ch]  ; pELFHdr->e_phnum
	movzx	ecx, cx				
	mov 	eax,[ebp+8]
	mov 	esi,[eax+1Ch]     ; pELFHdr->e_phoff
	mov 	eax,esi
	add 	esi,[ebp+8]
	mov eax,esi               ; offsetOfFile + pELFHdr->e_phoff
.bgn:
	mov	eax, [esi + 0]
	cmp	eax, 0				  ; PT_NULL
	jz	.noaction
	push	dword [esi + 010h]		; size	┓
	mov		eax, [esi + 04h]		;	┃
	add 	eax,[ebp+8] 			;	┣ ::memcpy(	(void*)(pPHdr->p_vaddr),
	push	eax						; src	┃		uchCode + pPHdr->p_offset,
	push	dword [esi + 08h]		; dst	┃		pPHdr->p_filesz;
	call	memcpy					;	┃
	add	esp, 12						;		┛
.noaction:
	add	esi, 020h					; esi += pELFHdr->e_phentsize
	dec	ecx
	jnz	.bgn

	pop ebp
	ret
;	push eax
;	push ebx
;	mov ebx,[esp+4]
;	xor eax,eax
;	mov ax,[ds:ebx]
;	cmp ax,100h
;	jnz infite
;	pop ebx
;	pop eax
;	mov eax,100h
;	ret
;infite:
;	jmp $

; ========================================================================
;		   void disable_int();
; ========================================================================
disable_int:
	cli
	ret

; ========================================================================
;		   void enable_int();
; ========================================================================
enable_int:
	sti
	ret


