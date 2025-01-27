#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
    
int create_directory(const char* path, mode_t mode) {
  struct stat dir_info;
  if (stat(path, &dir_info) == 0) {
    if (S_ISDIR(dir_info.st_mode)) { return 0; } 
    else { errno = ENOTDIR; return -1; }
  }

  if (mkdir(path, mode) != 0 && errno != EEXIST) { return -1; }

  return 0;
}

int create_directory_with_parents(const char* path, mode_t mode) {
  char* temp_path = (char*)malloc(strlen(path) * sizeof(char) + 1);
  if (!temp_path) { return -1; }
  strcpy(temp_path, path);
  char* pos = temp_path;
  
  int result = 0;
  while ((pos = strchr(pos, '/')) != NULL) {
    if (temp_path != pos) {
      *pos = '\0';
      if ((result = create_directory(temp_path, 0775)) != 0) { break; }
      *pos = '/';
    }
    ++pos;
  }
  
  free(temp_path);
  
  return (result ? result : create_directory(path, mode));
}
  
int main(int argc, char* argv[]) {
  int opt;
  int err = 0;
  _Bool parent_dirs = 0;
  mode_t mode = 0775;

  const char* short_options = "m:p";
  const struct option long_options[] = {
    {"mode", required_argument, NULL, 'm'},
    {NULL, 0, NULL, 0}
  };

  while ((opt = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
    if (opt == 'p') { parent_dirs = 1; }
    else if (opt == 'm') { mode = strtol(optarg, NULL, 8) & 0777; }
    else { return opterr; }
  }

  for (char** dir = argv + optind; dir < argv + argc; ++dir) {
    if (parent_dirs) { err = create_directory_with_parents(*dir, mode); }
    else { err = create_directory(*dir, mode); }
  }
  return err;
}
