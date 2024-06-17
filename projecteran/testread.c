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
    char read_buffer[BUFFER_SIZE + 1]; 
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

int main() {
    printf("Testing blocking read...\n");
    read_data(O_RDONLY);



    return 0;
}
