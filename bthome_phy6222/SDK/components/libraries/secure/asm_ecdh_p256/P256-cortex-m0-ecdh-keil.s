; P-256 ECDH
; Author: Emil Lenngren
; Licensed under the BSD 2-clause license.
; The 256x256->512 multiplication/square code is based on public domain µNaCl by Ana Helena Sánchez and Björn Haase (https://munacl.cryptojedi.org/curve25519-cortexm0.shtml)

; Note on calling conventions: some of the local functions in this file use custom calling conventions.
; Exported symbols use the standard C calling conventions for ARM, which means that r4-r11 and sp are preserved and the other registers are clobbered.

; Stack usage is 1356 bytes.

; Settings below for optimizing for speed vs size.
; When optimizing fully for speed, the run time is 4 456 738 cycles and code size is 3708 bytes.
; When optimizing fully for size, the run time is 5 764 182 cycles and code size is 2416 bytes.
; Get the time taken (in seconds) by dividing the number of cycles with the clock frequency (Hz) of the cpu.
; Different optimization levels can be done by setting some to 1 and some to 0.
; Optimizing all settings for size except use_mul_for_sqr and use_smaller_modinv gives a run time of 4 851 527 cycles and code size is 2968 bytes.

	gbla use_mul_for_sqr
use_mul_for_sqr seta 0 ; 1 to enable, 0 to disable (14% slower if enabled but saves 624/460 bytes (depending on use_noninlined_sqr64) of compiled code size)

	gbla use_noninlined_mul64
use_noninlined_mul64 seta 1 ; 1 to enable, 0 to disable (2.6%/4.0% slower (depending on use_mul_for_sqr) if enabled but saves 308 bytes of compiled code size)

	gbla use_noninlined_sqr64
use_noninlined_sqr64 seta 1 ; 1 to enable, 0 to disable (2.4% slower if enabled and use_mul_for_sqr=0 but saves 164 bytes of compiled code size)

	gbla use_interpreter
use_interpreter seta 1 ; 1 to enable, 0 to disable (3.6% slower if enabled but saves 268 bytes of compiled code size)

	gbla use_smaller_modinv
use_smaller_modinv seta 0 ; 1 to enable, 0 to disable (3.6% slower if enabled but saves 88 bytes of compiled code size)

	area |.text|,code,readonly
	align 2

	if use_noninlined_mul64 == 1
; in: (r4,r5) = a[0..1], (r2,r3) = b[0..1]
; out: r0-r3
; clobbers r4-r9 and lr
P256_mul64 proc
	mov r6,r5
	mov r1,r2
	subs r5,r4
	sbcs r0,r0
	eors r5,r0
	subs r5,r0
	subs r1,r3
	sbcs r7,r7
	eors r1,r7
	subs r1,r7
	eors r7,r0
	mov r9,r1
	mov r8,r5
	lsrs r1,r4,#16
	uxth r4,r4
	mov r0,r4
	uxth r5,r2
	lsrs r2,#16
	muls r0,r5,r0;//00
	muls r5,r1,r5;//10
	muls r4,r2,r4;//01
	muls r1,r2,r1;//11
	lsls r2,r4,#16
	lsrs r4,r4,#16
	adds r0,r2
	adcs r1,r4
	lsls r2,r5,#16
	lsrs r4,r5,#16
	adds r0,r2
	adcs r1,r4
	lsrs r4,r6,#16
	uxth r6,r6
	uxth r5,r3
	lsrs r3,r3,#16
	mov r2,r6
	muls r2,r5,r2
	muls r5,r4,r5
	muls r6,r3,r6
	muls r3,r4,r3
	lsls r4,r5,#16
	lsrs r5,r5,#16
	adds r2,r4
	adcs r3,r5
	lsls r4,r6,#16
	lsrs r5,r6,#16
	adds r2,r4
	adcs r3,r5
	eors r6,r6
	adds r2,r1
	adcs r3,r6
	mov r1,r9
	mov r5,r8
	mov r8,r0
	lsrs r0,r1,#16
	uxth r1,r1
	mov r4,r1
	lsrs r6,r5,#16
	uxth r5,r5
	muls r1,r5,r1
	muls r4,r6,r4
	muls r5,r0,r5
	muls r0,r6,r0
	lsls r6,r4,#16
	lsrs r4,#16
	adds r1,r6
	adcs r0,r4
	lsls r6,r5,#16
	lsrs r5,#16
	adds r1,r6
	adcs r0,r5
	eors r1,r7
	eors r0,r7
	eors r4,r4
	asrs r7,r7,#1
	adcs r1,r2
	adcs r2,r0
	adcs r7,r4
	mov r0,r8
	adds r1,r0
	adcs r2,r3
	adcs r3,r7
	bx lr
	endp
	endif

; in: *r10 = a, *r11 = b, (r4,r5) = a[0..1], (r2,r3) = b[0..1]
; out: r8,r9,r2-r7
; clobbers all other registers
P256_mul128 proc
	if use_noninlined_mul64 == 1
	push {lr}
	frame push {lr}
	endif
			;///////MUL128/////////////
			;MUL64
			if use_noninlined_mul64 == 0
			mov r6,r5
			mov r1,r2
			subs r5,r4
			sbcs r0,r0
			eors r5,r0
			subs r5,r0
			subs r1,r3
			sbcs r7,r7
			eors r1,r7
			subs r1,r7
			eors r7,r0
			mov r9,r1
			mov r8,r5
			lsrs r1,r4,#16
			uxth r4,r4
			mov r0,r4
			uxth r5,r2
			lsrs r2,#16
			muls r0,r5,r0;//00
			muls r5,r1,r5;//10
			muls r4,r2,r4;//01
			muls r1,r2,r1;//11
			lsls r2,r4,#16
			lsrs r4,r4,#16
			adds r0,r2
			adcs r1,r4
			lsls r2,r5,#16
			lsrs r4,r5,#16
			adds r0,r2
			adcs r1,r4
			lsrs r4,r6,#16
			uxth r6,r6
			uxth r5,r3
			lsrs r3,r3,#16
			mov r2,r6
			muls r2,r5,r2
			muls r5,r4,r5
			muls r6,r3,r6
			muls r3,r4,r3
			lsls r4,r5,#16
			lsrs r5,r5,#16
			adds r2,r4
			adcs r3,r5
			lsls r4,r6,#16
			lsrs r5,r6,#16
			adds r2,r4
			adcs r3,r5
			eors r6,r6
			adds r2,r1
			adcs r3,r6
			mov r1,r9
			mov r5,r8
			mov r8,r0
			lsrs r0,r1,#16
			uxth r1,r1
			mov r4,r1
			lsrs r6,r5,#16
			uxth r5,r5
			muls r1,r5,r1
			muls r4,r6,r4
			muls r5,r0,r5
			muls r0,r6,r0
			lsls r6,r4,#16
			lsrs r4,#16
			adds r1,r6
			adcs r0,r4
			lsls r6,r5,#16
			lsrs r5,#16
			adds r1,r6
			adcs r0,r5
			eors r1,r7
			eors r0,r7
			eors r4,r4
			asrs r7,r7,#1
			adcs r1,r2
			adcs r2,r0
			adcs r7,r4
			mov r0,r8
			adds r1,r0
			adcs r2,r3
			adcs r3,r7
			else
			bl P256_mul64
			endif
		push {r0,r1}
		frame address sp,8+use_noninlined_mul64*4
		mov r1,r10
		mov r10,r2
		ldm r1,{r0,r1,r4,r5}
		mov r2,r4
		mov r7,r5
		subs r2,r0
		sbcs r7,r1
		sbcs r6,r6
		eors r2,r6
		eors r7,r6
		subs r2,r6
		sbcs r7,r6
		push {r2,r7}
		frame address sp,16+use_noninlined_mul64*4
		mov r2,r11
		mov r11,r3
		ldm r2,{r0,r1,r2,r3}
		subs r0,r2
		sbcs r1,r3
		sbcs r7,r7
		eors r0,r7
		eors r1,r7
		subs r0,r7
		sbcs r1,r7
		eors r7,r6
		mov r12,r7
		push {r0,r1}
		frame address sp,24+use_noninlined_mul64*4
			;MUL64
			if use_noninlined_mul64 == 0
			mov r6,r5
			mov r1,r2
			subs r5,r4
			sbcs r0,r0
			eors r5,r0
			subs r5,r0
			subs r1,r3
			sbcs r7,r7
			eors r1,r7
			subs r1,r7
			eors r7,r0
			mov r9,r1
			mov r8,r5
			lsrs r1,r4,#16
			uxth r4,r4
			mov r0,r4
			uxth r5,r2
			lsrs r2,#16
			muls r0,r5,r0;//00
			muls r5,r1,r5;//10
			muls r4,r2,r4;//01
			muls r1,r2,r1;//11
			lsls r2,r4,#16
			lsrs r4,r4,#16
			adds r0,r2
			adcs r1,r4
			lsls r2,r5,#16
			lsrs r4,r5,#16
			adds r0,r2
			adcs r1,r4
			lsrs r4,r6,#16
			uxth r6,r6
			uxth r5,r3
			lsrs r3,r3,#16
			mov r2,r6
			muls r2,r5,r2
			muls r5,r4,r5
			muls r6,r3,r6
			muls r3,r4,r3
			lsls r4,r5,#16
			lsrs r5,r5,#16
			adds r2,r4
			adcs r3,r5
			lsls r4,r6,#16
			lsrs r5,r6,#16
			adds r2,r4
			adcs r3,r5
			eors r6,r6
			adds r2,r1
			adcs r3,r6
			mov r1,r9
			mov r5,r8
			mov r8,r0
			lsrs r0,r1,#16
			uxth r1,r1
			mov r4,r1
			lsrs r6,r5,#16
			uxth r5,r5
			muls r1,r5,r1
			muls r4,r6,r4
			muls r5,r0,r5
			muls r0,r6,r0
			lsls r6,r4,#16
			lsrs r4,#16
			adds r1,r6
			adcs r0,r4
			lsls r6,r5,#16
			lsrs r5,#16
			adds r1,r6
			adcs r0,r5
			eors r1,r7
			eors r0,r7
			eors r4,r4
			asrs r7,r7,#1
			adcs r1,r2
			adcs r2,r0
			adcs r7,r4
			mov r0,r8
			adds r1,r0
			adcs r2,r3
			adcs r3,r7
			else
			bl P256_mul64
			endif
		mov r4,r10
		mov r5,r11
		eors r6,r6
		adds r0,r4
		adcs r1,r5
		adcs r2,r6
		adcs r3,r6
		mov r10,r2
		mov r11,r3
		pop {r2-r5}
		frame address sp,8+use_noninlined_mul64*4
		push {r0,r1}
		frame address sp,16+use_noninlined_mul64*4
			if use_noninlined_mul64 == 0
			mov r6,r5
			mov r1,r2
			subs r5,r4
			sbcs r0,r0
			eors r5,r0
			subs r5,r0
			subs r1,r3
			sbcs r7,r7
			eors r1,r7
			subs r1,r7
			eors r7,r0
			mov r9,r1
			mov r8,r5
			lsrs r1,r4,#16
			uxth r4,r4
			mov r0,r4
			uxth r5,r2
			lsrs r2,#16
			muls r0,r5,r0;//00
			muls r5,r1,r5;//10
			muls r4,r2,r4;//01
			muls r1,r2,r1;//11
			lsls r2,r4,#16
			lsrs r4,r4,#16
			adds r0,r2
			adcs r1,r4
			lsls r2,r5,#16
			lsrs r4,r5,#16
			adds r0,r2
			adcs r1,r4
			lsrs r4,r6,#16
			uxth r6,r6
			uxth r5,r3
			lsrs r3,r3,#16
			mov r2,r6
			muls r2,r5,r2
			muls r5,r4,r5
			muls r6,r3,r6
			muls r3,r4,r3
			lsls r4,r5,#16
			lsrs r5,r5,#16
			adds r2,r4
			adcs r3,r5
			lsls r4,r6,#16
			lsrs r5,r6,#16
			adds r2,r4
			adcs r3,r5
			eors r6,r6
			adds r2,r1
			adcs r3,r6
			mov r1,r9
			mov r5,r8
			mov r8,r0
			lsrs r0,r1,#16
			uxth r1,r1
			mov r4,r1
			lsrs r6,r5,#16
			uxth r5,r5
			muls r1,r5,r1
			muls r4,r6,r4
			muls r5,r0,r5
			muls r0,r6,r0
			lsls r6,r4,#16
			lsrs r4,#16
			adds r1,r6
			adcs r0,r4
			lsls r6,r5,#16
			lsrs r5,#16
			adds r1,r6
			adcs r0,r5
			eors r1,r7
			eors r0,r7
			eors r4,r4
			asrs r7,r7,#1
			adcs r1,r2
			adcs r2,r0
			adcs r7,r4
			mov r0,r8
			adds r1,r0
			adcs r2,r3
			adcs r3,r7
			else
			bl P256_mul64
			endif
		pop {r4,r5}
		frame address sp,8+use_noninlined_mul64*4
		mov r6,r12
		mov r7,r12
		eors r0,r6
		eors r1,r6
		eors r2,r6
		eors r3,r6
		asrs r6,r6,#1
		adcs r0,r4
		adcs r1,r5
		adcs r4,r2
		adcs r5,r3
		eors r2,r2
		adcs r6,r2 ;//0,1
		adcs r7,r2
		pop {r2,r3}
		frame address sp,0+use_noninlined_mul64*4
		mov r8,r2
		mov r9,r3
		adds r2,r0
		adcs r3,r1
		mov r0,r10
		mov r1,r11
		adcs r4,r0
		adcs r5,r1
		adcs r6,r0
		adcs r7,r1
	if use_noninlined_mul64 == 1
	pop {pc}
	else
	bx lr
	endif
	endp
	
	if use_mul_for_sqr == 1
;thumb_func
P256_sqrmod ;label definition
	mov r2,r1
	; fallthrough
	endif

; *r0 = out, *r1 = a, *r2 = b
P256_mulmod proc
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	sub sp,#64
	frame address sp,72
	push {r1-r2}
	frame address sp,80
	mov r10,r2
	mov r11,r1
	mov r0,r2
	ldm r0!,{r4,r5}
	adds r0,#8
	ldm r1!,{r2,r3}
	adds r1,#8
	push {r0,r1}
	frame address sp,88
	
	bl P256_mul128
	add r0,sp,#24
	stm r0!,{r2,r3}
	add r0,sp,#16
	mov r2,r8
	mov r3,r9
	stm r0!,{r2,r3}

	;pop {r0} ;result+8
	;stm r0!,{r2,r3}
	pop {r1,r2} ;a+16 b+16
	frame address sp,80
	;push {r0}
	push {r4-r7}
	frame address sp,96
	mov r10,r1
	mov r11,r2
	ldm r1!,{r4,r5}
	ldm r2,{r2,r3}
	
	bl P256_mul128
	
	mov r0,r8
	mov r1,r9
	mov r8,r6
	mov r9,r7
	pop {r6,r7}
	frame address sp,88
	adds r0,r6
	adcs r1,r7
	pop {r6,r7}
	frame address sp,80
	adcs r2,r6
	adcs r3,r7
	;pop {r7} ;result+16
	add r7,sp,#24
	stm r7!,{r0-r3}
	mov r10,r7
	eors r0,r0
	mov r6,r8
	mov r7,r9
	adcs r4,r0
	adcs r5,r0
	adcs r6,r0
	adcs r7,r0
	pop {r1,r2} ;b a
	frame address sp,72
	mov r12,r2
	push {r4-r7}
	frame address sp,88
	ldm r1,{r0-r7}
	subs r0,r4
	sbcs r1,r5
	sbcs r2,r6
	sbcs r3,r7
	eors r4,r4
	sbcs r4,r4
	eors r0,r4
	eors r1,r4
	eors r2,r4
	eors r3,r4
	subs r0,r4
	sbcs r1,r4
	sbcs r2,r4
	sbcs r3,r4
	mov r6,r12
	mov r12,r4 ;//carry
	mov r5,r10
	stm r5!,{r0-r3}
	mov r11,r5
	mov r8,r0
	mov r9,r1
	ldm r6,{r0-r7}
	subs r4,r0
	sbcs r5,r1
	sbcs r6,r2
	sbcs r7,r3
	eors r0,r0
	sbcs r0,r0
	eors r4,r0
	eors r5,r0
	eors r6,r0
	eors r7,r0
	subs r4,r0
	sbcs r5,r0
	sbcs r6,r0
	sbcs r7,r0
	mov r1,r12
	eors r0,r1
	mov r1,r11
	stm r1!,{r4-r7}
	push {r0}
	frame address sp,92
	mov r2,r8
	mov r3,r9
	
	bl P256_mul128
	
	pop {r0} ;//r0,r1
	frame address sp,88
	mov r12,r0 ;//negative
	eors r2,r0
	eors r3,r0
	eors r4,r0
	eors r5,r0
	eors r6,r0
	eors r7,r0
	push {r4-r7}
	frame address sp,104
	add r1,sp,#32 ;result
	ldm r1!,{r4-r7}
	;mov r11,r1 ;//reference
	mov r1,r9
	eors r1,r0
	mov r10,r4
	mov r4,r8
	asrs r0,#1
	eors r0,r4
	mov r4,r10
	adcs r0,r4
	adcs r1,r5
	adcs r2,r6
	adcs r3,r7
	eors r4,r4
	adcs r4,r4
	mov r10,r4 ;//carry
	;mov r4,r11
	add r4,sp,#32+16
	ldm r4,{r4-r7}
	adds r0,r4
	adcs r1,r5
	adcs r2,r6
	adcs r3,r7
	mov r9,r4
	;mov r4,r11
	add r4,sp,#32+16
	stm r4!,{r0-r3}
	;mov r11,r4
	pop {r0-r3}
	frame address sp,88
	mov r4,r9
	adcs r4,r0
	adcs r5,r1
	adcs r6,r2
	adcs r7,r3
	movs r1,#0
	adcs r1,r1
	mov r0,r10
	mov r10,r1 ;//carry
	asrs r0,#1
	pop {r0-r3}
	frame address sp,72
	adcs r4,r0
	adcs r5,r1
	adcs r6,r2
	adcs r7,r3
	mov r8,r0
	;mov r0,r11
	add r0,sp,#32
	stm r0!,{r4-r7}
	;mov r11,r0
	mov r0,r8
	mov r6,r12
	mov r5,r10
	eors r4,r4
	adcs r5,r6
	adcs r6,r4
	adds r0,r5
	adcs r1,r6
	adcs r2,r6
	adcs r3,r6
	;mov r7,r11
	add r7,sp,#32+16
	stm r7!,{r0-r3}
	
	; multiplication done, now reducing

reduce ;label definition
	pop {r0-r7}
	frame address sp,40
	adds r3,r0
	adcs r4,r1
	adcs r5,r2
	adcs r6,r0
	mov r8,r2
	mov r9,r3
	mov r10,r4
	mov r11,r5
	mov r12,r6
	adcs r7,r1
	pop {r2-r5} ;8,9,10,11
	frame address sp,24
	adcs r2,r0 ;8+0
	adcs r3,r1 ;9+1
	movs r6,#0
	adcs r4,r6 ;10+#0
	adcs r5,r6 ;11+#0
	adcs r6,r6 ;C
	
	subs r7,r0 ;7-0
	sbcs r2,r1 ;8-1
	; r0,r1 dead
	mov r0,r8 ;2
	mov r1,r9 ;3
	sbcs r3,r0 ;9-2
	sbcs r4,r1 ;10-3
	movs r0,#0
	sbcs r5,r0 ;11-#0
	sbcs r6,r0 ;C-#0
	
	mov r0,r12 ;6
	adds r0,r1 ;6+3
	mov r12,r0
	mov r0,r10 ;4
	adcs r7,r0 ;7+4
	mov lr,r7
	mov r0,r8 ;2
	adcs r2,r0 ;8+2
	adcs r3,r1 ;9+3
	adcs r4,r0 ;10+2
	adcs r5,r1 ;11+3
	movs r7,#0
	adcs r6,r7 ;C+#0
	
	;2-3 are now dead (r8,r9)
	;4   5   6   7  8  9  10 11 C
	;r10 r11 r12 lr r2 r3 r4 r5 r6
	;r7: 0
	
	pop {r0,r1} ;12,13
	frame address sp,16
	
	adds r6,r0 ;12+C
	adcs r1,r7 ;13+#0
	adcs r7,r7 ;new Carry for 14
	
	;r0 dead
	
	mov r0,r11 ;5
	adds r2,r0 ;8+5
	mov r8,r2
	mov r2,r12 ;6
	adcs r3,r2 ;9+6
	mov r9,r3
	mov r3,r10 ;4
	adcs r4,r3 ;10+4
	mov r10,r4
	adcs r5,r0 ;11+5
	adcs r6,r3 ;12+4
	adcs r1,r0 ;13+5
	pop {r2,r4} ;14,15
	frame address sp,8
	adcs r2,r7 ;14+C
	movs r7,#0
	adcs r4,r7 ;15+#0
	adcs r7,r7 ;new Carry for 16
	
	;4  5  6   7  8  9  10  11 12 13 14 15 C
	;r3 r0 r12 lr r8 r9 r10 r5 r6 r1 r2 r4 r7
	;r11 is available
	
	subs r5,r3 ;11-4
	sbcs r6,r0 ;12-5
	mov r3,r12 ;6
	mov r0,lr ;7
	sbcs r1,r3 ;13-6
	sbcs r2,r0 ;14-7
	movs r3,#0
	sbcs r4,r3 ;15-#0
	sbcs r7,r3 ;C-#0
	mov lr,r4
	mov r11,r7
	
	mov r4,r10 ;10
	adds r4,r0 ;10+7
	adcs r5,r3 ;11+#0
	mov r7,r12 ;6
	adcs r6,r7 ;12+6
	adcs r1,r0 ;13+7
	adcs r2,r7 ;14+6
	mov r7,lr ;15
	adcs r7,r0 ;15+7
	mov r0,r11 ;C
	adcs r0,r3 ;C+#0
	
	; now (T + mN) / R is
	; 8 9 4 5 6 1 2 7 6 (lsb -> msb)
	
	subs r3,r3 ;set r3 to 0 and C to 1
	mov r10,r0
	mov r0,r8
	adcs r0,r3
	mov r11,r7
	mov r7,r9
	adcs r7,r3
	mov r12,r0
	mov r9,r7
	adcs r4,r3
	sbcs r5,r3
	sbcs r6,r3
	sbcs r1,r3
	movs r3,#1
	sbcs r2,r3
	movs r3,#0
	mov r0,r11
	mov r7,r10
	adcs r0,r3
	sbcs r7,r3
	
	; r12 r9 r4 r5 | r6 r1 r2 r0
	
	mov r8,r2
	mov r2,r12
	mov r11,r0
	mov r3,r9
reduce2 ;label definition
	adds r2,r7
	adcs r3,r7
	adcs r4,r7
	movs r0,#0
	adcs r5,r0
	adcs r6,r0
	adcs r1,r0
	pop {r0}
	frame address sp,4
	stm r0!,{r2-r6}
	movs r5,#1
	ands r5,r7
	mov r2,r8
	mov r3,r11
	adcs r2,r5
	adcs r3,r7
	stm r0!,{r1-r3}
	
	pop {pc}
	
	endp


	if use_mul_for_sqr == 0

	if use_noninlined_sqr64 == 1

P256_sqr64 proc
	 ; START: sqr 64 Refined Karatsuba
 ; Input operands in r4,r5
 ; Result in r0,r1,r2,r3
 ; Clobbers: r4-r6
 ; START: sqr 32
 ; Input operand in r4
 ; Result in r0 ,r1
 ; Clobbers: r2, r3
	uxth r0,r4
	lsrs r1,r4,#16
	mov r2,r0
	muls r2,r1,r2
	muls r0,r0,r0
	muls r1,r1,r1
	lsrs r3,r2,#15
	lsls r2,r2,#17
	adds r0,r2
	adcs r1,r3
 ; End: sqr 32
 ; Result in r0 ,r1
	subs r4,r5
	sbcs r6,r6
	eors r4,r6
	subs r4,r6
 ; START: sqr 32
 ; Input operand in r5
 ; Result in r2 ,r3
 ; Clobbers: r5, r6
	uxth r2,r5
	lsrs r3,r5,#16
	mov r5,r2
	muls r5,r3,r5
	muls r2,r2,r2
	muls r3,r3,r3
	lsrs r6,r5,#15
	lsls r5,r5,#17
	adds r2,r5
	adcs r3,r6
 ; End: sqr 32
 ; Result in r2 ,r3
	movs r6,#0
	adds r2,r1
	adcs r3,r6
 ; START: sqr 32
 ; Input operand in r4
 ; Result in r4 ,r5
 ; Clobbers: r1, r6
	lsrs r5,r4,#16
	uxth r4,r4
	mov r1,r4
	muls r1,r5,r1
	muls r4,r4,r4
	muls r5,r5,r5
	lsrs r6,r1,#15
	lsls r1,r1,#17
	adds r4,r1
	adcs r5,r6
 ; End: sqr 32
 ; Result in r4 ,r5
	mov r1,r2
	subs r1,r4
	sbcs r2,r5
	mov r5,r3
	movs r6,#0
	sbcs r3,r6
	adds r1,r0
	adcs r2,r5
	adcs r3,r6
 ; END: sqr 64 Refined Karatsuba
 ; Result in r0,r1,r2,r3
 ; Leaves r6 zero.
	bx lr
	endp

P256_sqr128 proc
	push {lr}
	frame push {lr}
 ; sqr 128 Refined Karatsuba
 ; Input in r4 ... r7
 ; Result in r0 ... r7
 ; clobbers all registers
	mov r0,r4
	mov r1,r5
	subs r0,r6
	sbcs r1,r7
	sbcs r2,r2
	eors r0,r2
	eors r1,r2
	subs r0,r2
	sbcs r1,r2
	mov r8,r0
	mov r9,r1
	mov r10,r6

	bl P256_sqr64

	mov r4,r10
	mov r5,r7
	mov r10,r0
	mov r11,r1
	mov r12,r2
	mov r7,r3

	bl P256_sqr64

	mov r4,r12
	adds r0,r4
	adcs r1,r7
	adcs r2,r6
	adcs r3,r6
	mov r7,r3
	mov r12,r0
	mov r4,r8
	mov r8,r1
	mov r5,r9
	mov r9,r2

	bl P256_sqr64

	mov r4,r12
	mov r5,r8
	mov r6,r9
	subs r4,r0
	sbcs r5,r1
	mov r0,r6
	mov r1,r7
	sbcs r0,r2
	sbcs r1,r3
	movs r2,#0
	sbcs r6,r2
	sbcs r7,r2
	mov r2,r10
	adds r2,r4
	mov r3,r11
	adcs r3,r5
	mov r4,r12
	adcs r4,r0
	mov r5,r8
	adcs r5,r1
	movs r0,#0
	adcs r6,r0
	adcs r7,r0
	mov r0,r10
	mov r1,r11
 ; END: sqr 128 Refined Karatsuba
	pop {pc}
	endp
	else
P256_sqr128 proc
 ; sqr 128 Refined Karatsuba
 ; Input in r4 ... r7
 ; Result in r0 ... r7
 ; clobbers all registers
	mov r0,r4
	mov r1,r5
	subs r0,r6
	sbcs r1,r7
	sbcs r2,r2
	eors r0,r2
	eors r1,r2
	subs r0,r2
	sbcs r1,r2
	mov r8,r0
	mov r9,r1
	mov r10,r6
 ; START: sqr 64 Refined Karatsuba
 ; Input operands in r4,r5
 ; Result in r0,r1,r2,r3
 ; Clobbers: r4-r6
 ; START: sqr 32
 ; Input operand in r4
 ; Result in r0 ,r1
 ; Clobbers: r2, r3
	uxth r0,r4
	lsrs r1,r4,#16
	mov r2,r0
	muls r2,r1,r2
	muls r0,r0,r0
	muls r1,r1,r1
	lsrs r3,r2,#15
	lsls r2,r2,#17
	adds r0,r2
	adcs r1,r3
 ; End: sqr 32
 ; Result in r0 ,r1
	subs r4,r5
	sbcs r6,r6
	eors r4,r6
	subs r4,r6
 ; START: sqr 32
 ; Input operand in r5
 ; Result in r2 ,r3
 ; Clobbers: r5, r6
	uxth r2,r5
	lsrs r3,r5,#16
	mov r5,r2
	muls r5,r3,r5
	muls r2,r2,r2
	muls r3,r3,r3
	lsrs r6,r5,#15
	lsls r5,r5,#17
	adds r2,r5
	adcs r3,r6
 ; End: sqr 32
 ; Result in r2 ,r3
	movs r6,#0
	adds r2,r1
	adcs r3,r6
 ; START: sqr 32
 ; Input operand in r4
 ; Result in r4 ,r5
 ; Clobbers: r1, r6
	lsrs r5,r4,#16
	uxth r4,r4
	mov r1,r4
	muls r1,r5,r1
	muls r4,r4,r4
	muls r5,r5,r5
	lsrs r6,r1,#15
	lsls r1,r1,#17
	adds r4,r1
	adcs r5,r6
 ; End: sqr 32
 ; Result in r4 ,r5
	mov r1,r2
	subs r1,r4
	sbcs r2,r5
	mov r5,r3
	movs r6,#0
	sbcs r3,r6
	adds r1,r0
	adcs r2,r5
	adcs r3,r6
 ; END: sqr 64 Refined Karatsuba
 ; Result in r0,r1,r2,r3
 ; Leaves r6 zero.
	mov r6,r10
	mov r10,r0
	mov r11,r1
	mov r12,r2
	mov r1,r3
 ; START: sqr 64 Refined Karatsuba
 ; Input operands in r6,r7
 ; Result in r2,r3,r4,r5
 ; Clobbers: r0,r7,r6
 ; START: sqr 32
 ; Input operand in r6
 ; Result in r2 ,r3
 ; Clobbers: r4, r5
	uxth r2,r6
	lsrs r3,r6,#16
	mov r4,r2
	muls r4,r3,r4
	muls r2,r2,r2
	muls r3,r3,r3
	lsrs r5,r4,#15
	lsls r4,r4,#17
	adds r2,r4
	adcs r3,r5
 ; End: sqr 32
 ; Result in r2 ,r3
	subs r6,r7
	sbcs r4,r4
	eors r6,r4
	subs r6,r4
 ; START: sqr 32
 ; Input operand in r7
 ; Result in r4 ,r5
 ; Clobbers: r0, r7
	uxth r4,r7
	lsrs r5,r7,#16
	mov r0,r4
	muls r0,r5,r0
	muls r4,r4,r4
	muls r5,r5,r5
	lsrs r7,r0,#15
	lsls r0,r0,#17
	adds r4,r0
	adcs r5,r7
 ; End: sqr 32
 ; Result in r4 ,r5
	movs r7,#0
	adds r4,r3
	adcs r5,r7
 ; START: sqr 32
 ; Input operand in r6
 ; Result in r7 ,r0
 ; Clobbers: r6, r3
	uxth r7,r6
	lsrs r0,r6,#16
	mov r6,r7
	muls r6,r0,r6
	muls r7,r7,r7
	muls r0,r0,r0
	lsrs r3,r6,#15
	lsls r6,r6,#17
	adds r7,r6
	adcs r0,r3
 ; End: sqr 32
 ; Result in r7 ,r0
	mov r3,r4
	subs r3,r7
	sbcs r4,r0
	mov r0,r5
	movs r6,#0
	sbcs r5,r6
	adds r3,r2
	adcs r4,r0
	adcs r5,r6
 ; END: sqr 64 Refined Karatsuba
 ; Result in r2,r3,r4,r5
 ; Leaves r6 zero.
	mov r0,r12
	adds r2,r0
	adcs r3,r1
	adcs r4,r6
	adcs r5,r6
	mov r12,r2
	mov r2,r8
	mov r8,r3
	mov r3,r9
	mov r9,r4
 ; START: sqr 64 Refined Karatsuba
 ; Input operands in r2,r3
 ; Result in r6,r7,r0,r1
 ; Clobbers: r2,r3,r4
 ; START: sqr 32
 ; Input operand in r2
 ; Result in r6 ,r7
 ; Clobbers: r0, r1
	uxth r6,r2
	lsrs r7,r2,#16
	mov r0,r6
	muls r0,r7,r0
	muls r6,r6,r6
	muls r7,r7,r7
	lsrs r1,r0,#15
	lsls r0,r0,#17
	adds r6,r0
	adcs r7,r1
 ; End: sqr 32
 ; Result in r6 ,r7
	subs r2,r3
	sbcs r4,r4
	eors r2,r4
	subs r2,r4
 ; START: sqr 32
 ; Input operand in r3
 ; Result in r0 ,r1
 ; Clobbers: r3, r4
	uxth r0,r3
	lsrs r1,r3,#16
	mov r3,r0
	muls r3,r1,r3
	muls r0,r0,r0
	muls r1,r1,r1
	lsrs r4,r3,#15
	lsls r3,r3,#17
	adds r0,r3
	adcs r1,r4
 ; End: sqr 32
 ; Result in r0 ,r1
	movs r4,#0
	adds r0,r7
	adcs r1,r4
 ; START: sqr 32
 ; Input operand in r2
 ; Result in r3 ,r4
 ; Clobbers: r2, r7
	uxth r3,r2
	lsrs r4,r2,#16
	mov r2,r3
	muls r2,r4,r2
	muls r3,r3,r3
	muls r4,r4,r4
	lsrs r7,r2,#15
	lsls r2,r2,#17
	adds r3,r2
	adcs r4,r7
 ; End: sqr 32
 ; Result in r3 ,r4
	mov r7,r0
	subs r7,r3
	sbcs r0,r4
	mov r2,r1
	movs r4,#0
	sbcs r1,r4
	adds r7,r6
	adcs r0,r2
	adcs r1,r4
 ; END: sqr 64 Refined Karatsuba
 ; Result in r6,r7,r0,r1
 ; Returns r4 as zero.
	mov r2,r12
	mov r3,r8
	mov r4,r9
	subs r2,r6
	sbcs r3,r7
	mov r6,r4
	mov r7,r5
	sbcs r4,r0
	sbcs r5,r1
	movs r0,#0
	sbcs r6,r0
	sbcs r7,r0
	mov r0,r10
	adds r2,r0
	mov r1,r11
	adcs r3,r1
	mov r0,r12
	adcs r4,r0
	mov r0,r8
	adcs r5,r0
	movs r0,#0
	adcs r6,r0
	adcs r7,r0
	mov r0,r10
 ; END: sqr 128 Refined Karatsuba
 ; Result in r0 ... r7
	bx lr
	endp
	
	endif

; ######################
; ASM Square 256 refined karatsuba:
; ######################
 ; sqr 256 Refined Karatsuba
 ; pInput in r1
 ; pResult in r0
P256_sqrmod proc
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	sub sp,#64
	frame address sp,72
	;mov lr,sp
	push {r1}
	frame address sp,76
	ldm r1!,{r4,r5,r6,r7}

	bl P256_sqr128

	push {r4,r5,r6,r7}
	frame address sp,92
	;mov r4,lr
	add r4,sp,#20
	stm r4!,{r0,r1,r2,r3}
	ldr r4,[sp,#16]
	adds r4,#16
	ldm r4,{r4,r5,r6,r7}

	bl P256_sqr128

	mov r8,r4
	mov r9,r5
	mov r10,r6
	mov r11,r7
	pop {r4,r5,r6,r7}
	frame address sp,76
	adds r0,r4
	adcs r1,r5
	adcs r2,r6
	adcs r3,r7
	mov r4,r8
	mov r5,r9
	mov r6,r10
	mov r7,r11
	mov r8,r0
	movs r0,#0
	adcs r4,r0
	adcs r5,r0
	adcs r6,r0
	adcs r7,r0
	mov r0,r8
	push {r0,r1,r2,r3,r4,r5,r6,r7}
	frame address sp,108
	ldr r4,[sp,#32]
	ldm r4,{r0,r1,r2,r3,r4,r5,r6,r7}
	subs r4,r0
	sbcs r5,r1
	sbcs r6,r2
	sbcs r7,r3
	sbcs r0,r0
	eors r4,r0
	eors r5,r0
	eors r6,r0
	eors r7,r0
	subs r4,r0
	sbcs r5,r0
	sbcs r6,r0
	sbcs r7,r0

	bl P256_sqr128

	mvns r0,r0
	mvns r1,r1
	mvns r2,r2
	mvns r3,r3
	mvns r4,r4
	mvns r5,r5
	mvns r6,r6
	mvns r7,r7
	mov r8,r4
	mov r9,r5
	mov r10,r6
	mov r11,r7
	subs r4,r4
	pop {r4,r5,r6,r7}
	frame address sp,92
	adcs r0,r4
	adcs r1,r5
	adcs r2,r6
	adcs r3,r7
	mov r12,r4
	;movs r4,#16
	;add r4,lr
	add r4,sp,#20+16
	stm r4!,{r0,r1,r2,r3}
	mov r4,r12
	mov r0,r8
	adcs r0,r4
	mov r8,r0
	mov r1,r9
	adcs r1,r5
	mov r9,r1
	mov r2,r10
	adcs r2,r6
	mov r10,r2
	mov r3,r11
	adcs r3,r7
	mov r11,r3
	movs r0,#0
	adcs r0,r0
	mov r12,r0
	;mov r0,lr
	add r0,sp,#20
	ldm r0,{r0,r1,r2,r3,r4,r5,r6,r7}
	adds r0,r4
	adcs r1,r5
	adcs r2,r6
	adcs r3,r7
	;movs r4,#16
	;add r4,lr
	add r4,sp,#20+16
	stm r4!,{r0,r1,r2,r3}
	;mov lr,r4
	mov r0,r13
	ldm r0!,{r4,r5,r6,r7}
	mov r1,r8
	adcs r4,r1
	mov r1,r9
	adcs r5,r1
	mov r1,r10
	adcs r6,r1
	mov r1,r11
	adcs r7,r1
	;mov r0,lr
	add r0,sp,#20+32
	stm r0!,{r4,r5,r6,r7}
	pop {r4,r5,r6,r7}
	frame address sp,76
	mov r1,r12
	movs r2,#0
	mvns r2,r2
	adcs r1,r2
	asrs r2,r1,#4
	adds r4,r1
	adcs r5,r2
	adcs r6,r2
	adcs r7,r2
	stm r0!,{r4,r5,r6,r7}
	add sp,#4
	frame address sp,72
	b reduce
	endp
	endif

; *r0 = output, *r1 = a, *r2 = b
P256_addmod proc
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	ldm r1!,{r0,r3,r4}
	ldm r2!,{r5,r6,r7}
	adds r0,r5
	adcs r3,r6
	adcs r4,r7
	mov r8,r0
	mov r9,r3
	mov r10,r4
	ldm r1!,{r5,r6}
	ldm r2!,{r3,r4}
	adcs r5,r3
	adcs r6,r4
	ldm r1,{r1,r3,r4}
	ldm r2,{r0,r2,r7}
	adcs r1,r0
	adcs r3,r2
	adcs r4,r7
	movs r7,#0
	adcs r7,r7
	
	subs r0,r0 ;set r0 to 0 and C to 1
	mov r2,r8
	mov r8,r7
	mov r7,r9
	mov r9,r4
	mov r4,r10
	adcs r2,r0
	mov r10,r2
	adcs r7,r0
	mov r11,r7
	adcs r4,r0
	sbcs r5,r0
	sbcs r6,r0
	sbcs r1,r0
	movs r0,#1
	sbcs r3,r0
	movs r0,#0
	mov r2,r9
	adcs r2,r0
	mov r7,r8
	sbcs r7,r0
	
	; r10 r11 r4 r5 | r6 r1 r3 r2 | r7
	
	mov r8,r3
	mov r3,r11
	mov r11,r2
	mov r2,r10
	
	; r2 r3 r4 r5 | r6 r1 r8 r11 | r7
	
	b reduce2
	
	endp
	
; *r0 = output, *r1 = a, *r2 = b
P256_submod proc
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	ldm r1!,{r0,r3,r4}
	ldm r2!,{r5,r6,r7}
	subs r0,r5
	sbcs r3,r6
	sbcs r4,r7
	mov r8,r0
	mov r9,r3
	mov r10,r4
	ldm r1!,{r5,r6}
	ldm r2!,{r3,r4}
	sbcs r5,r3
	sbcs r6,r4
	ldm r1,{r1,r3,r4}
	ldm r2,{r0,r2,r7}
	sbcs r1,r0
	sbcs r3,r2
	sbcs r4,r7
	
	sbcs r7,r7
	
	mov r2,r8
	mov r8,r3
	mov r11,r4
	mov r3,r9
	mov r4,r10
	b reduce2
	
	endp

; in: *r0 = output (8 words)
; out: r0 is preserved
P256_load_1 proc
	movs r1,#1
	stm r0!,{r1}
	movs r1,#0
	movs r2,#0
	stm r0!,{r1-r2}
	stm r0!,{r1-r2}
	stm r0!,{r1-r2}
	stm r0!,{r1}
	subs r0,#32
	bx lr
	endp

; in: *r1
; out: *r0
P256_to_montgomery proc
	push {r4-r7,lr}
	frame push {r4-r7,lr}
	adr r2,P256_R2_mod_p
	bl P256_mulmod
	pop {r4-r7,pc}
	endp

	align 4
	; (2^256)^2 mod p
P256_R2_mod_p
	dcd 3
	dcd 0
	dcd 0xffffffff
	dcd 0xfffffffb
	dcd 0xfffffffe
	dcd 0xffffffff
	dcd 0xfffffffd
	dcd 4

; in: *r1
; out: *r0
P256_from_montgomery proc
	push {r4-r7,lr}
	frame push {r4-r7,lr}
	movs r2,#0
	movs r3,#0
	push {r2-r3}
	frame address sp,28
	push {r2-r3}
	frame address sp,36
	push {r2-r3}
	frame address sp,44
	movs r2,#1
	push {r2-r3}
	frame address sp,52
	mov r2,sp
	bl P256_mulmod
	add sp,#32
	frame address sp,20
	pop {r4-r7,pc}
	endp

; Elliptic curve operations on the NIST curve P256

; Checks if a point is on curve
; in: *r0 = x,y(,scratch) in Montgomery form
; out: r0 = 1 if on curve, otherwise 0
P256_point_is_on_curve proc
	if use_interpreter == 1
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	adr r2,P256_point_is_on_curve_program
	bl P256_interpreter
	ldr r0,[sp]
	adds r0,#64
	adr r1,P256_b_mont
	bl P256_greater_or_equal_than
	beq %f0
	adr r0,P256_b_mont
	ldr r1,[sp]
	adds r1,#64
	bl P256_greater_or_equal_than
0
	pop {r1,pc}
	else
	push {r0,r4-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,24
	
	; We verify y^2 - (x^3 - 3x) = b
	
	; y^2
	mov r1,r0
	adds r1,#32
	sub sp,#32
	frame address sp,56
	mov r0,sp
	bl P256_sqrmod
	
	; x^2
	ldr r1,[sp,#32]
	sub sp,#32
	frame address sp,88
	mov r0,sp
	bl P256_sqrmod
	
	; x^3
	mov r0,sp
	ldr r1,[sp,#64]
	mov r2,sp
	bl P256_mulmod
	
	; x^3 - 3x
	movs r0,#3
0
	push {r0}
	frame address sp,92
	add r0,sp,#4
	add r1,sp,#4
	ldr r2,[sp,#68]
	bl P256_submod
	pop {r0}
	frame address sp,88
	subs r0,#1
	bne %b0
	
	; y^2 - (x^3 - 3x)
	mov r0,sp
	add r1,sp,#32
	mov r2,sp
	bl P256_submod
	
	; compare with b
	mov r0,sp
	adr r1,P256_b_mont
	bl P256_greater_or_equal_than
	beq %f1
	adr r0,P256_b_mont
	mov r1,sp
	bl P256_greater_or_equal_than
1
	add sp,#68
	frame address sp,20
	
	pop {r4-r7,pc}
	endif
	
	endp

	align 4
P256_b_mont
	dcd 0x29c4bddf
	dcd 0xd89cdf62
	dcd 0x78843090
	dcd 0xacf005cd
	dcd 0xf7212ed6
	dcd 0xe5a220ab
	dcd 0x04874834
	dcd 0xdc30061d
		
	if use_interpreter == 1
P256_point_is_on_curve_program
	dcw 0x2040
	dcw 0x2130
	dcw 0x1113
	dcw 0x4113
	dcw 0x4113
	dcw 0x4113
	dcw 0x4501
	dcw 0x0000
	endif

; input: *r0 = value, *r1 = limit
; output: 1 if value >= limit, otherwise 0
P256_greater_or_equal_than proc
	push {r4-r6,lr}
	frame push {r4-r6,lr}
	subs r5,r5 ; set r5 to 0 and C to 1
	mvns r6,r5 ; set r6 to -1
	movs r2,#8
0
	ldm r0!,{r3}
	ldm r1!,{r4}
	sbcs r3,r4
	add r2,r2,r6
	tst r2,r2
	bne %b0
	
	adcs r5,r5
	mov r0,r5
	pop {r4-r6,pc}
	endp

; in: *r0 = output location, *r1 = input, *r2 = 0/1, *r3 = m
; if r2 = 0, then *r0 is set to *r1
; if r2 = 1, then *r0 is set to m - *r1
; note that *r1 should be in the range [1,m-1]
; out: r0 and r1 will have advanced 32 bytes, r2 will remain as the input
P256_negate_mod_m_if proc
	push {r4-r7,lr}
	frame push {r4-r7,lr}
	movs r4,#1
	rsbs r5,r4,#0 ; r5=-1
	mov r8,r5
	subs r4,r4,r2 ; r4=!r2, C=1
	movs r6,#8
0
	ldm r1!,{r5}
	ldm r3!,{r7}
	sbcs r7,r5
	muls r7,r2,r7
	muls r5,r4,r5
	add r7,r7,r5
	stm r0!,{r7}
	add r6,r6,r8
	tst r6,r6
	bne %b0
	
	pop {r4-r7,pc}
	endp

; copies 8 words
; in: *r0 = result, *r1 = input
; out: *r0 = end of result, *r1 = end of input
P256_copy32 proc
	push {r4-r5,lr}
	frame push {r4-r5,lr}
	ldm r1!,{r2-r5}
	stm r0!,{r2-r5}
	ldm r1!,{r2-r5}
	stm r0!,{r2-r5}
	pop {r4-r5,pc}
	endp


; copies 32 bytes
; in: *r0 = result, *r1 = input
; out: *r0 = end of result, *r1 = end of input
P256_copy32_unaligned proc
	movs r2,#32
	add r2,r0
0
	ldrb r3,[r1]
	strb r3,[r0]
	adds r1,#1
	adds r0,#1
	cmp r0,r2
	bne %b0
	bx lr
	endp

; Selects one of many values
; *r0 = output, *r1 = table, r2 = index to choose [0..7]
P256_select proc
	push {r2,r4-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,24
	
	movs r6,#4
0
	push {r0,r6}
	frame address sp,32
	
	movs r7,#0
	mov r8,r7
	mov r9,r7
	mov r10,r7
	mov r11,r7
	mov r12,r7
	mov lr,r7
1
	ldr r0,[sp,#8]
	eors r0,r7
	mrs r0,apsr
	lsrs r0,#30
	
	ldm r1!,{r2-r4}
	muls r2,r0,r2
	muls r3,r0,r3
	muls r4,r0,r4
	add r8,r2
	add r9,r3
	add r10,r4
	ldm r1!,{r2-r4}
	muls r2,r0,r2
	muls r3,r0,r3
	muls r4,r0,r4
	add r11,r2
	add r12,r3
	add lr,r4
	
	adds r1,#72
	adds r7,#1
	cmp r7,#8
	bne %b1
	
	pop {r0,r6}
	frame address sp,24
	mov r2,r8
	mov r3,r9
	mov r4,r10
	stm r0!,{r2-r4}
	mov r2,r11
	mov r3,r12
	mov r4,lr
	stm r0!,{r2-r4}
	subs r1,#248
	subs r1,#248
	subs r1,#248
	subs r6,#1
	bne %b0
	
	pop {r0,r4-r7,pc}
	endp

; Doubles the point in Jacobian form (integers are in Montgomery form)
; *r0 = out, *r1 = in
P256_double_j proc
	if use_interpreter == 1
	adr r2,P256_double_j_prog
	b P256_interpreter
	else
	push {r0,r1,r4-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,28
	
	; https://eprint.iacr.org/2014/130.pdf, algorithm 10
	
	; t1 = Z1^2
	sub sp,#32
	frame address sp,60
	mov r0,sp
	adds r1,#64
	bl P256_sqrmod
	
	; Z2 = Y1 * Z1
	ldr r0,[sp,#32]
	ldr r1,[sp,#36]
	adds r0,#64
	adds r1,#32
	movs r2,#32
	adds r2,r1
	bl P256_mulmod
	
	; t2 = X1 + t1
	ldr r1,[sp,#36]
	mov r2,sp
	sub sp,#32
	frame address sp,92
	mov r0,sp
	bl P256_addmod
	
	; t1 = X1 - t1
	ldr r1,[sp,#68]
	add r2,sp,#32
	mov r0,r2
	bl P256_submod
	
	; t1 = t1 * t2
	add r1,sp,#32
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	; t2 = t1 / 2
	add sp,#32
	frame address sp,60
	mov r7,sp
	ldm r7!,{r0-r3}
	lsls r6,r0,#31
	asrs r5,r6,#31
	lsrs r6,#31
	movs r4,#0
	adds r0,r5
	adcs r1,r5
	adcs r2,r5
	adcs r3,r4
	push {r0-r3}
	frame address sp,76
	ldm r7!,{r0-r3}
	adcs r0,r4
	adcs r1,r4
	adcs r2,r6
	adcs r3,r5
	movs r4,#0
	adcs r4,r4
	lsls r7,r4,#31
	lsrs r6,r3,#1
	orrs r7,r6
	lsls r6,r3,#31
	lsrs r5,r2,#1
	orrs r6,r5
	lsls r5,r2,#31
	lsrs r4,r1,#1
	orrs r5,r4
	lsls r4,r1,#31
	lsrs r3,r0,#1
	orrs r4,r3
	lsls r3,r0,#31
	mov r8,r3
	pop {r0-r3}
	frame address sp,60
	push {r4-r7}
	frame address sp,76
	mov r7,r8
	lsrs r6,r3,#1
	orrs r7,r6
	lsls r6,r3,#31
	lsrs r5,r2,#1
	orrs r6,r5
	lsls r5,r2,#31
	lsrs r4,r1,#1
	orrs r5,r4
	lsls r4,r1,#31
	lsrs r3,r0,#1
	orrs r4,r3
	push {r4-r7}
	frame address sp,92
	
	; t1 = t1 + t2
	add r1,sp,#32
	mov r2,sp
	mov r0,r1
	bl P256_addmod
	
	; t2 = t1^2
	mov r0,sp
	add r1,sp,#32
	bl P256_sqrmod
	
	; Y2 = Y1^2
	ldr r0,[sp,#64]
	ldr r1,[sp,#68]
	adds r0,#32
	adds r1,#32
	bl P256_sqrmod
	
	; t3 = Y2^2
	ldr r1,[sp,#64]
	adds r1,#32
	sub sp,#32
	frame address sp,124
	mov r0,sp
	bl P256_sqrmod
	
	; Y2 = X1 * Y2
	ldr r0,[sp,#96]
	ldr r1,[sp,#100]
	adds r0,#32
	mov r2,r0
	bl P256_mulmod
	
	; X2 = 2 * Y2
	ldr r0,[sp,#96]
	mov r1,r0
	adds r1,#32
	mov r2,r1
	bl P256_addmod
	
	; X2 = t2 - X2
	ldr r0,[sp,#96]
	add r1,sp,#32
	mov r2,r0
	bl P256_submod
	
	; t2 = Y2 - X2
	ldr r2,[sp,#96]
	mov r1,r2
	adds r1,#32
	add r0,sp,#32
	bl P256_submod
	
	; t1 = t1 * t2
	add r0,sp,#64
	add r1,sp,#64
	add r2,sp,#32
	bl P256_mulmod
	
	; Y2 = t1 - t3
	ldr r0,[sp,#96]
	adds r0,#32
	add r1,sp,#64
	mov r2,sp
	bl P256_submod
	
	add sp,#104
	frame address sp,20
	
	pop {r4-r7,pc}
	endif
	endp


; Adds or subtracts points in Jacobian form (integers are in Montgomery form)
; The first operand is located in *r0, the second in *r1 (may not overlap)
; The result is stored at *r0
;
; Requirements:
; - no operand is the point at infinity
; - both operand must be different
; - one operand must not be the negation of the other
; If requirements are not met, the returned Z point will be 0
P256_add_j proc
	if use_interpreter == 1
	adr r2,P256_add_j_prog
	b P256_interpreter
	else
	push {r0,r1,r4-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,28
	
	; Here a variant of
	; https://www.hyperelliptic.org/EFD/g1p/auto-code/shortw/jacobian-3/addition/add-1998-cmo-2.op3
	; is used, but rearranged and uses less temporaries.
	; The first operand to the function is both (X3,Y3,Z3) and (X2,Y2,Z2).
	; The second operand to the function is (X1,Y1,Z1)
	
	; Z1Z1 = Z1^2
	sub sp,#32
	frame address sp,60
	mov r0,sp
	adds r1,#64
	bl P256_sqrmod
	
	; U2 = X2*Z1Z1
	ldr r1,[sp,#32]
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	; t1 = Z1*Z1Z1
	ldr r1,[sp,#36]
	adds r1,#64
	mov r2,sp
	mov r0,sp
	bl P256_mulmod
	
	; S2 = Y2*t1
	ldr r1,[sp,#32]
	adds r1,#32
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	; Z2Z2 = Z2^2
	sub sp,#32
	frame address sp,92
	mov r0,sp
	ldr r1,[sp,#64]
	adds r1,#64
	bl P256_sqrmod
	
	; U1 = X1*Z2Z2
	ldr r1,[sp,#68]
	mov r2,sp
	add r0,sp,#32
	bl P256_mulmod
	
	; t2 = Z2*Z2Z2
	ldr r1,[sp,#64]
	adds r1,#64
	mov r2,sp
	mov r0,sp
	bl P256_mulmod
	
	; S1 = Y1*t2
	ldr r1,[sp,#68]
	adds r1,#32
	mov r2,sp
	mov r0,sp
	bl P256_mulmod
	
	; H = U2-U1
	ldr r1,[sp,#64]
	add r2,sp,#32
	mov r0,r1
	bl P256_submod
	
	; HH = H^2
	ldr r1,[sp,#64]
	sub sp,#32
	frame address sp,124
	mov r0,sp
	bl P256_sqrmod
	
	; Z3 = Z2*H
	ldr r2,[sp,#96]
	mov r1,r2
	adds r1,#64
	mov r0,r1
	bl P256_mulmod
	
	; Z3 = Z1*Z3
	ldr r1,[sp,#100]
	adds r1,#64
	ldr r2,[sp,#96]
	adds r2,#64
	mov r0,r2
	bl P256_mulmod
	
	; HHH = H*HH
	ldr r1,[sp,#96]
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	; r = S2-S1
	ldr r1,[sp,#96]
	adds r1,#32
	add r2,sp,#32
	mov r0,r1
	bl P256_submod
	
	; V = U1*HH
	add r1,sp,#64
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	; t3 = r^2
	ldr r1,[sp,#96]
	adds r1,#32
	mov r0,sp
	bl P256_sqrmod
	
	; t2 = S1*HHH
	add r1,sp,#32
	ldr r2,[sp,#96]
	add r0,sp,#32
	bl P256_mulmod
	
	; X3 = t3-HHH
	mov r1,sp
	ldr r2,[sp,#96]
	mov r0,r2
	bl P256_submod
	
	; t3 = 2*V
	add r1,sp,#64
	add r2,sp,#64
	mov r0,sp
	bl P256_addmod
	
	; X3 = X3-t3
	ldr r1,[sp,#96]
	mov r2,sp
	mov r0,r1
	bl P256_submod
	
	; t3 = V-X3
	add r1,sp,#64
	ldr r2,[sp,#96]
	mov r0,sp
	bl P256_submod
	
	; t3 = r*t3
	ldr r1,[sp,#96]
	adds r1,#32
	mov r2,sp
	mov r0,sp
	bl P256_mulmod
	
	; Y3 = t3-t2
	mov r1,sp
	add r2,sp,#32
	ldr r0,[sp,#96]
	adds r0,#32
	bl P256_submod
	
	add sp,#104
	frame address sp,20
	
	pop {r4-r7,pc}
	endif
	endp

	if use_interpreter == 1
	align 4
P256_add_j_prog
	dcw 0x2080
	dcw 0x1330
	dcw 0x1080
	dcw 0x1440
	dcw 0x2150
	dcw 0x1061
	dcw 0x1151
	dcw 0x1171
	dcw 0x4330
	dcw 0x2230
	dcw 0x1553
	dcw 0x1585
	dcw 0x1332
	dcw 0x4441
	dcw 0x1002
	dcw 0x2240
	dcw 0x1113
	dcw 0x4323
	dcw 0x3200
	dcw 0x4332
	dcw 0x4203
	dcw 0x1242
	dcw 0x4421
	dcw 0x0000
	
	align 4
P256_double_j_prog
	dcw 0x2080
	dcw 0x1578
	dcw 0x3160
	dcw 0x4060
	dcw 0x1001
	dcw 0x5100
	dcw 0x3001
	dcw 0x2100
	dcw 0x2470
	dcw 0x2240
	dcw 0x1464
	dcw 0x3344
	dcw 0x4313
	dcw 0x4143
	dcw 0x1001
	dcw 0x4402
	dcw 0x0000

; in: *r0 = output, *r1 = input
P256_div2mod proc
	mov r9,r0
	mov r7,r1
	ldm r7!,{r0-r3}
	lsls r6,r0,#31
	asrs r5,r6,#31
	lsrs r6,#31
	movs r4,#0
	adds r0,r5
	adcs r1,r5
	adcs r2,r5
	adcs r3,r4
	push {r0-r3}
	frame address sp,76
	ldm r7!,{r0-r3}
	adcs r0,r4
	adcs r1,r4
	adcs r2,r6
	adcs r3,r5
	movs r4,#0
	adcs r4,r4
	lsls r7,r4,#31
	lsrs r6,r3,#1
	orrs r7,r6
	lsls r6,r3,#31
	lsrs r5,r2,#1
	orrs r6,r5
	lsls r5,r2,#31
	lsrs r4,r1,#1
	orrs r5,r4
	lsls r4,r1,#31
	lsrs r3,r0,#1
	orrs r4,r3
	lsls r3,r0,#31
	mov r0,r9
	adds r0,#16
	stm r0!,{r4-r7}
	mov r7,r3
	pop {r0-r3}
	lsrs r6,r3,#1
	orrs r7,r6
	lsls r6,r3,#31
	lsrs r5,r2,#1
	orrs r6,r5
	lsls r5,r2,#31
	lsrs r4,r1,#1
	orrs r5,r4
	lsls r4,r1,#31
	lsrs r3,r0,#1
	orrs r4,r3
	mov r0,r9
	stm r0!,{r4-r7}
	bx lr
	endp

; in: *r0 = op1, *r1 = op2, *r2 = program
; program is an array of 16-bit integers, ending with 0x0000
; in an opcode, bit 12-15 is function to execute (exit, mul, sqr, add, sub, div2),
; bit 8-11 is dest, bit 4-7 is first operand, bit 0-3 is second operand
; the operand is encoded like this:
; operand 0-2 is temporary variable 0-2
; operand 3-5 is op1[0], op1[1], op1[2]
; operand 6-8 is op2[0], op2[1], op2[2]
; each variable is 32 bytes
; for a function taking less than two parameters, the extra parameters are ignored
P256_interpreter proc
	push {r4-r7,lr}
	frame push {r4-r7,lr}
	
	sub sp,#96
	frame address sp,116
	
	movs r3,#32
	mov r4,r1
	adds r5,r1,r3
	adds r6,r5,r3
	push {r4-r6}
	frame address sp,128
	mov r4,r0
	adds r5,r0,r3
	adds r6,r5,r3
	push {r4-r6}
	frame address sp,140
	add r4,sp,#24
	adds r5,r4,r3
	adds r6,r5,r3
	push {r4-r6}
	frame address sp,152
	
0
	movs r4,#0x3c
	mov r5,sp
	ldrh r3,[r2]
	adds r2,#2
	push {r2}
	frame address sp,156
	lsls r2,r3,#2
	ands r2,r4
	ldr r2,[r5,r2]
	lsrs r1,r3,#2
	ands r1,r4
	ldr r1,[r5,r1]
	lsrs r0,r3,#6
	ands r0,r4
	ldr r0,[r5,r0]
	adr r5,P256_functions-4
	lsrs r6,r3,#10
	ands r6,r4
	beq %f1
	ldr r6,[r5,r6]
	blx r6
	pop {r2}
	frame address sp,152
	b %b0
1
	frame address sp,156
	add sp,#136
	frame address sp,20
	pop {r4-r7,pc}
	endp

	align 4
P256_functions
	dcd P256_mulmod  ;1
	dcd P256_sqrmod  ;2
	dcd P256_addmod  ;3
	dcd P256_submod  ;4
	dcd P256_div2mod ;5

	endif

	if use_smaller_modinv == 1
; in/out: r0-r7
P256_modinv proc
	push {r0-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,36
	sub sp,#36
	frame address sp,72
	mov r0,sp
	bl P256_load_1
	mov r1,r0
	bl P256_to_montgomery
	adr r0,P256_p
	ldm r0,{r0-r7}
	subs r0,#2
	push {r0-r7}
	frame address sp,104
	
	movs r0,#255
0
	str r0,[sp,#64]
	add r0,sp,#32
	add r1,sp,#32
	bl P256_sqrmod
	ldr r0,[sp,#64]
	lsrs r1,r0,#3
	add r1,r1,sp
	ldrb r1,[r1]
	movs r2,#7
	ands r2,r2,r0
	lsrs r1,r2
	movs r2,#1
	tst r1,r2
	beq %f1
	add r0,sp,#32
	add r1,sp,#32
	add r2,sp,#68
	bl P256_mulmod
1
	ldr r0,[sp,#64]
	subs r0,#1
	bpl %b0
	
	add sp,#32
	frame address sp,72
	pop {r0-r7}
	frame address sp,40
	add sp,#36
	frame address sp,4
	pop {pc}
	endp

	else

; in: *r0 = input/output, r1 = count, *r2 = operand for final multiplication
P256_sqrmod_many_and_mulmod proc
	push {r0,r2,lr}
	frame push {lr}
	frame address sp,12
	cmp r1,#0
	beq %f1
0
	push {r1}
	frame address sp,16
	ldr r0,[sp,#4]
	mov r1,r0
	bl P256_sqrmod
	pop {r1}
	frame address sp,12
	subs r1,#1
	bne %b0
1
	pop {r0,r1}
	frame address sp,4
	mov r2,r0
	bl P256_mulmod
	pop {pc}
	endp


; in: *r0 = value in/out
; for modinv, call input a, then if a = A * R % p, then it calculates A^-1 * R % p = (a/R)^-1 * R % p = R^2 / a % p
P256_modinv proc
	push {r0,lr}
	frame push {lr}
	frame address sp,8
	
	ldm r0,{r0-r7}
	push {r0-r7}
	frame address sp,40
	
	; t = a^2*a
	ldr r0,[sp,#32]
	movs r1,#1
	mov r2,sp
	bl P256_sqrmod_many_and_mulmod
	ldr r0,[sp,#32]
	ldm r0,{r0-r7}
	push {r0-r7}
	frame address sp,72
	
	; a4_2 = a2_0^(2^2)
	
	ldr r0,[sp,#64]
	mov r1,r0
	bl P256_sqrmod
	ldr r0,[sp,#64]
	mov r1,r0
	bl P256_sqrmod
	ldr r0,[sp,#64]
	ldm r0,{r0-r7}
	push {r0-r7}
	frame address sp,104
	
	; a4_0 = a4_2*a2_0
	ldr r0,[sp,#96]
	mov r1,sp
	add r2,sp,#32
	bl P256_mulmod
	add r0,sp,#32
	ldr r1,[sp,#96]
	bl P256_copy32
	
	ldr r7,[sp,#96]
	movs r4,#0
0
	adr r2,P256_invtbl
	ldrsb r0,[r2,r4]
	adds r2,#1
	ldrb r5,[r2,r4]
	lsls r6,r0,#2
	bpl %f1
	sub sp,#32
	frame address sp,200 ; not always correct
	mov r0,sp
	mov r1,r7
	bl P256_copy32
1
	mov r0,r7
	uxtb r1,r6
	mov r2,r5
	add r2,sp
	push {r4,r7}
	frame address sp,208 ; not always correct
	bl P256_sqrmod_many_and_mulmod
	pop {r4,r7}
	frame address sp,200 ; not always correct
	adds r4,#2
	cmp r4,#22
	bne %b0

	add sp,#6*32+4
	frame address sp,4
	
	pop {pc}
	
	endp

	align 4
P256_invtbl
	dcb ((8-4)>>2)
	dcb 32
	
	dcb ((16-8)>>2)+128
	dcb 0
	
	dcb (16>>2)+128
	dcb 0
	
	dcb (32>>2)+128
	dcb 5*32
	
	dcb (192-64)>>2
	dcb 0
	
	dcb (224-192)>>2
	dcb 0
	
	dcb (240-224)>>2
	dcb 32
	
	dcb (248-240)>>2
	dcb 64
	
	dcb (252-248)>>2
	dcb 128
	
	dcb (256-252)>>2
	dcb 96
	
	dcb 0
	dcb 5*32
	
	endif


; *r0 = output affine montgomery/input jacobian montgomery
P256_jacobian_to_affine proc
	push {r0,r4-r7,lr}
	frame push {r4-r7,lr}
	frame address sp,24
	
	adds r0,#64
	ldm r0,{r0-r7}
	if use_smaller_modinv == 0
		push {r0-r7}
		frame address sp,56
		mov r0,sp
		bl P256_modinv
	else
		bl P256_modinv
		push {r0-r7}
		frame address sp,56
	endif
	
	mov r1,sp
	sub sp,#32
	frame address sp,88
	mov r0,sp
	bl P256_sqrmod
	
	add r1,sp,#32
	mov r2,sp
	mov r0,r1
	bl P256_mulmod
	
	mov r1,sp
	ldr r0,[sp,#64]
	mov r2,r0
	bl P256_mulmod
	
	add r1,sp,#32
	ldr r0,[sp,#64]
	adds r0,#32
	mov r2,r0
	bl P256_mulmod
	
	add sp,#68
	frame address sp,20
	
	pop {r4-r7,pc}
	endp

; performs r0 := abs(r0)
P256_abs_int proc
	rsbs r2,r0,#0
	asrs r3,r0,#31
	ands r3,r2
	asrs r2,#31
	ands r0,r2
	orrs r0,r0,r3
	bx lr
	endp

; in: *r0 = output, *r1 = point, *r2 = scalar, r3 = include y in result (1/0)
; out: r0 = 1 on success, 0 if invalid point or scalar
P256_pointmult proc
	export P256_pointmult
	push {r4-r7,lr}
	frame push {r4-r7,lr}
	mov r4,r8
	mov r5,r9
	mov r6,r10
	mov r7,r11
	push {r0-r1,r4-r7}
	frame address sp,44
	frame save {r8-r11},-36
	sub sp,#256
	frame address sp,300
	
	lsls r6,r3,#16
	
	; load scalar into an aligned position
	add r0,sp,#32
	mov r1,r2
	bl P256_copy32_unaligned
	
	; fail if scalar == 0
	mov r0,sp
	bl P256_load_1
	add r0,sp,#32
	mov r1,sp
	bl P256_greater_or_equal_than
	bne %f1
0
	add sp,#256+8
	frame address sp,36
	b %f10
	frame address sp,300
1
	; fail if not (scalar < n)
	add r0,sp,#32
	adr r1,P256_order
	bl P256_greater_or_equal_than
	subs r0,#1
	beq %b0
	
	; select scalar if scalar is odd and -scalar mod n if scalar is even
	mov r0,sp
	add r1,sp,#32
	ldr r2,[r1]
	movs r3,#1
	ands r2,r3
	eors r2,r3
	add r6,r2 ; save original parity of scalar
	adr r3,P256_order
	bl P256_negate_mod_m_if
	
	; stack layout (initially offset 768):
	; 0-767: table of jacobian points P, 3P, 5P, ..., 15P
	; 768-863: current point (in jacobian form)
	; 864-927: scalar rewritten into 4-bit window, each element having an odd signed value
	; 928-1023: extracted selected point from the table
	; 1024-1027: output pointer
	; 1028-1031: input point
	
	; rewrite scalar into 4-bit window where every value is odd
	add r1,sp,#864-768
	ldr r0,[sp]
	lsls r0,#28
	lsrs r0,#28
	movs r2,#1
	mov r4,sp
	movs r5,#1
2
	lsrs r3,r2,#1
	ldrb r3,[r4,r3]
	lsls r7,r2,#31
	lsrs r7,#29
	lsrs r3,r7
	lsls r3,#28
	lsrs r3,#28
	movs r7,#1
	ands r7,r3
	eors r7,r5
	lsls r7,#4
	subs r0,r7
	strb r0,[r1]
	adds r1,#1
	orrs r3,r5
	mov r0,r3
	adds r2,#1
	cmp r2,#64
	bne %b2
	strb r0,[r1]
	
	; load point into an aligned position
	ldr r1,[sp,#1028-768]
	sub sp,#384
	frame address sp,684
	sub sp,#384
	frame address sp,1068
	mov r0,sp
	bl P256_copy32_unaligned
	bl P256_copy32_unaligned
	
	; fail if not x, y < p
	mov r0,sp
	adr r1,P256_p
	bl P256_greater_or_equal_than
	subs r0,#1
	bne %f4
3
	add sp,#384
	frame address sp,684
	add sp,#384
	frame address sp,300
	b %b0
	frame address sp,1068
4
	add r0,sp,#32
	adr r1,P256_p
	bl P256_greater_or_equal_than
	subs r0,#1
	beq %b3
	
	; convert basepoint x, y to montgomery form,
	; and place result as first element in table of Jacobian points
	
	mov r0,sp
	mov r1,sp
	bl P256_to_montgomery
	add r0,sp,#32
	add r1,sp,#32
	bl P256_to_montgomery
	
	; check that the basepoint lies on the curve
	mov r0,sp
	bl P256_point_is_on_curve
	cmp r0,#0
	beq %b3
	
	; load montgomery 1 for Z
	add r0,sp,#64
	bl P256_load_1
	mov r1,r0
	bl P256_to_montgomery
	
	; temporarily calculate 2P
	add r0,sp,#7*96
	mov r1,sp
	bl P256_double_j
	
	; calculate rest of the table (3P, 5P, ..., 15P)
	add r4,sp,#96
	movs r5,#7
5
	mov r0,r4
	add r1,sp,#7*96
	bl P256_copy32
	bl P256_copy32
	bl P256_copy32
	mov r0,r4
	mov r1,r0
	subs r1,#96
	bl P256_add_j
	adds r4,#96
	subs r5,#1
	bne %b5
	
	; select the initial current point based on the first highest 4 scalar bits
	add r7,sp,#928
	subs r7,#1
	ldrb r0,[r7]
	subs r7,#1
	sxtb r0,r0
	bl P256_abs_int
	lsrs r2,r0,#1
	add r0,sp,#768
	mov r1,sp
	bl P256_select
	
	; main loop iterating from index 62 to 0 of the windowed scalar
	add r5,sp,#864
6
	movs r4,#4
7
	add r0,sp,#768
	mov r1,r0
	bl P256_double_j
	subs r4,#1
	bne %b7
	
	; select the point to add, and then add to the current point
	ldrb r0,[r7]
	subs r7,#1
	sxtb r0,r0
	lsrs r4,r0,#31
	bl P256_abs_int
	lsrs r2,r0,#1
	add r0,sp,#928
	mov r1,sp
	bl P256_select
	add r0,sp,#960
	mov r1,r0
	mov r2,r4
	adr r3,P256_p
	bl P256_negate_mod_m_if
	cmp r7,r5
	bge %f8
	; see note below
	add r0,sp,#672
	add r1,sp,#768
	bl P256_double_j
8
	add r0,sp,#768
	add r1,sp,#928
	bl P256_add_j
	cmp r7,r5
	bge %b6
	
	; Note: ONLY for the scalars 2 and -2 mod n, the last addition will
	; be an addition where both input values are equal. The addition algorithm
	; fails for such a case (returns Z=0) and we must therefore use the doubling
	; formula. Both values are computed and then the correct value is selected
	; in constant time based on whether the addition formula returned Z=0.
	; Obviously if the scalar (private key) is properly randomized, this would
	; (with extremely high probability), never occur.
	mov r0,sp
	bl P256_load_1
	add r0,sp,#768+64
	mov r1,sp
	bl P256_greater_or_equal_than
	adds r2,r0,#6
	add r0,sp,#928
	add r1,sp,#96
	bl P256_select
	
	add sp,#464 ;928/2
	frame address sp,604
	add sp,#464
	frame address sp,140
	
	mov r0,sp
	bl P256_jacobian_to_affine
	
	mov r0,sp
	mov r1,sp
	bl P256_from_montgomery
	add r0,sp,#32
	add r1,sp,#32
	bl P256_from_montgomery
	
	add r0,sp,#32
	add r1,sp,#32
	uxtb r2,r6
	adr r3,P256_p
	bl P256_negate_mod_m_if
	
	ldr r0,[sp,#96]
	mov r1,sp
	bl P256_copy32_unaligned
	lsrs r6,#16
	beq %f9
	bl P256_copy32_unaligned
9
	
	movs r0,#1
	add sp,#96+8
	frame address sp,36
10
	pop {r4-r7}
	frame address sp,20
	mov r8,r4
	mov r9,r5
	mov r10,r6
	mov r11,r7
	pop {r4-r7,pc}
	endp

; in: *r0 = output, *r1 = private key scalar
; out: r0 = 1 on success, 0 if scalar is out of range
P256_ecdh_keygen proc
	export P256_ecdh_keygen
	mov r2,r1
	adr r1,P256_basepoint
	movs r3,#1
	b P256_pointmult
	endp

; in: *r0 = output, *r1 = other's public point, *r2 = private key scalar
; out: r0 = 1 on success, 0 if invalid public point or private key scalar
P256_ecdh_shared_secret proc
	export P256_ecdh_shared_secret
	movs r3,#0
	b P256_pointmult
	endp

	align 4
P256_p
	dcd 0xffffffff
	dcd 0xffffffff
	dcd 0xffffffff
	dcd 0
	dcd 0
	dcd 0
	dcd 1
	dcd 0xffffffff

P256_order
	dcd 0xFC632551
	dcd 0xF3B9CAC2
	dcd 0xA7179E84
	dcd 0xBCE6FAAD
	dcd 0xFFFFFFFF
	dcd 0xFFFFFFFF
	dcd 0
	dcd 0xFFFFFFFF

P256_basepoint
	dcd 0xD898C296
	dcd 0xF4A13945
	dcd 0x2DEB33A0
	dcd 0x77037D81
	dcd 0x63A440F2
	dcd 0xF8BCE6E5
	dcd 0xE12C4247
	dcd 0x6B17D1F2
	dcd 0x37BF51F5
	dcd 0xCBB64068
	dcd 0x6B315ECE
	dcd 0x2BCE3357
	dcd 0x7C0F9E16
	dcd 0x8EE7EB4A
	dcd 0xFE1A7F9B
	dcd 0x4FE342E2
	
	end
