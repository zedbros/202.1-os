#include "Lambda.hh"

#include <sstream>

namespace lambda {

Abs::Abs(Var x, Term const& t) : parameter(x), body(t) {}

bool Abs::operator==(Abs const& other) const {
  return (this->parameter == other.parameter) && (this->body == other.body);
}

App::App(Term const& t, Term const& u) : callee(t), argument(u) {}

bool App::operator==(App const& other) const {
  return (this->callee == other.callee) && (this->argument == other.argument);
}

Term Unit::substitute(Var, Term const&) const {
  return Unit{};
}

Term Var::substitute(Var x, Term const& t) const {
  return (*this == x) ? t : Var{id};
}

Term Abs::substitute(Var x, Term const& t) const {
  if (parameter == x) {
    return Abs{*this};
  } else {
    return Abs{parameter, lambda::substitute(*body, x, t)};
  }
}

Term App::substitute(Var x, Term const& t) const {
  return App{
    lambda::substitute(*callee, x, t),
    lambda::substitute(*argument, x, t)
  };
}

Term substitute(Term const& t, Var x, Term const& u) {
  return std::visit([&](auto&& a) -> Term {
    return a.substitute(x, u);
  }, t);
}

std::optional<Term> App::step() const {
  auto c = *(this->callee);
  if (lambda::step(c)) {
    return std::optional{App{c, *argument}};
  }

  auto a = *(this->argument);
  if (lambda::step(a)) {
    return std::optional{App{c, a}};
  }

  if (auto const* f = std::get_if<Abs>(&c)) {
    return std::optional{lambda::substitute(*(f->body), f->parameter, a)};
  } else {
    return std::nullopt;
  }
}

bool step(Term& t) {
  // Is the term an abstraction that is not in normal form?
  if (auto* a = std::get_if<Abs>(&t)) {
    return step(*(a->body));
  }

  // Is the term an application?
  else if (auto const* a = std::get_if<App>(&t)) {
    auto u = a->step();
    if (u.has_value()) {
      t = std::move(u.value());
      return true;
    } else {
      return false;
    }
  }

  // The term a normal form.
  else { return false; }
}

std::ostream& operator<<(std::ostream& o, Unit const&) {
  return o << "u";
}

std::ostream& operator<<(std::ostream& o, Var const& t) {
  return o << "x" << t.id;
}

std::ostream& operator<<(std::ostream& o, Abs const& t) {
  return o << "(λ" << t.parameter << "." << *(t.body) << ")";
}

std::ostream& operator<<(std::ostream& o, App const& t) {
  return o << "(" << *(t.callee) << " " << *(t.argument) << ")";
}

std::ostream& operator<<(std::ostream& o, Term const& t) {
  return std::visit([&](auto&& a) -> std::ostream& {
    return o << a;
  }, t);
}

} // namespace lambda
