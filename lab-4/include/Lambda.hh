#pragma once

#include "Indirect.hh"

#include <optional>
#include <ostream>
#include <variant>

namespace lambda {

struct Unit;
struct Var;
struct App;
struct Abs;

/// A term of the λ-calculus.
using Term = std::variant<
  Unit, // u
  Var,  // x
  Abs,  // λx.t
  App   // t t
>;

/// A unit value.
struct Unit {

  /// Returns `this`.
  Term substitute(Var, Term const&) const;

  /// Returns `true` iff `this` is equal to `other`.
  bool operator==(Unit const& other) const = default;

};

/// A variable.
struct Var {

  /// The identity of this variable.
  std::size_t id;

  /// Creates an instance with the given identity.
  Var(std::size_t id) : id(id) {}

  /// Returns `t` iff `this` is equal to `x`; otherwise, returns `this`.
  Term substitute(Var x, Term const& t) const;

  /// Returns `true` iff `this` is equal to other.
  bool operator==(Var const& other) const = default;

};

/// A term abstraction (i.e., a function).
struct Abs {

  /// The parameter of the function.
  Var parameter;

  /// The body of the function.
  Indirect<Term> body;

  /// Creates an instance denoting `λx.t`.
  Abs(Var x, Term const& t);

  /// Returns a copy of `this` in which free occurrences of `x` have been substituted for `t`.
  Term substitute(Var x, Term const& t) const;

  /// Returns `true` iff `this` is equal to `other`.
  bool operator==(Abs const& other) const;

};

/// The application of an abstraction (i.e., a function call).
struct App {

  /// The abstraction being applied.
  Indirect<Term> callee;

  /// The argument passed to the callee.
  Indirect<Term> argument;

  /// Creates an instance denoting `t u`.
  App(Term const& t, Term const& u);

  /// Returns a copy of `this` in which free occurrences of `x` have been substituted for `t`.
  Term substitute(Var x, Term const& t) const;

  /// Returns the one-step reduction of `this`, if any.
  ///
  /// Let `this` denote an application `t u`. The semantics of this method are defined as follows:
  ///
  /// - if `t` can step to `t'`, returns `t' u`; otherwise
  /// - if `u` can step to `u'`, returns `t 'u`; otherwise
  /// - if `t` is `λx.b`, returns `b` where `x` is substituted for `u`; otherwise
  /// - returns `std::nullopt`.
  std::optional<Term> step() const;

  /// Returns `true` iff `this` is equal to `other`.
  bool operator==(App const& other) const;

};

/// Returns `t` in which free occurrences of `x` have been substituted for `u`.
Term substitute(Term const& t, Var x, Term const& u);

/// Performs the one step reduction of `t` and returns `true` iff the result is not a value.
bool step(Term& t);

/// Returns the identity function.
inline Term identity() {
  return Abs{0, 0};
}

/// Returns a program that loops forever.
inline Term loopy() {
  Term t = Abs{0, App{0, 0}};
  return App{t, t};
}

/// Returns a program equivalent to `let x = t in u`.
inline Term let(Var x, Term const& t, Term const& u) {
  return App{Abs{x, u}, t};
}

std::ostream& operator<<(std::ostream& o, Term const& t);

std::ostream& operator<<(std::ostream& o, Unit const&);

std::ostream& operator<<(std::ostream& o, Var const& t);

std::ostream& operator<<(std::ostream& o, Abs const& t);

std::ostream& operator<<(std::ostream& o, App const& t);

}
