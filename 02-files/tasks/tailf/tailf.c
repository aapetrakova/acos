#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

int tail_f(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) { return -1; }

    char buffer[1024];
    int bytes_read;

    while (1) {
        bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read != -1) {
            if (write(STDOUT_FILENO, buffer, bytes_read) == -1) { return -1; }
        } else if (bytes_read < 1024) {
            break;
        }
        usleep(1000);
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) { return -1; }

    tail_f(argv[1]);

    return 0;
}
