.data
  // A 4-bytes buffer.
  buffer:
    .quad 0 // a word, or 4-bytes, should be enough for the 32 bits number.. but if
            // we have to take the '\n' char, then might need to add one extra byte.

            // ACTUALLY you need 4 bits per number we want to print plus the '\n'.
            // This means i need a bigger buffer => 40bits + '\n' => 5bytes + '\n'.
            // So have to use .quad 0
.text

//**************************************************************************
//**************************NOTE TO PROFFESOR(S)****************************
//**************************************************************************
// I unfortunatly took too much time with the str_to_nat.s section and did
// not manage to finish in time.
// I however, dug as deep as possible in it and am providing you the annotated
// file as proof of my research in hops that this will prove some level of
// dedication despite the not completing the inital given task.
// Sorry in advance James Zeiger.
//**************************************************************************
//**************************NOTE TO PROFFESOR(S)****************************
//**************************************************************************

.global _start
_start:
  // Prints 42.
  //mov   w0, 65538 // Reached a maximum of 65'537 --> exactly 2^16 so I have to change w1 currently=15 and w3 currently=16
                 // to 2^32 => w1=31 and w3=32. Still get "Error: immediate cannot be moved by a single instruction"
                // AHHH !! Because mov is capped at 65537 hahaha
               // alternative: using ldr exactly like the x3 buffer
  ldr w0, =4294967295 // oh you need an equal sign before
  bl    _print_nat

  //**********************My start modifications************************
  //cmp x0, #4294967295 // out of imm range but will see how to fix this later. => probably load 4 bytes 111... to x5.
  // implement ldr x5 just after the ldr w0 ******this was THE FINAL COMMENT 23:47 28/09/2025*******
  ldr x5, =4294967295
  cmp x0, x5
  blt all_good
  // Was testing to modify x0, to see if it was here that the print function depended on //add x0, x0, #3 => no.
  mov   w0, #1 // Exit with status 1.
  mov   w8, #93
  svc   #0
  //********************************************************************
  all_good:
    // Exit with status 0.
    mov   w0, #0
    mov   w8, #93
    svc   #0

/// Prints the natural number stored in `x0` followed by a newline.
_print_nat:
  // Writes `\n` at the end of the buffer.
  ldr   x3, =buffer     // Loads the buffer to the x3 register.
  mov   w4, #10        // This is the '\n' character..
  strb  w4, [x3, #31] // changed from 3 to 4 so that it is at the end of th buffer => has to be equal to w1 below.
                     // so it is here that the '\n' is inserted to the end of the x3 buffer, with an offset of 3.
                    // I think this fills the buffer upwards of 3 as such: x3 = (3,2,1,0) with the numbers
                   // representing the 4 bytes of 32 bits. When filled with '\n' then the 3rd byte 

  // Is `x0` equal to 0?
  cbz   w0, _print_nat_0

  // Otherwise, write each digit from right to left in a buffer, using `w1` to track the index of
  // the leftmost digit written in the buffer referred to by `x3`.
  mov   w1, #31 // Was 3. DAAAAMN it really shouldn't be 3 with this size of new buffer
  mov   w2, #10
_print_nat_head:

  // Are we done? 
  // aka. have we cleared all the digits available in the given number. 1234 -> 0000.000001234 otherwise infinite.
  cbz   w0, _print_nat_n

  // Otherwise, write the next digit in the buffer.
  // Basically one giant loop that eats the number from the right to the left and isolating the digit on the
  // right and then stores the crunched number as an ASCII character for the next iteration util we get to 
  // the last digit which will be 0.
  udiv  w4, w0, w2        // w4 = w0 / w2 => divides by 10 to get the next digit
  msub  w5, w4, w2, w0   // w5 = w0 % w2 => modulos it to get the last digit on it's own (value = [0-9])
  mov   w0, w4          // Stores the full number with the previous digit gone from the past loop in w0 (for the cbz check too)
  add   w5, w5, #48    // idk.. the offset for the buffer ?? no that's x1.. so idk. OHHHHH CONVERTS INT TO ASCII !!
  sub   w1, w1, #1    // Counter-increments the index tracker
  strb  w5, [x3, x1] // Adds the (XdigitX) ASCII Char to the x3 buffer. Overall starts from the right of the buffer
                    // with the weakest digit first then up : 1234 so 4 then 3 then 2 then 1. ('\n' is already behind 4)
  b     _print_nat_head // loops back till done.

_print_nat_0:
  mov   w4, #48
  strb  w4, [x3, #6]
  mov   w1, #6

_print_nat_n:
  // Print the contents of the buffer referred to by `x3` starting from `w2`.
  add   x4, x3, x1
  mov   w3, #32 // Was 4.. I believe this determines how many cycles this has to go through aka. how many symbols to print.
               // From what I observed with the '\n', this value has to be exactly one more than the w1 in _print_nat.
  sub   w2, w3, w1

  mov   x1, x4
  mov   x0, #1

  // Exit functions
  mov   w8, #64
  svc   #0
  ret
