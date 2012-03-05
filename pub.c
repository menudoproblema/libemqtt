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
#include <unistd.h>
#include <arpa/inet.h>


int socket_id;

int send_packet(void *socket_info, const void *buf, unsigned int count)
{
	return write(*((int *)socket_info), buf, count);
}

int init_socket(mqtt_broker_handle_t *broker)
{
	if((socket_id = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	struct sockaddr_in socket_address;
	// Create the stuff we need to connect
	socket_address.sin_family = AF_INET;
	socket_address.sin_port = htons(broker->port);
	socket_address.sin_addr.s_addr = inet_addr(broker->hostname);

	// Connect
	if((connect(socket_id, (struct sockaddr *)&socket_address, sizeof(socket_address))) < 0)
		return -1;

	broker->socket_info = (void *)&socket_id;
	broker->send = send_packet;

	return 0;
}

int close_socket(mqtt_broker_handle_t *broker)
{
	return close(*(int *)broker->socket_info);
}


int main(int argc, char **argv) {
	int result;
	mqtt_broker_handle_t broker;

	mqtt_broker_init(&broker, "192.168.10.40", 1883, "libemqtt pub");
	init_socket(&broker);

	result = mqtt_connect(&broker);
	printf("Connect: %d\n", result);

	mqtt_publish(&broker, "hello/emqtt", "It's me", 1);

	mqtt_disconnect(&broker);
	close_socket(&broker);
	return 0;
}

