import sys
import ctypes
import os
libc = ctypes.CDLL(None)

def cat():
    b = ctypes.ARRAY(ctypes.c_char, 256)() #256 c'est arbitraire.. ca devrait etre assez
    f = libc.syscall
    f.argtypes = (
        ctypes.c_long,
        ctypes.c_long,
        ctypes.POINTER(ctypes.c_char),
        ctypes.c_long)

    while True:
        n = f(63, 0, b, 256)
        if n == 0:
            return
        f(64, 1, b, n)

# The try catch to avoid the keyboard interupt message in the console when you press ctrl+c
# in order to exit the function.
try:
    cat()
except:
    pass