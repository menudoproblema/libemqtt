#
# This file is part of python-emqtt.
#
# python-emqtt is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# python-emqtt is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with python-emqtt.  If not, see <http://www.gnu.org/licenses/>.
#
#
# Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
#

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
        if len(fix_header):
            remain = ord(fix_header[1])

            data = sck.recv(remain)

            return libemqtt.MqttPacket(fix_header + data)
    return None


HOST = '192.168.10.40'
PORT = 1883
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

client = libemqtt.Mqtt(s, clientid='sancho', username='quijote', password='rocinante', keepalive=5)

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

# >>>>> SUBSCRIBE
msgid = client.subscribe("hello/emqtt")
# <<<<< SUBACK
packet = read(s)
if not packet:
    print('Error on read packet!', file=sys.stderr)
    sys.exit(-1)

if packet.type != libemqtt.SUBACK:
    print('SUBACK expected!', file=sys.stderr)
    sys.exit(-2)

if msgid != packet.message_id:
    print('%d message id was expected, but %d message id was found!' % (msgid, packet.message_id), file=sys.stderr)
    sys.exit(-2)

try:
    while True:
        # <<<<<
        packet = read(s)
        if packet:
            print('Packet Header: 0x%x' % packet.index(0))
            if packet.type == libemqtt.PUBLISH:
                print(packet.get_topic(), packet.get_message())
except KeyboardInterrupt:
    print('Goodbye!')


client.disconnect()

s.close()
