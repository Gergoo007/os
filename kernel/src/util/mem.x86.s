.global memset
.global memcpy
.global memcmp
.global memcpy_avx_aligned

# %rdi: cím
# %sl: érték
# %rdx: ennyit
memset:
	mov %rdx, %rcx
	mov %sil, %al
	rep stosb
	ret

# %rdi: amoda
# %rsi: innen
# %rdx: ennyit
// memcpy:
// 	push %rdx
// 	# Kezdés csak egy pár byte-al,
// 	# hogy 8 byte-ra legyen igazítva legalább a source
// 	# Aztán jön a 64 bit movesq
// 	# Aztán a maradék pár byte
// 	## %rax: ennyi byte-al kezdünk
// 	mov %rsi, %rax
// 	and $0b111, %rax
// 	mov %rax, %rcx
// 	sub %rcx, (%rsp) # Ennyi byte-al kevesebbet kell elintézni
// 	mov %rcx, %r8
// 	rep movsb
	
// 	add %r8, %rdi
// 	add %r8, %rsi

// 	# 64 bit move
// 	mov (%rsp), %rcx
// 	mov %rcx, %rax
// 	and $0b111, %rax
// 	# Mostantól %rax tartalmazza a maradékot
// 	mov %rcx, %r8
// 	shl $3, %rcx # count /= 8;
// 	rep movsq

// 	add %r8, %rdi
// 	add %r8, %rsi

// 	# Maradék másolása
// 	mov %rax, %rcx
// 	rep movsb

// 	pop %rdx
// 	ret

# %rdi: lhs
# %rsi: rhs
# %rdx: ennyit
memcmp:
	mov %rdx, %rcx
	repz cmpsb
	jz .egyezik
	jg .nagyobb
	movl $0xffffffff, %eax
	ret
.egyezik:
	xor %eax, %eax
	ret
.nagyobb:
	movl $1, %eax
	ret

# 32 byte-onként mozgat, hasznos write combine memcpy-ra
# %rdi: ide
# %rsi: innen
# %rdx: ennyit
# return: maradék byte-ok
memcpy_avx_aligned:
	mov %rdx, %rax
	and $31, %rax
	shl $5, %rdx

.avxloop:
	cmp $0, %rdx
	je .avxexit
	movaps (%rsi), %xmm0
	movaps %xmm0, (%rdi)
	dec %rdx

.avxexit:
	ret
