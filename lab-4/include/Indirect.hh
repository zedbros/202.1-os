#pragma once

#include <utility>

/// A box around an object stored out of line.
template<typename T>
struct Indirect {
private:

  /// The wrapped object.
  T* wrapped;

public:

  /// Creates an instance wrapping a copy of `value`.
  inline Indirect(T const& value) : wrapped(new T{value}) {}

  /// Creates an instance wrapping `value`.
  inline Indirect(T&& value) : wrapped(new T{std::move(value)}) {}

  /// Creates a copy of `other`.
  inline Indirect(Indirect const& other) : wrapped(new T{*other}) {}

  /// Destroys `this` and the object that it wraps.
  ~Indirect() {
    delete wrapped;
  }

  /// Wraps a copy of `other`s wrapped object.
  Indirect& operator=(Indirect& other) {
    *wrapped = *other;
    return *this;
  }

  /// Wraps `other`s wrapped object.
  Indirect& operator=(Indirect&& other) noexcept {
    std::swap(this->wrapped, other.wrapped);
    other.wrapped = nullptr;
    return *this;
  }

  /// Accesses the wrapped object.
  inline T& operator*() { return *wrapped; }

  /// Accesses the wrapped object.
  inline T const& operator*() const { return *wrapped; }

  /// Accesses the wrapped object.
  inline T* operator->() { return wrapped; }

  /// Accesses the wrapped object.
  inline T const* operator->() const { return wrapped; }

  /// Returns `true` iff `this` is equal to `other`.
  inline bool operator==(Indirect const& other) const { return **this == *other; }

};
