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
 * Created by Filipe Varela on 09/10/16.
 * Copyright 2009 Caixa Mágica Software. All rights reserved.
 *
 * Fork developed by Vicente Ruiz Rodríguez
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libemqtt.h>


int mqtt_connect(mqtt_broker_handle_t *broker, uint16_t keepalive)
{
	int clientidlen = strlen(broker->clientid);

	if((broker->socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		return 0;

	// Create the stuff we need to connect
	broker->connected = 0;
	broker->socket_address.sin_family = AF_INET;
	broker->socket_address.sin_port = htons(broker->port);
	broker->socket_address.sin_addr.s_addr = inet_addr(broker->hostname);

	// Connect
	if((connect(broker->socket, (struct sockaddr *)&broker->socket_address, sizeof(broker->socket_address))) < 0)
		return -1;

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_CONNECT, // Message Type, DUP flag, QoS level, Retain
		12+strlen(broker->clientid)+2 // Remaining length
	};

	// Variable header
	uint8_t var_header[] = {
		0x00,0x06,0x4d,0x51,0x49,0x73,0x64,0x70, // Protocol name: MQIsdp
		0x03, // Protocol version
		0x02, // Connect flags (0x02 = clear session)
		keepalive>>4, keepalive&0xF,
		0x00, clientidlen
	};

	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+clientidlen];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), broker->clientid, clientidlen);

	// Send the packet
	if(send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet)) {
		close(broker->socket);
		return -1;
	}

	// Set connected flag
	broker->connected = 1;

	return 1;
}

int mqtt_disconnect(mqtt_broker_handle_t *broker)
{
	if(!broker->connected)
		return 0;

	uint8_t packet[] = {
		MQTT_MSG_DISCONNECT, // Message Type, DUP flag, QoS level, Retain
		0x00 // Remaining length
	};

	// Send the packet
	if(send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet))
		return -1;

	// Set connected flag
	broker->connected = 0;
	// Close the socket
	close(broker->socket);

	return 1;
}

int mqtt_ping(mqtt_broker_handle_t *broker)
{
	if(!broker->connected)
		return 0;

	uint8_t packet[] = {
		MQTT_MSG_PINGREQ, // Message Type, DUP flag, QoS level, Retain
		0x00 // Remaining length
	};

	// Send the packet
	if(send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet))
		return -1;

	return 1;
}

int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, char *msg)
{
	if(!broker->connected)
		return 0;

	int topiclen = strlen(topic);
	int msglen = strlen(msg);

	uint8_t var_header[topiclen+2];
	memset(var_header, 0, topiclen+2);
	var_header[1] = topiclen;
	memcpy(var_header+2, topic, topiclen);

	uint8_t fixed_header[] = {
		MQTT_MSG_PUBLISH, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+msglen // Remaining length
	};

	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+msglen];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), msg, msglen);

	// Send the packet
	if(send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet))
		return -1;

	return 1;
}

int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic, void *(*callback)(mqtt_callback_data_t *))
{
	if(!broker->connected)
		return 0;

	uint8_t var_header[] = {0,10};
	uint8_t fixed_header[] = {
		MQTT_MSG_SUBSCRIBE, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+strlen(topic)+3
	};

	// utf topic
	uint8_t utf_topic[strlen(topic)+3];
	strcpy((char *)&utf_topic[2], topic);

	utf_topic[0] = 0;
	utf_topic[1] = strlen(topic);
	utf_topic[sizeof(utf_topic)-1] = 0;

	char packet[sizeof(var_header)+sizeof(fixed_header)+strlen(topic)+3];
	memset(packet,0,sizeof(packet));
	memcpy(packet,fixed_header,sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header),var_header,sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header),utf_topic,sizeof(utf_topic));

	// Send the packet
	if(send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet))
		return -1;

	return 1;
}
