/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 *
 * Andrew Ijano Lopes - NUSP 10297797
 *
 */

#include "subscription_manager.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Subscription subscriptions[MAX_SUBSCRIBERS];
ssize_t subscriptions_size = 0;

pthread_mutex_t mutex;

void SUBS_add_subscription(int connfd, char *topic) {
    pthread_mutex_lock(&mutex);
    subscriptions[subscriptions_size++] = (Subscription){connfd, topic};
    pthread_mutex_unlock(&mutex);
    printf("-- adding subscriber %d in topic: '%s'\n", connfd, topic);
}

void SUBS_remove_subscription(int connfd) {
    int subscription_index = -1;
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < subscriptions_size; i++) {
        if (connfd == subscriptions[i].connfd) {
            subscription_index = i;
            break;
        }
    }

    if (subscription_index < 0) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    subscriptions[subscription_index] = subscriptions[--subscriptions_size];
    pthread_mutex_unlock(&mutex);
    printf("-- removing subscriber %d\n", connfd);
}

void SUBS_publish_message(char *topic, uint8_t *publishing_packet,
                          ssize_t packet_size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < subscriptions_size; i++) {
        Subscription sub = subscriptions[i];
        if (strcmp(topic, sub.topic) == 0) {
            printf("-- publishing in topic '%s' to subscriber %d\n", topic,
                   sub.connfd);
            write(sub.connfd, publishing_packet, packet_size);
        }
    }
    pthread_mutex_unlock(&mutex);
}
