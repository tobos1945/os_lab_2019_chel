#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

void *Thread1(void *arg) {
  (void)arg;
  pthread_mutex_lock(&m1);
  printf("Thread 1: locked m1\n");
  sleep(1);                     /* даём второму потоку захватить m2 */
  pthread_mutex_lock(&m2);      /* здесь поток 1 застрянет */
  printf("Thread 1: locked m2\n");
  pthread_mutex_unlock(&m2);
  pthread_mutex_unlock(&m1);
  return NULL;
}

void *Thread2(void *arg) {
  (void)arg;
  pthread_mutex_lock(&m2);
  printf("Thread 2: locked m2\n");
  sleep(1);
  pthread_mutex_lock(&m1);      /* здесь поток 2 застрянет */
  printf("Thread 2: locked m1\n");
  pthread_mutex_unlock(&m1);
  pthread_mutex_unlock(&m2);
  return NULL;
}

int main(void) {
  pthread_t t1, t2;
  pthread_create(&t1, NULL, Thread1, NULL);
  pthread_create(&t2, NULL, Thread2, NULL);
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  printf("Finished (this line will never print)\n");
  return 0;
}
