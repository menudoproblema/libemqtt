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

#ifndef __LIBEMQTT_H__
#define __LIBEMQTT_H__

#include <stdint.h>

#ifndef MQTT_CONF_USERNAME_LENGTH
	#define MQTT_CONF_USERNAME_LENGTH 13 // Recommended by MQTT Specification (12 + '\0')
#endif

#ifndef MQTT_CONF_PASSWORD_LENGTH
	#define MQTT_CONF_PASSWORD_LENGTH 13 // Recommended by MQTT Specification (12 + '\0')
#endif


#define MQTT_MSG_CONNECT       1<<4
#define MQTT_MSG_CONNACK       2<<4
#define MQTT_MSG_PUBLISH       3<<4
#define MQTT_MSG_PUBACK        4<<4
#define MQTT_MSG_PUBREC        5<<4
#define MQTT_MSG_PUBREL        6<<4
#define MQTT_MSG_PUBCOMP       7<<4
#define MQTT_MSG_SUBSCRIBE     8<<4
#define MQTT_MSG_SUBACK        9<<4
#define MQTT_MSG_UNSUBSCRIBE  10<<4
#define MQTT_MSG_UNSUBACK     11<<4
#define MQTT_MSG_PINGREQ      12<<4
#define MQTT_MSG_PINGRESP     13<<4
#define MQTT_MSG_DISCONNECT   14<<4


/** Extract the message type from buffer.
 * @param buffer Pointer to the packet.
 *
 * @return Message Type byte.
 */
#define MQTTMessageType(buffer) ( *buffer & 0xF0 )

/** Extract the message id from buffer.
 * @param buffer Pointer to the packet.
 * @param id This variable will store the message id. If the message has
 *        not Message ID, it will set to 0.
 *
 * @return None.
 */
#define MQTTMessageId(buffer, id) {										\
	uint8_t type = MQTTMessageType(buffer);								\
	uint8_t qos = MQTTMessageQos(buffer);								\
	id = 0;																\
	if(type >= MQTT_MSG_PUBLISH && type <= MQTT_MSG_UNSUBACK)			\
	{																	\
		if(type == MQTT_MSG_PUBLISH)									\
		{																\
			if(qos != 0)												\
			{															\
				uint8_t offset = *(buffer+2)<<8; offset |= *(buffer+3);	\
				offset += 4; /* Fixed header + Topic utf8-encoded */	\
				/* Fixed header length + Topic utf8-encoded */			\
				id = *(buffer+offset)<<8; id |= *(buffer+offset+1);		\
			}															\
		}																\
		else															\
			/* Fixed header length */									\
			id = *(buffer+2)<<8; id |= *(buffer+3);						\
	}																	\
}

/** Extract the message QoS level.
 * @param buffer Pointer to the packet.
 *
 * @return QoS Level (0, 1 or 2).
 */
#define MQTTMessageQos(buffer) ( (*buffer & 0x06) >> 1 )

/** Extract the topic from a publish message.
 * If the packet is not a publish message, this macro has no effect.
 * @param buffer Pointer to the packet.
 * @param topic Buffer where topic will be copied.
 * @param len This variable will store the length of the topic.
 *
 * @return None.
 */
#define MQTTPublishMessageTopic(buffer, topic, len) {					\
	if(MQTTMessageType(buffer) == MQTT_MSG_PUBLISH)						\
	{																	\
		len = *(buffer+2)<<8; len |= *(buffer+3);						\
		memcpy(topic, (buffer + 4), len);								\
		*(topic + len) = 0;												\
	}																	\
}



typedef struct {
	void* socket_info;
	int (*send)(void* socket_info, const void* buf, unsigned int count);
	// Connection info
	char clientid[24];
	// Auth fields
	char username[MQTT_CONF_USERNAME_LENGTH];
	char password[MQTT_CONF_PASSWORD_LENGTH];
	// Will topic
	uint8_t will_retain;
	uint8_t will_qos;
	uint8_t clean_session;
	// Management fields
	uint16_t seq;
	uint16_t alive;
} mqtt_broker_handle_t;



/** Initialize the information to connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 * @param clientid A string that identifies the client id.
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_init(mqtt_broker_handle_t* broker, const char* clientid);

/** Enable the authentication to connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 * @param username A string that contains the username.
 * @param password A string that contains the password.
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_init_auth(mqtt_broker_handle_t* broker, const char* username, const char* password);

/** Set the keep alive timer.
 * @param broker Data structure that contains the connection information with the broker.
 * @param alive Keep aliver timer value (in seconds).
 *
 * @note Only has effect before to call mqtt_connect
 */
void mqtt_set_alive(mqtt_broker_handle_t* broker, uint16_t alive);

/** Connect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_connect(mqtt_broker_handle_t* broker);

/** Disconnect to the broker.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @note The socket must also be closed.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_disconnect(mqtt_broker_handle_t* broker);

/** Publish a message on a topic. This message will be published with 0 Qos level.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param msg The message.
 * @param retain Enable or disable the Retain flag (values: 0 or 1).
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_publish(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain);

/** Publish a message on a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param msg The message.
 * @param retain Enable or disable the Retain flag (values: 0 or 1).
 * @param qos Quality of Service (values: 0, 1 or 2)
 * @param message_id Variable that will store the Message ID, if the pointer is not NULL.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id);

/** Send a PUBREL message. It's used for PUBLISH message with 2 QoS level.
 * @param broker Data structure that contains the connection information with the broker.
 * @param message_id Message ID
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_pubrel(mqtt_broker_handle_t* broker, uint16_t message_id);

/** Subscribe to a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 * @param message_id Variable that will store the Message ID, if the pointer is not NULL.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);

/** Unsubscribe from a topic.
 * @param broker Data structure that contains the connection information with the broker.
 * @param topic The topic name.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic);

/** Make a ping.
 * @param broker Data structure that contains the connection information with the broker.
 *
 * @retval  1 On success.
 * @retval  0 On connection error.
 * @retval -1 On IO error.
 */
int mqtt_ping(mqtt_broker_handle_t* broker);


#endif // __LIBEMQTT_H__
