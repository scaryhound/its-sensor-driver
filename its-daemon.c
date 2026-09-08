#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>

#define SENSOR_PATH "/dev/its_sensor"
#define BUFFER_SIZE 64
#define SLEEP_INTERVAL 2

int main(void) {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Open syslog for our daemon
    openlog("its_daemon", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting ITS Edge Gateway Daemon...");

    // Infinite loop to continuously read telemetry
    while (1) {
        // Open the sensor device node
        fd = open(SENSOR_PATH, O_RDONLY);
        if (fd < 0) {
            syslog(LOG_ERR, "Failed to open sensor node %s: %s", SENSOR_PATH, strerror(errno));
            sleep(SLEEP_INTERVAL);
            continue; // Keep trying instead of crashing
        }

        // Read the telemetry string
        bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the string
            
            // Strip the newline character if it exists
            buffer[strcspn(buffer, "\n")] = 0;
            
            syslog(LOG_INFO, "Telemetry Read: %s", buffer);
        } else if (bytes_read < 0) {
            syslog(LOG_ERR, "Failed to read from sensor: %s", strerror(errno));
        }

        close(fd);
        
        // Wait before reading again to simulate periodic sampling
        sleep(SLEEP_INTERVAL);
    }

    closelog();
    return 0;
}
