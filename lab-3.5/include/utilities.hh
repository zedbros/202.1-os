#pragma once

#include <cstddef>
#include <unistd.h>

template<typename I, typename O, typename F>
I copy_while(I first, I last, O target, F&& predicate) {
  while ((first != last) && predicate(first)) {
    *(last++) = *(first++);
  }
  return first;
}

/// Writes the null-terminated string `cstr` to the file descriptor `fd`.
void write_cstr(int fd, char const* cstr) {
  constexpr std::size_t n = 64;
  char b[n];
  auto i = 0, j = 0;
  while (cstr[j] != 0) {
    if (i == n) {
      write(fd, b, n);
      i = 0;
    }
    b[i++] = cstr[j++];
  }
  write(fd, b, i);
}

/// Writes the null-terminated string `cstr` concatenated with `terminator` to the standard output.
///
/// This function is safer to use in a signal handler than iostream (or `printf`) because it does
/// not allocate any memory. Hence, it is guaranteed not to re-enter `malloc` (or another dynamic
/// allocation routine) if the signal being handled has been raised by `malloc`.
///
/// The null terminator is not written to the standard output.
void print_cstr(char const* cstr, char const* terminator = "\n") {
  write_cstr(1, cstr);
  write_cstr(1, terminator);
}
