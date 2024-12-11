#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum {
    MAX_ARGS_COUNT = 256,
    MAX_CHAIN_LINKS_COUNT = 256
};

typedef struct {
    char* command;
    uint64_t argc;
    char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
    uint64_t chain_links_count;
    chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;


void create_chain(char* command, chain_t* chain) {
  char* chain_token, *arg;
  char* save_command_ptr, *save_chain_ptr;

  chain_link_t* chain_link = chain->chain_links;
  for (chain->chain_links_count = 0; ; ++chain_link, command = NULL) {
    chain_token = strtok_r(command, "|", &save_command_ptr);
    if (chain_token == NULL) { break; }

    ++chain->chain_links_count;
    char** current_argument = &chain_link->command;

    for (chain_link->argc = 0; ; chain_token = NULL) {
      arg = strtok_r(chain_token, " ", &save_chain_ptr);
      if (arg == NULL) { break; }

      ++chain_link->argc;
      *current_argument = strdup(arg);

      if (current_argument == &chain_link->command) {
        chain_link->argv[0] = strdup(chain_link->command);
        current_argument = chain_link->argv + 1;
      } else {
        ++current_argument;
      }
    }

    if (chain_link->argc > 0) {
      chain_link->argv[chain_link->argc] = NULL;
    }
  }
}

void run_chain(chain_t* chain) {
  int previous_pipe[2] = {-1, -1};
  int current_pipe[2];

  for (uint64_t i = 0; i < chain->chain_links_count; ++i) {
    if (pipe(current_pipe) == -1) {
      perror("pipe");
      return;
    }

    pid_t pid = fork();
    if (pid == 0) {
      if (previous_pipe[0] != -1) {
        close(previous_pipe[1]);
        dup2(previous_pipe[0], STDIN_FILENO);
      }

      close(current_pipe[0]);
      dup2(current_pipe[1], STDOUT_FILENO);

      if (execvp(chain->chain_links[i].command, chain->chain_links[i].argv) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
      }
    }

    if (previous_pipe[0] != -1) {
      close(previous_pipe[0]);
      close(previous_pipe[1]);
    }

    previous_pipe[0] = current_pipe[0];
    previous_pipe[1] = current_pipe[1];
  }

  close(current_pipe[1]);

  char buffer[MAX_ARGS_COUNT];
  ssize_t bytes_read;
  while ((bytes_read = read(previous_pipe[0], buffer, sizeof(buffer))) > 0) {
    write(STDOUT_FILENO, buffer, bytes_read);
  }

  close(previous_pipe[0]);
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return -1;
  }
  
  chain_t my_chain;
  create_chain(argv[1], &my_chain);
  run_chain(&my_chain);

  for (uint64_t i = 0; i < my_chain.chain_links_count; ++i) {
    free(my_chain.chain_links[i].command);
    for (uint64_t j = 0; j < my_chain.chain_links[i].argc; ++j) {
      free(my_chain.chain_links[i].argv[j]);
    }
  }

  return 0;
}

