#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

static int signal_fd[2]; 

void signal_handler(int sig) {
  if (write(signal_fd[1], &sig, sizeof(sig)) == -1) { return; }
}

int signalfd() {
  if (pipe(signal_fd) == -1) { return -1; }

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);

  for (int sig_num = 1; sig_num < 32; ++sig_num) {
    if (sig_num != SIGKILL && sig_num != SIGSTOP) {
      if (sigaction(sig_num, &sa, NULL) == -1) {
        close(signal_fd[0]);
        close(signal_fd[1]);
        return -1;
      }
    }
  }

  return signal_fd[0];
}

void close_signalfd() {
  close(signal_fd[0]); 
  close(signal_fd[1]); 
}

