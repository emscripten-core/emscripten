.global fegetround
.type fegetround,%function
fegetround:
	mrc p10, 7, r0, cr1, cr0, 0
	and r0, r0, #0xc00000
	bx lr

.global __fesetround
.type __fesetround,%function
__fesetround:
	mrc p10, 7, r3, cr1, cr0, 0
	bic r3, r3, #0xc00000
	orr r3, r3, r0
	mcr p10, 7, r3, cr1, cr0, 0
	mov r0, #0
	bx lr

.global fetestexcept
.type fetestexcept,%function
fetestexcept:
	and r0, r0, #0x1f
	mrc p10, 7, r3, cr1, cr0, 0
	and r0, r0, r3
	bx lr

.global feclearexcept
.type feclearexcept,%function
feclearexcept:
	and r0, r0, #0x1f
	mrc p10, 7, r3, cr1, cr0, 0
	bic r3, r3, r0
	mcr p10, 7, r3, cr1, cr0, 0
	mov r0, #0
	bx lr

.global feraiseexcept
.type feraiseexcept,%function
feraiseexcept:
	and r0, r0, #0x1f
	mrc p10, 7, r3, cr1, cr0, 0
	orr r3, r3, r0
	mcr p10, 7, r3, cr1, cr0, 0
	mov r0, #0
	bx lr

.global fegetenv
.type fegetenv,%function
fegetenv:
	mrc p10, 7, r3, cr1, cr0, 0
	str r3, [r0]
	mov r0, #0
	bx lr

.global fesetenv
.type fesetenv,%function
fesetenv:
	mrc p10, 7, r3, cr1, cr0, 0
	cmn r0, #1
	moveq r3, #0
	ldrne r3, [r0]
	mcr p10, 7, r3, cr1, cr0, 0
	mov r0, #0
	bx lr
