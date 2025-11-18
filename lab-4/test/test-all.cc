#include "Church.hh"
#include "Scheduler.hh"

#include <algorithm>
#include <boost/ut.hpp>
#include <type_traits>
#include <utility>

/// The maximum number of execution steps the program will simulate.
constexpr std::size_t max_fuel = 100;

/// Tests an instance of `S` using the given `tasks`.
///
/// This function uses `tasks` to check the postconditions of `S<n>::step`, where `S<n>` is some
/// subtype of `Scheduler<n>`.
///
/// The test stats by constructing a scheduler using `S<n>(tasks)`. Then, the `step` method of this
/// instance is called at most `max_fuel` times or until it returns `false`. The postconditions of
/// `step` are checked after each iteration.
///
/// The return value is a pair `(f, m)` where
/// - `f` is the amount of fuel left after the test completed.
/// - `m` is a mapping from task ID to the number of times it stepped.
template<template<std::size_t> typename S, std::size_t n>
std::pair<std::size_t, std::vector<std::size_t>> test_scheduler(sch::TaskList& tasks) {
  static_assert(std::is_base_of<sch::Scheduler<n>, S<n>>::value, "'T' is not a scheduler.");
  using namespace boost::ut;

  /// The scheduler under test.
  S<n> scheduler{tasks};

  /// The configuration of each task before the last step.
  sch::TaskList snapshot = tasks;

  /// The identities of the tasks that were running in the last step.
  std::vector<std::size_t> running;

  /// A mapping from task to the number of times it stepped.
  std::vector<std::size_t> progress;
  std::fill_n(std::back_inserter(progress), tasks.size(), 0);

  auto fuel = max_fuel;
  while ((fuel > 0) && scheduler.step()) {
    running.clear();
    running.reserve(tasks.size());

    for (std::size_t i = 0; i < tasks.size(); ++i) {
      if (tasks.at(i).running()) {
        running.push_back(i);
        snapshot.at(i).program = tasks.at(i).program;
        progress.at(i)++;
      } else {
        expect(tasks.at(i).program == snapshot.at(i).program);
      }

      expect(!snapshot.at(i).terminated() || tasks.at(i).terminated());
    }

    // There should be at most two running tasks.
    expect(running.size() <= 2);

    fuel--;
  }

  return std::make_pair(fuel, progress);
}

int main() {
  using namespace boost::ut;

  "[λ-calculus]"_test = [] {
    should("(λx.x u) → u") = [] {
      lambda::Term p = lambda::App{lambda::identity(), lambda::Unit{}};
      expect(lambda::step(p));
      expect(p == lambda::Term{lambda::Unit{}});
    };

    should("(λx.x x) (λx.x x) → (λx.x x) (λx.x x)") = [] {
      lambda::Term p = lambda::loopy();
      expect(lambda::step(p));
      expect(p == lambda::loopy());
    };

    should("λx.x \u2192\u0338") = [] {
      lambda::Term p = lambda::identity();
      expect(!lambda::step(p));
      expect(p == lambda::identity());
    };
  };

  "[Church encodings]"_test = [] {
    should("succ 0 → 1") = [] {
      using namespace lambda;
      lambda::Term p = lambda::App{church::successor(), church::number(0)};
      expect(lambda::step(p));
      // expect(p == church::number(1));
    };
  };

  "[Schedulers]"_test = [] {
    using namespace lambda;

    auto t = church::tru();
    auto f = church::fls();
    auto z = church::number(0);
    auto s = church::successor();

    // Create some tasks.
    sch::TaskList tasks = {
      sch::Task{loopy(), 4},
      sch::Task{App{App{church::land(), t}, f}, 3},
      sch::Task{App{App{church::lor(), t}, f}, 2},
      sch::Task{let(0, z, App{s, z}), 1},
      sch::Task{App{s, App{s, z}}, 0},
    };

    "[FCFS]"_test = [ts = tasks] mutable {
      auto r = test_scheduler<sch::FCFS, 2>(ts);

      // The two first tasks must have stepped at least once.
      expect(r.second.at(0) > 0);
      expect(r.second.at(1) > 0);
    };

    // "[RoundRobin]"_test = [ts = tasks] mutable {
    //   auto r = test_scheduler<sch::RoundRobin, 2>(ts);
    //
    //   // All tasks must have stepped at least once.
    //   auto b = std::all_of(r.second.begin(), r.second.end(), [](auto const& i) { return i > 0; });
    //   expect(b);
    // };

    // "[Priority]"_test = [ts = tasks] mutable {
    //   auto r = test_scheduler<sch::Priority, 2>(ts);
    //
    //   // Tasks with higher priority have stepped at least as much as tasks with lower priority.
    //   for (std::size_t i = ts.size(); i > 2; --i) {
    //     expect(r.second.at(i - 1) > r.second.at(i - 2));
    //   }
    // };
  };

  return 0;
}
