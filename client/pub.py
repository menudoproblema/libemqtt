from __future__ import print_function

import select
import signal
import socket
import sys

import libemqtt


def read(sck, timeout=None):
    try:
        inputready, outputready, exceptready = select.select([sck], [], [], timeout)
    except:
        return None

    if sck in inputready:
        fix_header = sck.recv(2)
        remain = ord(fix_header[1])

        data = sck.recv(remain)

        return libemqtt.MqttPacket(fix_header + data)
    return None


HOST = '192.168.10.40'
PORT = 1883
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

client = libemqtt.Mqtt(s, clientid='avengalvon', username='cid', password='campeador', keepalive=5)

def alive_callback(client):
    def alive(signum, frame):
        print('Timeout! Sending ping...')
        client.ping()
        signal.alarm(client.keepalive)

    return alive

signal.signal(signal.SIGALRM, alive_callback(client))
signal.alarm(client.keepalive)

# >>>>> CONNECT
client.connect()
# <<<<< CONNACK
packet = read(s)
if not packet:
    print('Error on read packet!', file=sys.stderr)
    sys.exit(-1)

if packet.type != libemqtt.CONNACK:
    print('CONNACK expected!', file=sys.stderr)
    sys.exit(-2)

if packet.index(3) != 0x00:
    print('CONNACK failed!', file=sys.stderr)
    sys.exit(-2)

# >>>>> PUBLISH QoS 0
print('Publish: QoS 0')
client.publish('hello/emqtt', 'Example: QoS 0')

# >>>>> PUBLISH QoS 1
print('Publish: Example: QoS 1')
msgid = client.publish('hello/emqtt', 'Example: QoS 1', qos=1)
# <<<<< PUBACK
packet = read(s)
if not packet:
    print('Error on read packet!', file=sys.stderr)
    sys.exit(-1)

if packet.type != libemqtt.PUBACK:
    print('PUBACK expected!', file=sys.stderr)
    sys.exit(-2)

if msgid != packet.message_id:
    print('%d message id was expected, but %d message id was found!' % (msgid, packet.message_id), file=sys.stderr)
    sys.exit(-2)

# >>>>> PUBLISH QoS 2
print('Publish: Example: QoS 2')
msgid = client.publish('hello/emqtt', 'Example: QoS 2', qos=2, retain=1)
# <<<<< PUBREC
packet = read(s)
if not packet:
    print('Error on read packet!', file=sys.stderr)
    sys.exit(-1)

if packet.type != libemqtt.PUBREC:
    print('PUBREC expected!', file=sys.stderr)
    sys.exit(-2)

if msgid != packet.message_id:
    print('%d message id was expected, but %d message id was found!' % (msgid, packet.message_id), file=sys.stderr)
    sys.exit(-2)

# >>>>> PUBREL
client.pubrel(msgid)
# <<<<< PUBCOMP
packet = read(s)
if not packet:
    print('Error on read packet!', file=sys.stderr)
    sys.exit(-1)

if packet.type != libemqtt.PUBCOMP:
    print('PUBCOMP expected!', file=sys.stderr)
    sys.exit(-2)

if msgid != packet.message_id:
    print('%d message id was expected, but %d message id was found!' % (msgid, packet.message_id), file=sys.stderr)
    sys.exit(-2)


# >>>>> DISCONNECT
client.disconnect()

s.close()
