.global _start
_start:
  
    cmp w0, #2
    blt tail
    lsl w0, w0, #1
  tail:
    add w0, w0, #6


  mov w8, #93
  svc #0
