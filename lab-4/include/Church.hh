#pragma once

#include "Lambda.hh"

namespace lambda {
namespace church {

/// Returns the Church encoding of `true`.
inline Term tru() {
  return Abs{0, Abs{1, 0}};
}

/// Returns the Church encoding of `false`.
inline Term fls() {
  return Abs{0, Abs{1, 1}};
}

/// Returns the Church encoding of the logical AND between operator.
inline Term land() {
  return Abs{0, Abs{1, App{App{0, 1}, 0}}};
}

/// Returns the Church encoding of the logical OR between operator.
inline Term lor() {
  return Abs{0, Abs{1, App{App{0, 0}, 1}}};
}

/// Returns the Church encoding of `n`.
Term number(std::size_t n);

/// Returns the Chuch encoding of the successor function (`λn.λf.λx.f (n f x)`).
inline Term successor() {
  return Abs{0, Abs{1, Abs{2, App{1, App{App{0, 1}, 2}}}}};
}

} // namespace church
} // namespace lambda
