.text
.global _start
_start:
  sub x1, sp, #256
_l0:
	mov w8, #63
	mov x0, #0
	mov x2, #256
	svc #0
	mov w8, #64
	mov x2, x0
	mov x0, #1
	svc #0
	b   _l0
