#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <cjson/cJSON.h>
#include <mosquitto.h>

#define SENSOR_PATH "/dev/its_sensor"
#define MQTT_HOST "localhost"
#define MQTT_PORT 1883
#define MQTT_TOPIC "its/telemetry"
#define BUFFER_SIZE 64
#define SLEEP_INTERVAL 2

int main(void) {
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    struct mosquitto *mosq = NULL;
    int sensor_id, speed;

    openlog("its_daemon", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Starting ITS Edge Gateway Daemon...");

    /* Initialize Mosquitto */
    mosquitto_lib_init();
    mosq = mosquitto_new("its_edge_client", true, NULL);
    if (!mosq) {
        syslog(LOG_ERR, "Failed to create Mosquitto instance.");
        return 1;
    }

    /* Connect to MQTT Broker */
    if (mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS) {
        syslog(LOG_ERR, "Failed to connect to MQTT broker at %s:%d", MQTT_HOST, MQTT_PORT);
        /* In a production daemon, we would retry, but for the assignment we'll exit if no broker */
        return 1;
    }
    syslog(LOG_INFO, "Connected to Mosquitto broker.");

    /* Main Data Loop */
    while (1) {
        fd = open(SENSOR_PATH, O_RDONLY);
        if (fd < 0) {
            syslog(LOG_ERR, "Failed to open sensor node: %s", strerror(errno));
            sleep(SLEEP_INTERVAL);
            continue;
        }

        bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            
            /* Issue #5: Parse raw data and serialize to JSON */
            if (sscanf(buffer, "ID:%d,SPEED:%d", &sensor_id, &speed) == 2) {
                cJSON *json_payload = cJSON_CreateObject();
                cJSON_AddNumberToObject(json_payload, "sensor_id", sensor_id);
                cJSON_AddNumberToObject(json_payload, "speed", speed);
                
                char *json_string = cJSON_PrintUnformatted(json_payload);
                
                /* Issue #6: Publish MQTT Message */
                mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(json_string), json_string, 0, false);
                syslog(LOG_INFO, "Published: %s", json_string);
                
                /* Clean up memory */
                free(json_string);
                cJSON_Delete(json_payload);
            } else {
                syslog(LOG_WARNING, "Failed to parse sensor data: %s", buffer);
            }
        }

        close(fd);
        sleep(SLEEP_INTERVAL);
    }

    /* Cleanup (unreachable in infinite loop, but good practice) */
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    closelog();
    return 0;
}
