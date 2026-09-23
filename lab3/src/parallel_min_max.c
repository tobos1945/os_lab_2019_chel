#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    static struct option options[] = {
        {"seed",       required_argument, 0, 0},
        {"array_size", required_argument, 0, 0},
        {"pnum",       required_argument, 0, 0},
        {"by_files",   no_argument,       0, 'f'},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);
    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed < 0) { printf("seed must be non-negative\n"); return 1; }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) { printf("array_size must be positive\n"); return 1; }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) { printf("pnum must be positive\n"); return 1; }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;

      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  if (array == NULL) { perror("malloc"); return 1; }
  GenerateArray(array, array_size, seed);

  int active_child_processes = 0;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  
  int (*pipes)[2] = NULL;
  if (!with_files) {
    pipes = malloc(pnum * sizeof(*pipes));
    if (pipes == NULL) { perror("malloc"); free(array); return 1; }
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) == -1) {
        perror("pipe");
        for (int j = 0; j < i; j++) { close(pipes[j][0]); close(pipes[j][1]); }
        free(pipes); free(array);
        return 1;
      }
    }
  }

 
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();

    if (child_pid < 0) { perror("fork"); return 1; }
    active_child_processes += 1;

    if (child_pid == 0) {
      unsigned int begin = (unsigned int)i * array_size / pnum;
      unsigned int end   = (unsigned int)(i + 1) * array_size / pnum;

      struct MinMax min_max = GetMinMax(array, begin, end);

      if (with_files) {
        char filename[64];
        snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
        FILE *f = fopen(filename, "w");
        if (f == NULL) { perror("fopen"); _exit(1); }
        fprintf(f, "%d %d\n", min_max.min, min_max.max);
        fclose(f);
      } else {
        for (int j = 0; j < pnum; j++) {
          close(pipes[j][0]);
          if (j != i) close(pipes[j][1]);
        }
        ssize_t n = write(pipes[i][1], &min_max, sizeof(min_max));
        if (n != (ssize_t)sizeof(min_max)) { perror("write"); _exit(1); }
        close(pipes[i][1]);
      }
      _exit(0);
    }

    if (!with_files) close(pipes[i][1]);
  }

  while (active_child_processes > 0) {
    int status;
    pid_t pid = wait(&status);
    if (pid > 0) active_child_processes -= 1;
    else if (pid == -1) { perror("wait"); break; }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[64];
      snprintf(filename, sizeof(filename), "minmax_%d.txt", i);
      FILE *f = fopen(filename, "r");
      if (f != NULL) {
        if (fscanf(f, "%d %d", &min, &max) != 2) { min = INT_MAX; max = INT_MIN; }
        fclose(f);
        unlink(filename);
      }
    } else {
      struct MinMax child_min_max;
      ssize_t n = read(pipes[i][0], &child_min_max, sizeof(child_min_max));
      if (n == (ssize_t)sizeof(child_min_max)) {
        min = child_min_max.min;
        max = child_min_max.max;
      }
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }
  free(pipes);

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);

  return 0;
}
