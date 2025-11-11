#include <iostream>

#include <sys/wait.h>
#include <unistd.h>

int main() {
  // Creates a clone of the current process.
  auto p = fork();

  // We're the child if `p` is equal to 0.
  if (p == 0) {
    std::cout << "I am the child of " << getppid() << std::endl;
  }

  // Otherwise the child's PID has been written to `p`.
  else {
    // Wait for the child to terminate.
    int s = 0;
    waitpid(p, &s, 0);
    std::cout << "My child exited with status " << s << std::endl;
  }

  return 0;
}
