#include <dirent.h>
#include <fcntl.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

void perform_search(pcre* regex_pattern, const char* file_path) {
  struct stat file_info;
  if (lstat(file_path, &file_info) == -1) {
    perror("Failed to get file status");
    return;
  }

  if (S_ISDIR(file_info.st_mode)) {
    DIR* dir_stream = opendir(file_path);
    if (dir_stream == NULL) {
      perror("Failed to open directory");
      return;
    }

    struct dirent* entry;
    while ((entry = readdir(dir_stream)) != NULL) {
      if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
        char* new_file_path = malloc(strlen(file_path) + strlen(entry->d_name) + 2);
        if (new_file_path == NULL) {
          perror("Failed to allocate memory");
          closedir(dir_stream);
          return;
        }
        sprintf(new_file_path, "%s/%s", file_path, entry->d_name); 
        perform_search(regex_pattern, new_file_path);
        free(new_file_path);
      }
    }

    if (closedir(dir_stream) == -1) {
      perror("Failed to close directory");
      return;
    }
  } else {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
      perror("Failed to open file");
      return;
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
      perror("Failed to get file statistics");
      close(fd);
      return;
    }

    char* mapped_file = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
    if (mapped_file == MAP_FAILED) {
      perror("Failed to map file");
      close(fd);
      return;
    }

    close(fd);

    unsigned int line_number = 1;
    size_t start_index = 0;

    for (size_t idx = 0; idx < file_stat.st_size; ++idx) {
      if (mapped_file[idx] == '\n') {
        size_t line_length = idx - start_index;
        char* line = malloc(idx - start_index + 1);
        if (line == NULL) {
          perror("Failed to allocate memory for line");
          munmap(mapped_file, file_stat.st_size);
          return;
        }
        memcpy(line, mapped_file + start_index, line_length);
        line[line_length] = '\0';

        int options[30];
        memset(options, 0, sizeof(options));

        int result = pcre_exec(regex_pattern, NULL, line, line_length, 0, 0, options, 30);
        if (result > 0) { printf("%s:%d: %s\n", file_path, line_number, line); }

        free(line);
        ++line_number;
        start_index = idx + 1;
      }
    }

    if (start_index < file_stat.st_size) {
      size_t last_line_length = file_stat.st_size - start_index;
      char* last_line = malloc(last_line_length + 1);
      if (last_line == NULL) {
        perror("Failed to allocate memory for last line");
        munmap(mapped_file, file_stat.st_size);
        return;
      }

      memcpy(last_line, mapped_file + start_index, last_line_length);
      last_line[last_line_length] = '\0';

      int options[30];
      memset(options, 0, sizeof(options));

      int result = pcre_exec(regex_pattern, NULL, last_line, last_line_length, 0, 0, options, 30);
      if (result > 0) { printf("%s:%d: %s\n", file_path, line_number, last_line); }
      free(last_line);
    }

    munmap(mapped_file, file_stat.st_size);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <pattern> <directory_path>\n", argv[0]);
    return -1;
  }

  const char* regex = argv[1];
  const char* dir = argv[2];

  const char* error;
  int erroffset;
  pcre* re = pcre_compile(regex, 0, &error, &erroffset, NULL);
  if (re == NULL) {
    fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
    return -1;
  }

  perform_search(re, dir);
  pcre_free(re);

  return 0;
}
