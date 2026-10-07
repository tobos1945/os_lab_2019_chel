#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "psum_lib.h"

struct SumArgs {
  const int *array;
  int begin;
  int end;
};

void *ThreadSum(void *args) {
  struct SumArgs *a = (struct SumArgs *)args;
  return (void *)(size_t)Sum(a->array, a->begin, a->end);
}

int main(int argc, char **argv) {
  uint32_t threads_num = 1, array_size = 100, seed = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--threads_num") == 0 && i + 1 < argc)
      threads_num = atoi(argv[++i]);
    else if (strcmp(argv[i], "--array_size") == 0 && i + 1 < argc)
      array_size = atoi(argv[++i]);
    else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
      seed = atoi(argv[++i]);
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);   /* вне замера времени */

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);

  int chunk = array_size / threads_num;

  struct timespec t1, t2;
  clock_gettime(CLOCK_MONOTONIC, &t1);

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = i * chunk;
    args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk;
    if (pthread_create(&threads[i], NULL, ThreadSum, &args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }

  long total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    void *ret;
    pthread_join(threads[i], &ret);
    total_sum += (long)(size_t)ret;
  }

  clock_gettime(CLOCK_MONOTONIC, &t2);
  double elapsed = (t2.tv_sec - t1.tv_sec) +
                   (t2.tv_nsec - t1.tv_nsec) / 1e9;

  printf("Total: %ld\n", total_sum);
  printf("Time: %.6f sec\n", elapsed);

  free(array);
  free(threads);
  free(args);
  return 0;
}
