.global __setjmp
.global _setjmp
.global setjmp
.type   __setjmp, @function
.type   _setjmp,  @function
.type   setjmp,   @function
__setjmp:
_setjmp:
setjmp:
	add   #36, r4
	sts.l  pr,   @-r4
	mov.l  r15   @-r4
	mov.l  r14,  @-r4
	mov.l  r13,  @-r4
	mov.l  r12,  @-r4
	mov.l  r11,  @-r4
	mov.l  r10,  @-r4
	mov.l  r9,   @-r4
	mov.l  r8,   @-r4
	rts
	 mov  #0, r0
