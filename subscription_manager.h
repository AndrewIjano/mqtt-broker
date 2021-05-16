#ifndef MQTT_SUB_MANAGER_H
#define MQTT_SUB_MANAGER_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_SUBSCRIBERS 100

typedef struct {
    int connfd;
    char *topic;
} Subscription;

void SUBS_add_subscription(int connfd, char *topic);

void SUBS_remove_subscription(int connfd);

void SUBS_publish_message(char *topic, uint8_t *publishing_packet,
                          ssize_t packet_size);
#endif