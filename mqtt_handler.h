#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <stdint.h>
#include <sys/types.h>

ssize_t MQTT_handle_connect(uint8_t *response);

ssize_t MQTT_handle_publish(uint8_t *control_packet, ssize_t packet_size);

ssize_t MQTT_handle_subscribe(uint8_t *response, int connfd,
                              uint8_t *control_packet);

ssize_t MQTT_handle_ping(uint8_t *response);

ssize_t MQTT_handle_unsubscribe(uint8_t *response, int connfd,
                                uint8_t *control_packet);

ssize_t MQTT_handle_disconnect(int connfd);

ssize_t MQTT_handle_default(uint8_t message_type);
#endif