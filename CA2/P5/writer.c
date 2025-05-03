#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAX_BUF_SIZE 1024

int main(int argc, char *argv[]) {
    char device[MAX_BUF_SIZE];
    char user_msg[MAX_BUF_SIZE];
    int fd;
    ssize_t ret;

    // Check command-line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <device> <message>\n", argv[0]);
        return 1;
    }

    // Copy device path and message
    strncpy(device, argv[1], MAX_BUF_SIZE - 1);
    device[MAX_BUF_SIZE - 1] = '\0';
    strncpy(user_msg, argv[2], MAX_BUF_SIZE - 1);
    user_msg[MAX_BUF_SIZE - 1] = '\0';

    // Open device in write-only mode
    fd = open(device, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // Write to device
    ret = write(fd, user_msg, strlen(user_msg));
    if (ret < 0) {
        perror("Failed to write");
        close(fd);
        return 1;
    }
    printf("Wrote %zd bytes: %s\n", ret, user_msg);

    // Close device
    close(fd);
    return 0;
}
