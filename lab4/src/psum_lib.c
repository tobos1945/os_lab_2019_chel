#include "psum_lib.h"
#include <stdlib.h>

void GenerateArray(int *array, int size, unsigned int seed) {
  srand(seed);
  for (int i = 0; i < size; i++) array[i] = rand() % 100;
}

int Sum(const int *array, int begin, int end) {
  int s = 0;
  for (int i = begin; i < end; i++) s += array[i];
  return s;
}
