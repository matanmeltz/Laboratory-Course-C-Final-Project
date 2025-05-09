
;be ready for a lot of macros

macr theFirst
	cmp S3, *r4
	bne SHEEP
endmacr


	add *r2, *r2
    	sub *r0, *r6
    	
	theFirst
	theSecond
mouse: .data 75
	not mouse
S3:	dec *r0
	theSecond
.extern Mtv

macr theSecond
	cmp #80 , #-34
	lea Uvt , Mtv
	rts
endmacr

DOG: .extern SHEEP
.extern Uvt
; and now
MEOW: .entry S3

