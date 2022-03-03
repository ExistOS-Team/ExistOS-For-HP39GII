




















































	.text
	.align	3
	.globl	__gmpn_udiv_qrnnd 
	.type	__gmpn_udiv_qrnnd,#function
__gmpn_udiv_qrnnd:
	mov	r12, #8			
	cmp	r3, #0x80000000		
	bcs	.L_large_divisor

.Loop:	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	sub	r12, r12, #1
	teq	r12, #0
	bne	.Loop

	str	r1, [r0]		
	adc	r0, r2, r2		
	mov	r15, 	r14

.L_large_divisor:
	stmfd	r13!, { r8, r14 }

	and	r8, r2, #1		
	mov	r14, r1, lsl #31
	orrs	r2, r14, r2, lsr #1	
	mov	r1, r1, lsr #1		

	and	r14, r3, #1		
	movs	r3, r3, lsr #1		
	adc	r3, r3, #0		

.Loop2:
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	adcs	r2, r2, r2
	adc	r1, r1, r1
	cmp	r1, r3
	subcs	r1, r1, r3
	sub	r12, r12, #1
	teq	r12, #0
	bne	.Loop2

	adc	r2, r2, r2		
	add	r1, r8, r1, lsl #1	
	tst	r14, r14			
	beq	.L_even_divisor

	rsb	r3, r14, r3, lsl #1	
	adds	r1, r1, r2		
	addcs	r2, r2, #1		
	subcs	r1, r1, r3		
	cmp	r1, r3			
	subcs	r1, r1, r3		
	addcs	r2, r2, #1		

.L_even_divisor:
	str	r1, [r0]		
	mov	r0, r2			
	ldmfd	r13!, { r8, r15 }

	.size	__gmpn_udiv_qrnnd,.-__gmpn_udiv_qrnnd
