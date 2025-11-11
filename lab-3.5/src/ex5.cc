#include <algorithm>
#include <iostream>
#include <functional>
#include <random>
#include <sstream>

#include <pthread.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cassert>

/// The minimum size of the array for which threads are created during a concurrent sort.
static inline constexpr std::size_t MIN_SPLIT = 4096;

/// Returns a textual description of the array of size `length` starting at `first`.
template<typename T>
std::string describe(T* xs, std::size_t length) {
  std::ostringstream o;
  o << "[";
  for (std::size_t i = 0; i < length; ++i) {
    if (i > 0) { o << ", "; }
    o << xs[i];
  }
  o << "]";
  return o.str();
}

/// Returns `true` iff the contents of the array of size `length` starting at `first`is ordered
/// according to `is_before`.
template<typename T, typename F>
bool is_ordered(T* first, std::size_t length, F&& is_before) {
  return (length < 2)
    ? true
    : is_before(first[0], first[1]) && is_ordered(first + 1, length - 1, is_before);
}

/// Sorts the of the array of size `length` starting at `first` according to `is_before`.
template<typename T, typename F>
void insertion_sort(T* first, std::size_t length, F&& is_before) {
  std::size_t p = 1;
  while (p < length) {
    std::size_t q = p;
    while ((q > 0) && is_before(first[q], first[q - 1])) {
      std::swap(first[q], first[q - 1]);
      q--;
    }
    p++;
  }
}

/// Sorts the of the array of size `length` starting at `first` according to `is_before`.
template<typename T, typename F>
void concurrent_quicksort(T* first, std::size_t length, F&& is_before);

/// The arguments of a call to `concurrent_quicksort`.
template<typename T, typename F>
struct concurrent_quicksort_arguments {
  T* first;
  std::size_t length;
  F const& is_before;
};

/// A trampoline function that calls `concurrent_quicksort` with the specified arguments.
template<typename T, typename F>
void* concurrent_quicksort_recurse(void* arguments) {
  auto const* a = static_cast<concurrent_quicksort_arguments<T, F>*>(arguments);
  concurrent_quicksort(a->first, a->length, a->is_before);
  return nullptr;
}

template<typename T, typename F>
void concurrent_quicksort(T* first, std::size_t length, F&& is_before) {
  // Nothing to do if the array contains less than 2 elements.
  if (length < 2) {
    return;
  }

  // Use insertion sort if the array contains less than 16 elements.
  if (length < 16) {
    insertion_sort(first, length, is_before);
    return;
  }

  // Partition the array so that elements in the range [0, m) are ordered before the last element.
  auto const p = length - 1;
  auto const m = std::partition(first, first + p, [&](auto const& x){ return x < first[p]; });

  // Move the pivot at its right position and sort the two partitions.
  std::swap(first[p], *m);
  auto const d = static_cast<std::size_t>(std::distance(first, m));

  // Sort the two partitions sequentially if the array contains less than MIN_SPLIT elements.
  if (length < MIN_SPLIT) {
    concurrent_quicksort(first, d, is_before);
    concurrent_quicksort(m + 1, length - d - 1, is_before);
  }

  // Otherwise, sort the two partitions concurrently.
  else {
    auto a = concurrent_quicksort_arguments{first, d, is_before};
    pthread_t th;
    pthread_create(&th, nullptr, concurrent_quicksort_recurse<T, F>, &a);

    // Sort the second partition in this thread.
    concurrent_quicksort(m + 1, length - d - 1, is_before);

    // Combine results.
    pthread_join(th, nullptr);
  }
}

int inner() {
  // Allocate an array of `n` floating-point numbers in shared memory.
  constexpr std::size_t n = 1 << 18;
  void* shared = mmap(
    0, n * sizeof(double),
    PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS,
    -1, 0);
  auto* xs = static_cast<double*>(shared);

  // Fill the array with random elements.
  std::random_device d;
  std::mt19937 mt(d());
  std::uniform_real_distribution<double> dice(0.0, 1000.0);
  for (std::size_t i = 0; i < n; ++i) {
    xs[i] = dice(mt);
  }

  // Sort the array.
  concurrent_quicksort(xs, n, std::less<double>{});
  assert(is_ordered(xs, n, std::less<double>{}));

  // Show the up to the 8 first elements of the array.
  std::cout << describe(xs, std::min<std::size_t>(n, 8)) << std::endl;
  return 0;
}

int main() {
  for (auto i = 0; i < 100; ++i) inner();
}


// Replace the quicksort by merge sort.