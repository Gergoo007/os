.global memset
.global memcpy
.global memset_aligned
.global memcpy_aligned

# %rdi: cím
# %sl: érték
# %rdx: ennyit
memset:
	mov %rdx, %rcx
	mov %sil, %al
	rep stosb
	ret

# %rdi: innen
# %rsi: amoda
# %rdx: ennyit
memcpy:
	xchg %rdi, %rsi
	mov %rdx, %rcx
	rep movsb
	ret

#
##
##### TODO: AVX, SSE
##
#

# %rdi: 64 byte-os határra illeszkedő cím
# %sl: érték
# %rdx: ennyit (8 byte többszörös)
memset_aligned:
	mov %rdx, %rcx
	mov %rsi, %rax
	rep stosq
	ret

# %rdi: 64 byte-os határra illeszkedő innen
# %rsi: 64 byte-os határra illeszkedő amoda
# %rdx: ennyit (8 byte többszörös)
memcpy_aligned:
	xchg %rdi, %rsi
	mov %rdx, %rcx
	rep movsq
	ret
