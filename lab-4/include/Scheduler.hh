#pragma once

#include "Lambda.hh"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <forward_list>
#include <vector>

namespace sch {

/// The description of a process.
struct Task {

  /// The program being executed by the process.
  lambda::Term program;

  /// The priority of the process (higher is more important).
  const std::uint8_t priority = 0;

  /// The current state of the process.
  enum { Ready, Running, Terminated } state = Ready;

  /// Returns `true` iff this task is running.
  inline bool running() const { return state == Running; }

  /// Returns `true` iff this task is terminated.
  inline bool terminated() const { return state == Terminated; }

};

/// An collection of tasks.
using TaskList = std::vector<Task>;

/// The execution of scheduling algorithm for processing tasks on a system using `core_count` CPUs.
template<std::size_t core_count>
struct Scheduler {

  /// Destroys `this`.
  virtual ~Scheduler() {}

  /// Advances the state of the system and returns `true` iff there are outstanding tasks.
  ///
  /// This method simulates the progress of the system and is meant to be called repeatedly until
  /// either all tasks have been completed or some timeout has been reached.
  ///
  /// `step` selects up to `core_count` tasks, moves them to `Task::Running` and steps them. Other
  /// tasks are moved to `Task::Ready` if they should be scheduled later, or `Task::Terminated` if
  /// they terminated. A task cannot move to the `Task::Terminated` if it can step.
  ///
  /// `step` does not remove nor reorder the elements of `tasks`. Any change to the contents of
  /// `tasks` between consecutive calls to `step` may trigger undefined behavior.
  ///
  /// The return value is `true` iff at least one task stepped. Otherwise, the result is `false`
  /// and all task are in `Task::Terminated`, meaning that the system can no longer progress.
  virtual bool step() = 0;

};

/// The execution of scheduling algorithm adopting the "first-come, first-serve" strategy.
template<std::size_t core_count>
struct FCFS final : public Scheduler<core_count> {

  static_assert(core_count > 0);

private:

  /// The tasks to process.
  TaskList& tasks;

  /// The IDs of the task not yet completed.
  std::forward_list<std::size_t> work;

public:

  /// Initializes the state of a scheduler applying this strategy to process `tasks`.
  FCFS(TaskList& tasks) : tasks(tasks) {
    // Fill `work` with task indices, in reverse order.
    for (std::size_t i = tasks.size(); i > 0; --i) { work.push_front(i - 1); }
  }

  bool step() {
    // Is there any work left?
    if (work.empty()) { return false; }

    std::size_t processed = 0;
    auto i = work.before_begin();
    auto j = work.begin();
    while ((processed < core_count) && (j != work.end())) {
      // Pull the next task, which is either ready or running.
      auto& task = tasks.at(*j);
      assert(task.state != Task::Terminated);

      // Does the task step?
      if (lambda::step(task.program)) {
        task.state = Task::Running;
        i = j++;
      } else {
        task.state = Task::Terminated;
        j = work.erase_after(i);
      }

      processed++;
    }

    return true;
  }

};

/// The execution of scheduling algorithm adopting the "round-robin" strategy.
template<std::size_t core_count>
struct RoundRobin final : public Scheduler<core_count> {

  static_assert(core_count > 0);

private:
public:

  /// Initializes the state of a scheduler applying this strategy to process `tasks`.
  RoundRobin(TaskList& tasks) {
    // TODO
  }

  bool step() {
    // TODO
    return false;
  }

};

/// The execution of scheduling algorithm adopting priority scheduling.
template<std::size_t core_count>
struct Priority final : public Scheduler<core_count> {

  static_assert(core_count > 0);

private:
public:

  /// Initializes the state of a scheduler applying this strategy to process `tasks`.
  Priority(TaskList& tasks) {
    // TODO
  }

  bool step() {
    // TODO
    return false;
  }

};

} // namespace sch
