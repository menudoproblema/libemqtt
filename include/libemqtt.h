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

/** Extract message type from buffer. Details follow
 *  here.
 */
#define MQTTMessageType(buffer, type) ( (*buffer&0xF0) & type )

#define MQTTMessageID(buffer, id) { id = 0; id = *(buffer)<<8; id |= *(buffer+1); }


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



/**
 * @param broker
 * @param clientid
 *
 * Note: Only has effect before to call mqtt_connect
 */
void mqtt_init(mqtt_broker_handle_t* broker, const char* clientid);

/**
 * @param broker
 * @param username
 * @param password
 *
 * Note: Only has effect before to call mqtt_connect
 */
void mqtt_init_auth(mqtt_broker_handle_t* broker, const char* username, const char* password);

/**
 * @param broker
 * @param alive
 *
 * Note: Only has effect before to call mqtt_connect
 */
void mqtt_set_alive(mqtt_broker_handle_t* broker, uint16_t alive);

/**
 * @param broker
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_connect(mqtt_broker_handle_t* broker);

/**
 * @param broker
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_disconnect(mqtt_broker_handle_t* broker);

/**
 * @param broker
 * @param topic
 * @param msg
 * @param retain
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_publish(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain);

/**
 * @param broker
 * @param topic
 * @param msg
 * @param retain
 * @param qos
 * @param message_id
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint8_t retain, uint8_t qos, uint16_t* message_id);

/**
 * @param broker
 * @param message_id
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_pubrel(mqtt_broker_handle_t* broker, uint16_t message_id);

/**
 * @param broker
 * @param topic
 * @param message_id
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id);

/**
 * @param broker
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_ping(mqtt_broker_handle_t* broker);

/**
 * @param broker
 * @param topic
 *
 * @return On success, 1 is returned. On connection error, 0 is returned.
 * On IO error, -1 is returned.
 */
int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic);


#endif // __LIBEMQTT_H__
