#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/user.h>
#include <sys/syscall.h>

typedef struct Counter{
    char filename[PATH_MAX];
    int counter;
    struct Counter* next;
} Counter;

typedef struct Counters{
    struct Counter* head;
} Counters;

void increment(Counters* counters, char* filename, int value) {
    Counter* current = counters->head;
    while (current != NULL) {
        if (strncmp(current->filename, filename, PATH_MAX) == 0) {
            current->counter += value;
            return;
        }
        current = current->next;
    }
    Counter* new_head = malloc(sizeof(Counter));
    new_head->next = counters->head;
    new_head->counter = value;
    strncpy(new_head->filename, filename, PATH_MAX - 1);
    counters->head = new_head;
}

void print(Counters* counters) {
    Counter* current = counters->head;
    while (current != NULL) {
        printf("%s:%d\n", current->filename, current->counter);
        current = current->next;
    }
}

int main(int argc, char *argv[]) {
  Counters* counters = malloc(sizeof(Counter));
  if (!counters) { return -1; }
  counters->head = NULL;

  int child_pid = fork();
  if (child_pid < 0) {
    free(counters);
    return -1;
  } else if (child_pid == 0) {
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) { return -1; }
    execvp(argv[1], argv + 1);
    return -1;
  }

  int status;
  waitpid(child_pid, &status, 0);

  if (ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD) == -1) {
    free(counters);
    return -1;
  }

  while (!WIFEXITED(status)) {
    if (ptrace(PTRACE_SYSCALL, child_pid, 0, 0) == -1) {
      free(counters);
      return -1;
    }

    waitpid(child_pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == (SIGTRAP | 0x80)) {
      struct user_regs_struct registers;
      if (ptrace(PTRACE_GETREGS, child_pid, 0, &registers) == -1) {
        free(counters);
        return -1;
      }

      if (registers.orig_rax == SYS_write) {
        char* descr_link = (char*)malloc(1ULL << 20);
        char* file_path = (char*)malloc(1ULL << 20);
        snprintf(descr_link, 1ULL << 20, "/proc/%u/fd/%llu", child_pid, registers.rdi);

        ssize_t file_path_len = readlink(descr_link, file_path, 1ULL << 20);
        if (file_path_len < 0) {
          free(counters);
          return -1;
        }

        file_path[file_path_len] = '\0';
        increment(counters, file_path, registers.rdx);
        free(descr_link);
        free(file_path);
      }

      ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
      waitpid(child_pid, &status, 0);
    }
  }

  print(counters);
  free(counters);
  return 0;
}


