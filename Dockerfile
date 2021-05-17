FROM gcc:4.9
COPY . /usr/src/mqtt_broker
WORKDIR /usr/src/mqtt_broker
EXPOSE 1883/tcp
RUN make
CMD ["./mqtt_broker", "1883"]