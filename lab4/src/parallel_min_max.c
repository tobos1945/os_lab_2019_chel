#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define N_CHILDREN 2

int array_size = 10;
int *array;
pid_t *child_pids;
int children_count = 0;
volatile sig_atomic_t timed_out = 0;

void GenerateArray(int *a, int size, unsigned int seed) {
  srand(seed);
  for (int i = 0; i < size; i++) a[i] = rand() % 100;
}

void FindMinMax(int begin, int end, int *mn, int *mx) {
  *mn = array[begin];
  *mx = array[begin];
  for (int i = begin; i < end; i++) {
    if (array[i] < *mn) *mn = array[i];
    if (array[i] > *mx) *mx = array[i];
  }
}

void OnAlarm(int sig) {
  (void)sig;
  timed_out = 1;
  for (int i = 0; i < children_count; i++)
    if (child_pids[i] > 0) kill(child_pids[i], SIGKILL);
}

int main(int argc, char **argv) {
  int timeout = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc)
      timeout = atoi(argv[++i]);
  }

  array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, 42);

  child_pids = malloc(sizeof(pid_t) * N_CHILDREN);
  children_count = N_CHILDREN;

  if (timeout > 0) {
    signal(SIGALRM, OnAlarm);
    alarm(timeout);
  }

  for (int i = 0; i < N_CHILDREN; i++) {
    int begin = i * (array_size / N_CHILDREN);
    int end = (i == N_CHILDREN - 1) ? array_size
                                    : (i + 1) * (array_size / N_CHILDREN);
    pid_t pid = fork();
    if (pid == 0) {
      sleep(3);
      int mn, mx;
      FindMinMax(begin, end, &mn, &mx);
      printf("Child %d: min=%d max=%d\n", i, mn, mx);
      exit(0);
    }
    child_pids[i] = pid;
  }

  while (1) {
    int alive = 0;
    for (int i = 0; i < N_CHILDREN; i++) {
      if (child_pids[i] <= 0) continue;
      int status;
      pid_t r = waitpid(child_pids[i], &status, WNOHANG);
      if (r > 0) child_pids[i] = 0;
      else alive = 1;
    }
    if (!alive) break;
    usleep(1000);
  }

  if (timed_out) printf("Timeout! Children were killed.\n");

  free(array);
  free(child_pids);
  return 0;
}
