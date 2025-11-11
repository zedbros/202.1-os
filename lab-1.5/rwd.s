.data
  g0:
    .quad 0
    .quad 0
  g1:
    .ascii "file not found\n"
  g2:
    .ascii "usage: rwd file\n"
.text
.global _start
_start:
  ldp		x0, x1, [sp], #16
  cmp 	w0, #1
  ble   _l7
  lsl   w2, w0, #3
  and   w3, w2, #15
  mov   w4, #16
  sub   w5, w4, w3
  add   w6, w2, w5
  sub   sp, sp, w6
  mov   x2, sp
  ldr   x3, =g0
  stp   x0, x2, [x3]
  mov   x7, x0
_l0:
  cbz   x0, _l2
  sub   x0, x0, #1
  str   x1, [x2], #8
  mov   w4, #0
_l1:
  ldrb	w3, [x1]
	add   x1, x1, #1
	cbz   x3, _l0
  b     _l1
_l2:
  sub   x0, x7, #1
  bl    _la
  ldr   x1, =g0
  ldp   x1, x2, [x1]
  lsl   x3, x0, #3
  add   x3, x3, #8
  ldr   x1, [x2, x3]
  mov   x0, #-100
  mov   x2, #0
  mov   w8, #56
  svc   #0
  cmn   x0, #0
  blt   _l6
  mov   x3, x0
  sub   sp, sp, #240
  mov   x1, sp
	mov   x2, #240
  mov   w8, #63
  svc   #0
  mov   x4, x0
  mov   x0, x3
  mov   w8, #57
  svc   #0
  mov   x0, x4
  cbz   x0, _l5
  add   x24, x0, #1
  bl    _la
  add   x7, sp, #240
  add   x6, sp, x0
  add   x5, sp, x24
  mov   x4, #15
_l3:
  cbz   x4, _l4
  ldrb  w1, [x6], #1
  strb  w1, [x7], #1
  sub   x4, x4, #1
  cmp   x6, x5
  blt   _l3
  mov   x6, sp
  b    _l3
_l4:
  mov   x3, #10
  strb  w3, [sp, #255]
  mov   x0, #1
  add   x1, sp, #240
  mov   x2, #16
  mov   w8, #64
  svc   #0
  add   sp, sp, 256
_l5:
  mov   x0, #0
  b     _l9
_l6:
  mov   w8, #64
  mov   x0, #1
  ldr   x1, =g1
  mov   x2, #15
  svc   #0
  b     _l8
_l7:
  mov   w8, #64
  mov   x0, #1
  ldr   x1, =g2
  mov   x2, #16
  svc   #0
_l8:
  mov   x0, #1
_l9:
  mov 	w8, #93
	svc 	#0
_la:
  stp   x29, x30, [sp, #-16]!
  mov   x29, sp
  stp   x0, x0, [sp, #-16]!
  mov   x0, sp
  mov   x1, #8
  mov   x2, #2
  mov   w8, #278
  svc   #0
  ldp   x0, x1, [sp], #16
  udiv  x2, x0, x1
	msub  x0, x2, x1, x0
  ldp   x29, x30, [sp], #16
  ret
