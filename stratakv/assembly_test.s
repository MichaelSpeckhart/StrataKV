	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 26, 0	sdk_version 26, 1
	.globl	_loop_function                  ; -- Begin function loop_function
	.p2align	2
_loop_function:                         ; @loop_function
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #16                     ; Align the stack pointer to 16-byte
	.cfi_def_cfa_offset 16
	str	x0, [sp, #8]                    ; x0 is int* arr, the pointer to the passed in variable, store the x0 in address at stackpointer + 8
	str	wzr, [sp, #4]                   ; store wzr 32
	b	LBB0_1
LBB0_1:                                 ; =>This Inner Loop Header: Depth=1
	ldr	w8, [sp, #4]
	subs	w8, w8, #64
	b.ge	LBB0_4
	b	LBB0_2
LBB0_2:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldr	w8, [sp, #4]
	lsl	w8, w8, #6
	ldr	x9, [sp, #8]
	ldrsw	x10, [sp, #4]
	str	w8, [x9, x10, lsl #2]
	b	LBB0_3
LBB0_3:                                 ;   in Loop: Header=BB0_1 Depth=1
	ldr	w8, [sp, #4]
	add	w8, w8, #1
	str	w8, [sp, #4]
	b	LBB0_1
LBB0_4:
	add	sp, sp, #16
	ret
	.cfi_endproc
                                        ; -- End function
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	mov	x0, #256                        ; =0x100
	bl	_malloc
	str	x0, [sp, #8]
	mov	w0, #0                          ; =0x0
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
