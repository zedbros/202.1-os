.global _start
_start:
  mov x1, #12 // value of fib(x12)
  bl _fib
  mov x0, x3 // sets the value of x0 to the final fib number which is stored in x2
  mov w8, #93
  svc #0

_fib:
  mov x7, lr // stores the address value to return line 5

  mov x2, #0 // x0 in classic fib
  mov x3, #1 // x1 in classic fib
  mov x4, #0 // temporary

  bl loop

  loop:
    cmp x1, #2 // sees if the counter is less than 2 aka. = to 1
    blt finished
    mov x4, x3
    add x3, x2, x3
    mov x2, x4
    sub x1, x1, #1
    ret

  finished:
    ret x7 // return to lr which is line 5
