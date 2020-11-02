	global crypto_blockfunc
	section .text

crypto_blockfunc:
	mov	eax, edi ; IV  = IV0
	xor	edx, edx ; Zero register
%rep 37
	mov     ecx, esi ; T = KEY
	ror     eax, 1   ; IV = ROR( IV, 1 )
	cmovnc  ecx, edx ; if ( IV.NC ) T = 0
	xor     eax, ecx ; IV ^= T
%endrep
	xor	eax, edi ; IV ^= IV0
	ret
