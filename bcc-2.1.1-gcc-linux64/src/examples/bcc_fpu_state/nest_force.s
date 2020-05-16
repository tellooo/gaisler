	.section	".text"
	.global		force_with_float

force_with_float:
	save	%sp, -(96 + 8 + 8), %sp
	/* Save %f8, %f9, since we must preserve them */
	std	%f8, [%sp + 96 + 8]
	nop

	std	%i0, [%sp + 96]
	ldd	[%sp + 96], %f8
	/* We do not expect %f8 to be trashed when we do 'call' */
	mov	%i2, %o0
	call	bcc_int_force
	 nop
	fmovs	%f8, %f0
	fmovs	%f9, %f1

	/* Restore %f8, %f9 */
	ldd	[%sp + 96 + 8], %f8
	ret
	restore

