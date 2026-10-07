#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
  pid_t pid = fork();
  if (pid == 0) {
    printf("Child (pid=%d) exiting now\n", getpid());
    exit(0);
  }
  printf("Parent (pid=%d), child pid=%d. Spim 30s...\n",
         getpid(), pid);
  sleep(30);       
  wait(NULL);      
  printf("Child ynichtozen, no zombie\n");
  return 0;
}
