	.file	"benchmark.c"
	.section	.rodata
.LC0:
	.string	"world"
.LC1:
	.string	"hello"
.LC2:
	.string	"helloworld"
.LC3:
	.string	"benchmark.c"
	.align 8
.LC4:
	.string	"strcmp(\"helloworld\",dest) == 0"
	.align 8
.LC6:
	.string	"Executed concat 10000 times in %f ms\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$56, %rsp
	.cfi_offset 3, -24
	movq	%fs:40, %rax
	movq	%rax, -24(%rbp)
	xorl	%eax, %eax
	call	clock
	movq	%rax, -56(%rbp)
	movl	$0, -60(%rbp)
	jmp	.L2
.L3:
	leaq	-48(%rbp), %rax
	movq	%rax, %rdx
	movl	$.LC0, %esi
	movl	$.LC1, %edi
	call	concat
	addl	$1, -60(%rbp)
.L2:
	cmpl	$9999, -60(%rbp)
	jle	.L3
	leaq	-48(%rbp), %rax
	movq	%rax, %rsi
	movl	$.LC2, %edi
	call	strcmp
	testl	%eax, %eax
	je	.L4
	movl	$__PRETTY_FUNCTION__.2489, %ecx
	movl	$14, %edx
	movl	$.LC3, %esi
	movl	$.LC4, %edi
	call	__assert_fail
.L4:
	movq	-56(%rbp), %rax
	imulq	$1000, %rax, %rax
	cvtsi2ssq	%rax, %xmm0
	movss	.LC5(%rip), %xmm1
	divss	%xmm1, %xmm0
	unpcklps	%xmm0, %xmm0
	cvtps2pd	%xmm0, %xmm0
	movl	$.LC6, %edi
	movl	$1, %eax
	call	printf
	movl	$0, %eax
	movq	-24(%rbp), %rbx
	xorq	%fs:40, %rbx
	je	.L6
	call	__stack_chk_fail
.L6:
	addq	$56, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.rodata
	.type	__PRETTY_FUNCTION__.2489, @object
	.size	__PRETTY_FUNCTION__.2489, 5
__PRETTY_FUNCTION__.2489:
	.string	"main"
	.align 4
.LC5:
	.long	1232348160
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
