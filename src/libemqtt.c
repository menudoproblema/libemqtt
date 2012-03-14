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
#include <libemqtt.h>

#define MQTT_DUP_FLAG     1<<3
#define MQTT_QOS0_FLAG    0<<1
#define MQTT_QOS1_FLAG    1<<1
#define MQTT_QOS2_FLAG    2<<1

#define MQTT_RETAIN_FLAG  1

#define MQTT_CLEAN_SESSION  1<<1
#define MQTT_WILL_FLAG      1<<2
#define MQTT_WILL_RETAIN    1<<5
#define MQTT_USERNAME_FLAG  1<<7
#define MQTT_PASSWORD_FLAG  1<<6


void mqtt_init(mqtt_broker_handle_t *broker, const char* hostname, short port, const char* clientid)
{
	// Connection options
	broker->connected = 0;
	broker->alive = 300; // 300 seconds = 5 minutes
	broker->seq = 0; // Sequency for message indetifiers
	// Client options
	if(clientid)
		strncpy(broker->clientid, clientid, sizeof(broker->clientid));
	else
		strcpy(broker->clientid, "emqtt");
	memset(broker->username, 0, sizeof(broker->username));
	memset(broker->password, 0, sizeof(broker->password));
	// Broker options
	broker->port = port ? port : 1883;
	strncpy(broker->hostname, hostname, sizeof(broker->hostname));
}

void mqtt_init_auth(mqtt_broker_handle_t *broker, const char* username, const char* password)
{
	strncpy(broker->username, username, sizeof(broker->username));
	strncpy(broker->password, password, sizeof(broker->password));
}

int mqtt_connect(mqtt_broker_handle_t *broker)
{
	uint16_t clientidlen = strlen(broker->clientid);
	uint16_t payload_len = 0;

	uint8_t clientid_payload[clientidlen+2];
	memset(clientid_payload, 0, sizeof(clientid_payload));
	clientid_payload[0] = clientidlen>>8;
	clientid_payload[1] = clientidlen&0xFF;
	memcpy(clientid_payload+2, broker->clientid, clientidlen);

	payload_len += sizeof(clientid_payload);

	// Variable header
	uint8_t var_header[] = {
		0x00,0x06,0x4d,0x51,0x49,0x73,0x64,0x70, // Protocol name: MQIsdp
		0x03, // Protocol version
		0x02, // Connect flags (0x02 = clear session)
		broker->alive>>8, broker->alive&0xFF, // Keep alive
	};

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_CONNECT, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+payload_len // Remaining length
	};

	uint16_t offset = 0;
	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+payload_len];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	offset += sizeof(fixed_header);
	memcpy(packet+offset, var_header, sizeof(var_header));
	offset += sizeof(var_header);
	memcpy(packet+offset, clientid_payload, sizeof(clientid_payload));
	offset += sizeof(clientid_payload);

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

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
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

	// Set connected flag
	broker->connected = 0;

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
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

	return 1;
}

int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, const char *msg, uint8_t retain)
{
	if(!broker->connected)
		return 0;

	uint16_t topiclen = strlen(topic);
	uint16_t msglen = strlen(msg);

	// Variable header
	uint8_t var_header[topiclen+2]; // Topic size (2 bytes), utf-encoded topic
	memset(var_header, 0, sizeof(var_header));
	var_header[0] = topiclen>>8;
	var_header[1] = topiclen&0xFF;
	memcpy(var_header+2, topic, topiclen);

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_PUBLISH, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+msglen // Remaining length
	};
	if(retain)
		fixed_header[0] |= MQTT_RETAIN_FLAG;

	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+msglen];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), msg, msglen);

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

	return 1;
}

int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic)
{
	if(!broker->connected)
		return 0;

	uint16_t topiclen = strlen(topic);

	// Variable header
	uint8_t var_header[] = {0x00, 0x0a}; // Message ID

	// utf topic
	uint8_t utf_topic[topiclen+3]; // Topic size (2 bytes), utf-encoded topic, QoS byte
	memset(utf_topic, 0, sizeof(utf_topic));
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_SUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+sizeof(utf_topic)
	};

	uint8_t packet[sizeof(var_header)+sizeof(fixed_header)+sizeof(utf_topic)];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, sizeof(utf_topic));

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

	return 1;
}

int mqtt_unsubscribe(mqtt_broker_handle_t *broker, const char *topic)
{
	if(!broker->connected)
		return 0;

	uint16_t topiclen = strlen(topic);

	// Variable header
	uint8_t var_header[] = {0x00, 0x0a}; // Message ID

	// utf topic
	uint8_t utf_topic[topiclen+2]; // Topic size (2 bytes), utf-encoded topic
	memset(utf_topic, 0, sizeof(utf_topic));
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_UNSUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+sizeof(utf_topic)
	};

	uint8_t packet[sizeof(var_header)+sizeof(fixed_header)+sizeof(utf_topic)];
	memset(packet, 0, sizeof(packet));
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, sizeof(utf_topic));

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet))
		return -1;

	return 1;
}
