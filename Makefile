.PHONY: clean

CC = gcc
CFLAGS = -Wall -pedantic -pthread -O2 -std=c99
OBJS = \
	mqtt_broker.o \
	tcp_handler.o \
	mqtt_handler.o \
	subscription_manager.o

all: mqtt_broker

mqtt_broker: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o
