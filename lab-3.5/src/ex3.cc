#include "utilities.hh"

#include <iostream>

#include <sys/mman.h>
#include <sys/wait.h>

int main() {
  // Allocate memory that will be visible to child processes.
  void* shared = mmap(0, 2048, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  auto* x = new(shared) int{1};

  // Allocate memory that is *not* visible to child processes.
  void* copied = mmap(0, 2048, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  auto* y = new(copied) int{1};

  // Creates a clone of the current process.
  auto p = fork();

  // We're the child if `p` is equal to 0.
  if (p == 0) {
    // Only the first update is shared. The other is invisible to the parent.
    *x += 1;
    *y += 1;

    std::cout << "Child sees  ";
    std::cout << "(x: " << *x << ", y: " << *y << ")" << std::endl;
  }

  // Otherwise the child's PID has been written to `p`.
  else {
    // Wait for the child to terminate.
    int s = 0;
    waitpid(p, &s, 0);

    std::cout << "Parent sees ";
    std::cout << "(x: " << *x << ", y: " << *y << ")" << std::endl;
  }

  return 0;
}
