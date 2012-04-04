/*
 * This file is part of libemqtt.
 *
 * libemqtt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libemqtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with libemqtt.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 *
 * Created by Vicente Ruiz Rodríguez
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#include <libemqtt.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/tcp.h>


#define RCVBUFSIZE 1024
uint8_t packet_buffer[RCVBUFSIZE];

int socket_id;




int send_packet(void* socket_info, const void* buf, unsigned int count)
{
	int fd = *((int*)socket_info);
	return send(fd, buf, count, 0);
}

int init_socket(mqtt_broker_handle_t* broker, const char* hostname, short port)
{
	int flag = 1;
	int keepalive = 3; // Seconds

	// Create the socket
	if((socket_id = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	// Disable Nagle Algorithm
	if (setsockopt(socket_id, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
		return -2;

	struct sockaddr_in socket_address;
	// Create the stuff we need to connect
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(port);
	socket_address.sin_addr.s_addr = inet_addr(hostname);

	// Connect the socket
	if((connect(socket_id, (struct sockaddr*)&socket_address, sizeof(socket_address))) < 0)
		return -1;

	// MQTT stuffs
	mqtt_set_alive(broker, keepalive);
	broker->socket_info = (void*)&socket_id;
	broker->send = send_packet;

	return 0;
}

int close_socket(mqtt_broker_handle_t* broker)
{
	int fd = *((int*)broker->socket_info);
	return close(fd);
}




int read_packet(int timeout)
{
	if(timeout > 0)
	{
		fd_set readfds;
		struct timeval tmv;

		// Initialize the file descriptor set
		FD_ZERO (&readfds);
		FD_SET (socket_id, &readfds);

		// Initialize the timeout data structure
		tmv.tv_sec = timeout;
		tmv.tv_usec = 0;

		// select returns 0 if timeout, 1 if input available, -1 if error
		if(select(1, &readfds, NULL, NULL, &tmv))
			return -2;
	}

	int total_bytes = 0, bytes_rcvd, packet_length;
	memset(packet_buffer, 0, sizeof(packet_buffer));

	while(total_bytes < 2) // Reading fixed header
	{
		if((bytes_rcvd = recv(socket_id, (packet_buffer+total_bytes), RCVBUFSIZE, 0)) <= 0)
			return -1;
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	packet_length = packet_buffer[1] + 2; // Remaining length + fixed header length

	while(total_bytes < packet_length) // Reading the packet
	{
		if((bytes_rcvd = recv(socket_id, (packet_buffer+total_bytes), RCVBUFSIZE, 0)) <= 0)
			return -1;
		total_bytes += bytes_rcvd; // Keep tally of total bytes
	}

	return packet_length;
}







int main(int argc, char* argv[])
{
	int packet_length;
	uint16_t msg_id, msg_id_rcv;
	mqtt_broker_handle_t broker;

	mqtt_init(&broker, "libemqtt pub");
	mqtt_init_auth(&broker, "username", NULL);
	init_socket(&broker, "192.168.10.40", 1883);

	// >>>>> CONNECT
	mqtt_connect(&broker);
	// <<<<< CONNACK
	packet_length = read_packet(1);
	if(packet_length < 0)
	{
		fprintf(stderr, "Error(%d) on read packet!\n", packet_length);
		return -1;
	}

	if(!MQTTMessageType(packet_buffer, MQTT_MSG_CONNACK))
	{
		fprintf(stderr, "CONNACK expected!\n");
		return -2;
	}

	if(packet_buffer[3] != 0x00)
	{
		fprintf(stderr, "CONNACK failed!\n");
		return -2;
	}

	// >>>>> PUBLISH QoS 0
	printf("Publish: QoS 0\n");
	mqtt_publish(&broker, "hello/emqtt", "Example: QoS 0", 0);

	// >>>>> PUBLISH QoS 1
	printf("Publish: QoS 1\n");
	mqtt_publish_with_qos(&broker, "hello/emqtt", "Example: QoS 1", 0, 1, &msg_id);
	// <<<<< PUBACK
	packet_length = read_packet(1);
	if(packet_length < 0)
	{
		fprintf(stderr, "Error(%d) on read packet!\n", packet_length);
		return -1;
	}

	if(!MQTTMessageType(packet_buffer, MQTT_MSG_PUBACK))
	{
		fprintf(stderr, "PUBACK expected!\n");
		return -2;
	}

	MQTTMessageIDFromBuffer((packet_buffer+2), msg_id_rcv);
	if(msg_id != msg_id_rcv)
	{
		fprintf(stderr, "%d message id was expected, but %d message id was found!\n", msg_id, msg_id_rcv);
		return -3;
	}

	// >>>>> PUBLISH QoS 2
	printf("Publish: QoS 2\n");
	mqtt_publish_with_qos(&broker, "hello/emqtt", "Example: QoS 2", 1, 2, &msg_id); // Retain
	// <<<<< PUBACK
	packet_length = read_packet(1);
	if(packet_length < 0)
	{
		fprintf(stderr, "Error(%d) on read packet!\n", packet_length);
		return -1;
	}

	if(!MQTTMessageType(packet_buffer, MQTT_MSG_PUBACK))
	{
		fprintf(stderr, "PUBACK expected!\n");
		return -2;
	}

	MQTTMessageIDFromBuffer((packet_buffer+2), msg_id_rcv);
	if(msg_id != msg_id_rcv)
	{
		fprintf(stderr, "%d message id was expected, but %d message id was found!\n", msg_id, msg_id_rcv);
		return -3;
	}

	// >>>>> PUBREL
	mqtt_pubrel(&broker, msg_id);
	// <<<<< PUBCOMP
	packet_length = read_packet(1);
	if(packet_length < 0)
	{
		fprintf(stderr, "Error(%d) on read packet!\n", packet_length);
		return -1;
	}

	if(!MQTTMessageType(packet_buffer, MQTT_MSG_PUBCOMP))
	{
		fprintf(stderr, "PUBCOMP expected!\n");
		return -2;
	}

	MQTTMessageIDFromBuffer((packet_buffer+2), msg_id_rcv);
	if(msg_id != msg_id_rcv)
	{
		fprintf(stderr, "%d message id was expected, but %d message id was found!\n", msg_id, msg_id_rcv);
		return -3;
	}

	// >>>>> DISCONNECT
	mqtt_disconnect(&broker);
	close_socket(&broker);
	return 0;
}

