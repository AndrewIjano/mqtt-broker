import os
import sys
import subprocess
import time
import threading
import random

MAX_SLEEP = 5 # seconds

def build_get_topic(topic, is_random=False):
    def _get_topic():
        if not is_random:
            return topic
        return random.choice(['a', 'b', 'c', 'd', 'e'])
    return _get_topic

def publish(port, get_topic, should_loop):
    cmd = 'mosquitto_pub -h localhost -m "Hello" -t {0} -V mqttv5 -p {1}'
    proc = subprocess.Popen([cmd.format(get_topic(), port)], shell=True)
    if should_loop:
        while True:
            time.sleep(random.random() * MAX_SLEEP)
            proc.terminate()
            proc = subprocess.Popen([cmd.format(get_topic(), port)], shell=True)

def subscribe(port, get_topic, should_loop):
    cmd = 'mosquitto_sub -h localhost -t {0} -V mqttv5 -p {1}'
    proc = subprocess.Popen([cmd.format(get_topic(), port)], shell=True)
    if should_loop:
        while True:
            time.sleep(random.random() * MAX_SLEEP)
            proc.terminate()
            proc = subprocess.Popen([cmd.format(get_topic(), port)], shell=True)

def publish_all(n, port, get_topic, should_loop):
    for i in range(n):
        t = threading.Thread(target=publish, args=(port, get_topic, should_loop))
        t.start()

def subscribe_all(n, port, get_topic, should_loop):
    for i in range(n):
        t = threading.Thread(target=subscribe, args=(port, get_topic, should_loop))
        t.start()

if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(f'Usage: python {sys.argv[0]} <action> <n> <port> <topic> <should_loop>')
        exit()    

    _, action, n, port, topic, should_loop, *_ = sys.argv
    should_loop = should_loop == 'y'

    get_topic = build_get_topic(topic, topic == 'random')
    if action == 'p':
        publish_all(int(n), port, get_topic, should_loop)
    else:
        subscribe_all(int(n), port, get_topic, should_loop)

