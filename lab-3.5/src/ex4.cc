#include "utilities.hh"

#include <chrono>
#include <thread>
#include <iostream>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

/// `true` iff the program should terminate.
static volatile bool should_exit = false;

void handle_interrupt(int) {
  print_cstr("Terminating ...");
  should_exit = true;
}

void handle_child(int) {
  int s = 0;
  while (wait(&s) > 0);
}

int main() {
  // Install a signal handler for `SIGINT`.
  signal(SIGINT, handle_interrupt);

  // Install a signal handler for `SIGCHLD`.
  signal(SIGCHLD, handle_child);

  // Creates a clone of the current process.
  auto p = fork();

  // We're the child if `p` is equal to 0.
  if (p == 0) {
    std::cout << "I am the child of " << getppid() << std::endl;
  }

  // Otherwise the child's PID has been written to `p`.
  else {
    while (!should_exit) {
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
  }

  return 0;
}
