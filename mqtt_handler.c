#include "mqtt_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt_broker.h"
#include "subscription_manager.h"

uint16_t get_unit16(uint8_t *byte_array) {
    return (byte_array[0] << 8) | byte_array[1];
}

ssize_t MQTT_handle_connect(uint8_t *response) {
    printf("CONNECT\n");
    ssize_t response_size = 5;
    uint8_t remaining_length = response_size - HEADER_LENGTH;
    uint8_t conn_ack_flags = 0;
    uint8_t conn_ack_reason_code = 0;
    uint8_t conn_ack_property_size = 0;
    uint8_t conn_ack_message[] = {CONNACK << 4, remaining_length,
                                  conn_ack_flags, conn_ack_reason_code,
                                  conn_ack_property_size};
    memcpy(response, conn_ack_message, response_size);
    return response_size;
}

ssize_t MQTT_handle_publish(uint8_t *control_packet, ssize_t packet_size) {
    printf("PUBLISH\n");
    uint8_t *body = control_packet + HEADER_LENGTH;
    uint16_t topic_size = get_unit16(body);
    uint8_t *body_topic = body + 2;

    char *topic = malloc(sizeof(char) * topic_size);
    memcpy(topic, body_topic, topic_size);
    topic[topic_size] = 0;

    SUBS_publish_message(topic, control_packet, packet_size);

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

    uint8_t subscribe_ack_message[] = {SUBACK << 4,   remaining_length,
                                       msg_id_1,      msg_id_2,
                                       property_size, granted_qos};
    memcpy(response, subscribe_ack_message, response_size);
    return response_size;
}

ssize_t MQTT_handle_subscribe(uint8_t *response, int connfd,
                              uint8_t *control_packet) {
    printf("SUBSCRIBE\n");
    uint8_t *body = control_packet + HEADER_LENGTH;
    uint16_t msg_id = get_unit16(body);
    uint8_t property_size = body[2];
    uint16_t topic_size = get_unit16(body + 3);
    uint8_t *body_topic = body + 5 + property_size;

    char *topic = malloc(sizeof(char) * topic_size);
    memcpy(topic, body_topic, topic_size);
    topic[topic_size] = 0;

    SUBS_add_subscription(connfd, topic);

    return get_subscribe_ack(msg_id, response);
}

ssize_t MQTT_handle_ping(uint8_t *response) {
    printf("PING\n");
    uint8_t remaining_length = 0;
    uint8_t ping_resp_message[] = {PINGRESP << 4, remaining_length};
    ssize_t response_size = sizeof(ping_resp_message) / sizeof(uint8_t);
    memcpy(response, ping_resp_message, response_size);
    return response_size;
}

ssize_t MQTT_handle_unsubscribe(int connfd) {
    printf("UNSUBSCRIBE\n");
    SUBS_remove_subscription(connfd);
    return 0;
}

ssize_t MQTT_handle_disconnect(int connfd) {
    printf("DISCONNECT\n");
    SUBS_remove_subscription(connfd);
    return 0;
}

ssize_t MQTT_handle_default(uint8_t message_type) {
    printf("DEFAULT - unhandled message type: %d\n", message_type);
    return 0;
}
