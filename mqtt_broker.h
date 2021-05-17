/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 * 
 * Andrew Ijano Lopes - NUSP 10297797
 * 
 */

#ifndef MQTT_BROKER_H
#define MQTT_BROKER_H
#include <stdint.h>
#include <stdlib.h>

#define MAX_PACKET_SIZE 4096

#define HEADER_LENGTH 2

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

int main(int argc, char **argv);

void *handle_connection(void *connfd_pointer);

int handle_message(uint8_t *response, uint8_t message_type, int connfd,
                   uint8_t *packet, ssize_t packet_size);

void show_usage_and_exit(char *prog_name);

void exit_with_message(char *message);

#endif