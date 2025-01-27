#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

int remove_path(const char* path, _Bool recursive_mode) {
  struct stat path_info;
  if (lstat(path, &path_info) == -1) { return -1; }

  if (S_ISDIR(path_info.st_mode)) {
    if (!recursive_mode) {
      errno = EISDIR;
      return -1;
    }

    DIR* dir_stream = opendir(path);
    if (dir_stream == NULL) { return -1; }

    struct dirent* entry;
    while ((entry = readdir(dir_stream)) != NULL) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        char* new_full_path = malloc(strlen(path) + strlen(entry->d_name) + 2);
        if (new_full_path == NULL) {
            closedir(dir_stream);
            return -1;
        }

        sprintf(new_full_path, "%s/%s", path, entry->d_name);

        if (remove_path(new_full_path, 1) == -1) {
          free(new_full_path);
          closedir(dir_stream);
          return -1;
        }
        free(new_full_path);
      }
    }

    closedir(dir_stream);
    return rmdir(path);
  } else { return unlink(path); }
}

int remove_multiple_paths(char* paths[], size_t num_paths, _Bool recursive_mode) {
  for (size_t i = 0; i < num_paths; ++i) {
    if (remove_path(paths[i], recursive_mode) < 0) { return -1; }
  }
  return 0;
}

int main(int argc, char* argv[]) {
  _Bool recursive_mode = 0;
  int opt;
  while ((opt = getopt(argc, argv, "r")) != -1) {
    if (opt == 'r') { recursive_mode = 1; }
    else { return opterr; }
    }

    return remove_multiple_paths(argv + optind, argc - optind, recursive_mode);
}
