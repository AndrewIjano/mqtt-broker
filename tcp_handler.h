#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#define LISTEN_QUEUE 40

int TCP_init_socket();

int TCP_bind_socket_address(int socketfd, char *port);

int TCP_listen_connections(int listenfd);

int TCP_await_connection(int listenfd);

#endif