#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DEVICE_PATH "/dev/myDev"
#define BUFFER_SIZE 4096

void read_data(int flags) {
    int fd;
    char read_buffer[BUFFER_SIZE + 1]; // +1 for null terminator
    ssize_t bytes_read;

    // Open the device
    fd = open(DEVICE_PATH, flags);
    if (fd < 0) {
        perror("Failed to open device");
        exit(EXIT_FAILURE);
    }

    // Read data from the device
    bytes_read = read(fd, read_buffer, 10 );
    if (bytes_read < 0) {
        if (errno == EAGAIN) {
            printf("Non-blocking read: no data available\n");
        } else {
            perror("Failed to read from the device");
        }
        close(fd);
        return;
    }
    read_buffer[bytes_read] = '\0'; // Null terminate the read data
    printf("Read %zd bytes: %s\n", bytes_read, read_buffer);

    // Close the device
    close(fd);
}
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
    const char *test_data = "eran_tayeb";

    printf("Testing non-blocking write...\n");
    write_data(test_data, O_WRONLY | O_NONBLOCK);

    printf("Testing non-blocking read...\n");
    read_data(O_RDONLY | O_NONBLOCK);



    return 0;
}
