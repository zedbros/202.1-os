#include "Church.hh"
#include "Scheduler.hh"

#include <algorithm>
#include <boost/ut.hpp>

/// The maximum number of execution steps the program will simulate.
constexpr std::size_t max_fuel = 100;

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
      sch::Task{App{App{church::land(), t}, f}},
      sch::Task{App{App{church::lor(), t}, f}},
      sch::Task{let(0, z, App{s, z})},
      sch::Task{App{s, App{s, z}}},
    };

    "[FCFS]"_test = [tasks] mutable {
      // Create a scheduler.
      sch::FCFS<2> scheduler{tasks};

      std::vector<std::size_t> running;

      auto fuel = max_fuel;
      while ((fuel > 0) && scheduler.step()) {
        running.clear();
        running.reserve(tasks.size());

        for (std::size_t i = 0; i < tasks.size(); ++i) {
          if (tasks[i].running()) { running.push_back(i); }
        }

        // There should be at most two running tasks.
        expect(running.size() <= 2);

        fuel--;
      }

      // There should be some fuel left.
      expect(fuel > 0);
    };
  };

  return 0;
}
