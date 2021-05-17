/*
 * EP1 - Servidor MQTT
 * MAC0352 - Redes de Computadores e Sistemas Distribuídos
 * IME-USP (2021)
 *
 * Andrew Ijano Lopes - NUSP 10297797
 *
 */

#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#define LISTEN_QUEUE 40

int TCP_init_socket();

int TCP_bind_socket_address(int socketfd, char *port);

int TCP_listen_connections(int listenfd);

int TCP_await_connection(int listenfd);

#endif