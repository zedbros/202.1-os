#include "Church.hh"

namespace lambda {
namespace church {

/// Returns `fⁿ x`.
Term number_helper(std::size_t n, Term const& f, Term const& x) {
  return (n == 0) ? x : number_helper(n - 1, f, App{f, x});
}

Term number(std::size_t n) {
  return Abs{0, Abs{1, number_helper(n, Var{0}, Var{1})}};
}

} // namespace church
} // namespace lambda
