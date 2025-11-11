#include "array.hh"

void exchange(int* xs, int p, int q) {
  int x = xs[p];
  xs[p] = xs[q];
  xs[q] = x;
}

void reverse(int* xs, int length) {
    int i = 0;
    int j = length;
    while (i < j) { exchange(xs, i++, --j); }
}

void quicksort(int* start, int length) {
  // TODO
}
