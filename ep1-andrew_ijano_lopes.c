/*
 * TODO:
 * - refatorar header
 * - reorganizar códig
 * - validar funcionalidades
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1
#define MAX_PACKET_SIZE 4096

#define HEADER_LENGTH 2
#define MAX_SUBSCRIBERS 50

enum MessageType {
    CONNECT = 1,
    CONNACK,
    PUBLISH,
    PUBACK,
    PUBREC,
    PUBREL,
    PUBCOMP,
    SUBSCRIBE,
    SUBACK,
    UNSUBSCRIBE,
    UNSUBACK,
    PINGREQ,
    PINGRESP,
    DISCONNECT
};

typedef struct {
    int connfd;
    char *topic;
} Subscription;

Subscription subscriptions[MAX_SUBSCRIBERS];
ssize_t subscriptions_size = 0;

pthread_mutex_t mutex;

uint16_t get_unit16(uint8_t *byte_array) {
    return (byte_array[0] << 8) | byte_array[1];
}

void add_subscription(int connfd, char *topic) {
    pthread_mutex_lock(&mutex);
    subscriptions[subscriptions_size++] = (Subscription){connfd, topic};
    pthread_mutex_unlock(&mutex);
}

void remove_subscription(int connfd) {
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
}

void publish_message(char *topic, uint8_t *publishing_packet, int packet_size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < subscriptions_size; i++) {
        Subscription sub = subscriptions[i];
        printf("-- publishing in topic '%s' to subscriber %d\n", topic, sub.connfd);
        if (strcmp(topic, sub.topic) == 0) {
            write(sub.connfd, publishing_packet, packet_size);
        }
    }
    pthread_mutex_unlock(&mutex);
}

ssize_t handle_connect(uint8_t *response) {
    printf("CONNECT\n");
    ssize_t response_size = 5;
    uint8_t remaining_length = response_size - HEADER_LENGTH;
    uint8_t conn_ack_flags = 0;
    uint8_t conn_ack_reason_code = 0;
    uint8_t conn_ack_property_size = 0;
    uint8_t conn_ack_message[] = {CONNACK << 4, remaining_length, conn_ack_flags,
                                  conn_ack_reason_code, conn_ack_property_size};
    memcpy(response, conn_ack_message, response_size);
    return response_size;
}

ssize_t handle_publish(uint8_t *body, uint8_t *read_control_packet,
                       int packet_size) {
    printf("PUBLISH\n");
    uint16_t topic_size = get_unit16(body);
    uint8_t *body_topic = body + 2;

    char *topic = malloc(sizeof(char) * topic_size);
    memcpy(topic, body_topic, topic_size);
    topic[topic_size] = 0;

    publish_message(topic, read_control_packet, packet_size);

    free(topic);
    return 0;
}

ssize_t get_subscribe_ack(uint16_t msg_id, uint8_t *response) {
    ssize_t response_size = 6;
    uint8_t remaining_length = response_size - HEADER_LENGTH;
    uint8_t msg_id_1 = (msg_id >> 4) & 0x0F;
    uint8_t msg_id_2 = msg_id & 0x0F;
    uint8_t property_size = 0;
    uint8_t granted_qos = 0;

    uint8_t subscribe_ack_message[] = {SUBACK << 4,  remaining_length,
                                       msg_id_1,     msg_id_2,
                                       property_size, granted_qos};
    memcpy(response, subscribe_ack_message, response_size);
    return response_size;
}

ssize_t handle_subscribe(uint8_t *response, int connfd, uint8_t *body) {
    printf("SUBSCRIBE\n");
    uint16_t msg_id = get_unit16(body);
    uint8_t property_size = body[2];
    uint16_t topic_size = get_unit16(body + 3);
    uint8_t *body_topic = body + 5 + property_size;

    char *topic = malloc(sizeof(char) * topic_size);
    memcpy(topic, body_topic, topic_size);
    topic[topic_size] = 0;

    add_subscription(connfd, topic);

    printf("-- subscriber %d in topic: '%s'\n", connfd, topic);

    return get_subscribe_ack(msg_id, response);
}

ssize_t handle_ping(uint8_t *response) {
    printf("PING\n");
    uint8_t remaining_length = 0;
    uint8_t ping_resp_message[] = {PINGRESP << 4, remaining_length};
    ssize_t response_size = sizeof(ping_resp_message) / sizeof(uint8_t);
    memcpy(response, ping_resp_message, response_size);
    return response_size;
}

ssize_t handle_unsubscribe(int connfd) {
    printf("UNSUBSCRIBE\n");
    remove_subscription(connfd);
    return 0;
}

ssize_t handle_disconnect(int connfd) {
    printf("DISCONNECT\n");
    remove_subscription(connfd);
    return 0;
}



void *handle_connection(void *connfd_pointer) {
    int connfd = *((int *)connfd_pointer);

    printf("\nconnection %d open\n", connfd);

    uint8_t packet[MAX_PACKET_SIZE + 1];
    ssize_t packet_size;

    while ((packet_size = read(connfd, packet, HEADER_LENGTH)) > 0) {
        uint8_t header = packet[0];
        uint8_t body_size = packet[1];
        uint8_t msg_type = (header >> 4) & 0x0F;

        packet_size += read(connfd, packet + HEADER_LENGTH, body_size);
        uint8_t *body = packet + HEADER_LENGTH;

        uint8_t response[MAX_PACKET_SIZE];
        ssize_t response_size = 0;
        printf("client %d with message type ", connfd);
        switch (msg_type) {
            case CONNECT:
                response_size = handle_connect(response);
                break;

            case PUBLISH:
                handle_publish(body, packet, packet_size);
                break;

            case SUBSCRIBE:
                response_size = handle_subscribe(response, connfd, body);
                break;

            case UNSUBSCRIBE:
                handle_unsubscribe(connfd);
                break;

            case PINGREQ:
                response_size = handle_ping(response);
                break;

            case DISCONNECT:
                handle_disconnect(connfd);
                break;

            default:
                printf("DEFAULT - unhandled message type: %d\n", msg_type);
                break;
        }

        write(connfd, response, response_size);
    }

    close(connfd);
    printf("connection %d closed\n\n", connfd);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        fprintf(stderr, "You will run a MQTT broker in port <port> TCP\n");
        exit(1);
    }

    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("\nbroker running on port %s 🚀\n", argv[1]);

    int connfd;
    for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) == -1) {
            perror("accept :(\n");
            exit(5);
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handle_connection, (void *)&connfd);
    }

    printf("\n broker terminating\n");

    exit(0);
}
