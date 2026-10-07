#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

long k = 0;
long pnum = 1;
long mod = 1;
long result = 1;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

struct Args {
  long begin;
  long end;
};

void *Worker(void *arg) {
  struct Args *a = (struct Args *)arg;
  long local = 1;
  for (long i = a->begin; i <= a->end; i++) {
    local = (local * (i % mod)) % mod;
  }
  pthread_mutex_lock(&mut);
  result = (result * local) % mod;
  pthread_mutex_unlock(&mut);
  return NULL;
}

int main(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-k") == 0 && i + 1 < argc)
      k = atol(argv[++i]);
    else if (strncmp(argv[i], "--pnum=", 7) == 0)
      pnum = atol(argv[i] + 7);
    else if (strncmp(argv[i], "--mod=", 6) == 0)
      mod = atol(argv[i] + 6);
  }

  if (k < 0 || pnum < 1 || mod < 1) {
    printf("Usage: ./factorial -k N --pnum=P --mod=M\n");
    return 1;
  }

  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);
  struct Args *args = malloc(sizeof(struct Args) * pnum);

  long chunk = k / pnum;
  if (chunk == 0) chunk = 1;

  for (long i = 0; i < pnum; i++) {
    args[i].begin = i * chunk + 1;
    args[i].end = (i == pnum - 1) ? k : (i + 1) * chunk;
    if (args[i].begin > k) args[i].begin = k + 1;  /* пустой диапазон */
    if (pthread_create(&threads[i], NULL, Worker, &args[i]) != 0) {
      perror("pthread_create");
      return 1;
    }
  }

  for (long i = 0; i < pnum; i++) pthread_join(threads[i], NULL);

  if (k == 0) result = 1 % mod;
  printf("%ld! mod %ld = %ld\n", k, mod, result);

  free(threads);
  free(args);
  return 0;
}
