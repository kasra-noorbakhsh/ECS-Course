#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int fd;
    char buf[BUFFER_SIZE];
    char *rev_buf;
    ssize_t ret;
    int i;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <device>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Failed to open device: %s\n", strerror(errno));
        return 1;
    }

    ret = read(fd, buf, BUFFER_SIZE - 1);
    if (ret < 0) {
        fprintf(stderr, "Failed to read: %s\n", strerror(errno));
        close(fd);
        return 1;
    } else if (ret == 0) {
        printf("EOF: No data available\n");
    } else {
        // Reverse the read bytes for printing
        rev_buf = malloc(ret + 1);
        if (!rev_buf) {
            fprintf(stderr, "Failed to allocate memory\n");
            close(fd);
            return 1;
        }
        for (i = 0; i < ret; i++) {
            rev_buf[i] = buf[ret - 1 - i];
        }
        rev_buf[ret] = '\0';
        printf("Read %zd bytes: %s\n", ret, rev_buf);
        free(rev_buf);
    }

    close(fd);
    return 0;
}
