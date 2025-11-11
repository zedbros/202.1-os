import sys
import ctypes
import os

libc = ctypes.CDLL(None)

def sayIt(msg):
    b = ctypes.ARRAY(ctypes.c_char, len(msg))(*(ord(m) for m in msg))
    f = libc.syscall
    f.argtypes = (
        ctypes.c_long,
        ctypes.c_long,
        ctypes.POINTER(ctypes.c_char),
        ctypes.c_long)
    f(64, 1, b, len(msg))

sayIt(sys.argv[1] + "\n")
