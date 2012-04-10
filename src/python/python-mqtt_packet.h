/*
 * This file is part of python-emqtt.
 *
 * python-emqtt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * python-emqtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with python-emqtt.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#ifndef __LIBEMQTT_PYTHON_MQTT_PACKET_H__
#define __LIBEMQTT_PYTHON_MQTT_PACKET_H__

#include <Python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD
	PyObject* data;
} MqttPacket;

extern PyTypeObject MqttPacketType;

#endif // __LIBEMQTT_PYTHON_MQTT_PACKET_H__
