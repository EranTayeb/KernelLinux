#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DEVICE_PATH "/dev/myDev"
#define BUFFER_SIZE 4096

void write_data(const char *data, int flags) {
    int fd;
    ssize_t bytes_written;

    // Open the device
    fd = open(DEVICE_PATH, flags);
    if (fd < 0) {
        perror("Failed to open device");
        exit(EXIT_FAILURE);
    }
    // Write data to the device
    bytes_written = write(fd, data, strlen(data));
    if (bytes_written < 0) {
        perror("Failed to write to the device");
        close(fd);
        exit(EXIT_FAILURE);
    }
    printf("Wrote %zd bytes: %s\n", bytes_written, data);

    // Close the device
    close(fd);
}

int main() {
    const char *test_data = "HelloWorldkkkb";

    printf("Testing blocking write...\n");
    write_data(test_data, O_WRONLY);

    return 0;
}
