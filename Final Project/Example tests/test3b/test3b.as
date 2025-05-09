
.entry MOMO
.extern POPO
macr FirstMacro
	lea STR, r6
	inc r6
	mov *r6, POPO
	sub r1, r4
	cmp r3, #-6
endmacr

macr SecondtMacro
	lea LIST, r2
	inc r4
	mov *r5, POPO
endmacr

macr FirstMacro
	lea STR, STR
	clr MOMO
	sub r1, r4
	bne END
endmacr

MAIN: add r3, LIST
	  FirstMacro
	  SecondtMacro
	  bne END
	  add r7, *r6
	  clr MOMO
	  sub L3, POPO
.entry MAIN
END:  stop
STR:  .string "abcd"
LIST: .data 6, -9
	  .data -100
MOMO: .data 31
.extern L3

