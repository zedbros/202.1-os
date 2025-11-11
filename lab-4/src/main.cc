#include "Church.hh"
#include "Scheduler.hh"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>

/// The maximum number of execution steps the program will simulate.
constexpr std::size_t max_fuel = 10;

int main() {
  using namespace lambda;

  auto t = church::tru();
  auto f = church::fls();

  // Create some tasks.
  sch::TaskList tasks = {
    sch::Task{App{App{church::land(), t}, f}},
    sch::Task{App{App{church::lor(), t}, f}},
  };

  // Create a scheduler.
  sch::FCFS<4> scheduler{tasks};

  // Run the scheduler until either all tasks are terminated or some timeout has been reached.
  auto fuel = max_fuel;
  while ((fuel > 0) && scheduler.step()) {
    if (fuel < max_fuel) { std::cout << std::endl; }
    fuel--;

    std::cout << "Fuel remaining: " << fuel << std::endl;
    for (std::size_t i = 0; i < tasks.size(); ++i) {
      std::cout << "  " << std::left << std::setw(2) << i << ": ";
      std::cout << tasks.at(i).program << std::endl;
    }
  }

  // Either we timed out or all tasks are terminated.
   assert(
    (fuel == 0)
    || std::all_of(tasks.begin(), tasks.end(), [](auto const& t) { return t.terminated(); }));

  return 0;
}
