#include "utilities.hh"

#include <chrono>
#include <thread>
#include <iostream>

#include <sys/wait.h>
#include <signal.h>

void handle_child(int) {
  print_cstr("My child sent SIGUSR1");
}

int main() {
  // Install a signal handler for `SIGUSR1`.
  signal(SIGUSR1, handle_child);

  // Creates a clone of the current process.
  auto p = fork();

  // We're the child if `p` is equal to 0.
  if (p == 0) {
    // Wait for a second and send a signal to the parent.
    std::this_thread::sleep_for(std::chrono::seconds{1});
    kill(getppid(), SIGUSR1);

    // Wait for a another second.
    std::this_thread::sleep_for(std::chrono::seconds{1});
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
