; Start of the assembly code

MAIN: 	add r3, LIST
LOOP:prn #48
; Macro declaration
		macr m_macr
			cmp r3, ##-6
			bne END
		endmacr

		prn #8200
		lea STR
		inc r6, r1
		mov K, #15
E TOTO:	sub r1, r4
		m_macr
		.entry POPO
		.extern POPO
		.extern TOTO
TOTO:	dec K
		jmp LIST
3END: 	stop
STR: 	.string "abcd"
STR:	.string "str2"
STRING: .string "string
LIST:	.data 6, -9
		.data -100, , 307
LIST2:	.data 3.5, 75, 93.3
LIST3:	.data 77, 66000
POPO:  .data 5, 7, 5,
K:
cmp:	cmp r3, POPO

End-of-the-assembly-code
