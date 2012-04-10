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

#include <Python.h>
#include <python-mqtt.h>



int send_packet(void* socket_info, const void* buf, unsigned int count)
{
	Mqtt* self = (Mqtt *)socket_info;
	PyObject* packet = PyString_FromStringAndSize(buf, count);
	PyObject* result = PyObject_CallMethod(self->socket, "send", "S", packet);
	int bytes = PyInt_AsLong(result);
	Py_DECREF(packet);
	return bytes;
}



static PyObject*
Mqtt_init(Mqtt* self, PyObject* args, PyObject* kwargs)
{
	static char* kwlist[] = {"socket", "clientid", "username", "password", NULL};

	char* clientid = "python-emqtt";
	char* username = NULL;
	char* password = NULL;

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|sss", kwlist, &self->socket, &clientid, &username, &password))
	{
		// TODO: Exception
		return NULL;
	}

	mqtt_init(&self->broker, clientid);
	mqtt_init_auth(&self->broker, username, password);

	self->broker.socket_info = (void*)self;
	self->broker.send = send_packet;

	self->connected = 0;

	return Py_BuildValue("");
}

static PyObject*
Mqtt_connect(Mqtt* self)
{
	if(self->connected <= 0) // Prevent reconnect
	{
		self->connected = mqtt_connect(&self->broker);
		return Py_BuildValue("b", self->connected);
	}
	return Py_BuildValue("b", Py_False);
}

static PyObject*
Mqtt_disconnect(Mqtt* self)
{
	if(self->connected > 0)
	{
		int result = mqtt_disconnect(&self->broker);
		self->connected = 0;
		return Py_BuildValue("i", result);
	}
	return Py_BuildValue("");
}

static PyObject*
Mqtt_ping(Mqtt* self)
{
	if(self->connected <= 0) // Not connected
	{
		// TODO: Exception
		return NULL;
	}

	return Py_BuildValue("i", mqtt_ping(&self->broker));
}

static PyObject*
Mqtt_publish(Mqtt* self, PyObject* args, PyObject* kwargs)
{
	static char* kwlist[] = {"topic", "message", "retain", "qos", NULL};

	char* topic = NULL;
	char* message = NULL;
	PyObject* retain = Py_False;
	int qos = 0;
	uint16_t message_id;

	int result;

	if(self->connected <= 0) // Not connected
	{
		// TODO: Exception
		return NULL;
	}

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "ss|Oi", kwlist, &topic, &message, &retain, &qos))
	{
		// TODO: Exception
		return NULL;
	}

	if(!PyBool_Check(retain))
	{
		// TODO: Exception
	}

	if(qos < 0 || qos > 2)
	{
		// TODO: Exception
	}

	result = mqtt_publish_with_qos(&self->broker, topic, message, PyInt_AsLong(retain), qos, &message_id);
	if(result > 0)
		result = message_id;

	return Py_BuildValue("i", result);
}

static PyObject*
Mqtt_pubrel(Mqtt* self, PyObject* args, PyObject* kwargs)
{
	static char* kwlist[] = {"msgid", NULL};

	int message_id;

	if(self->connected <= 0) // Not connected
	{
		// TODO: Exception
		return NULL;
	}

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &message_id))
	{
		// TODO: Exception
		return NULL;
	}

	return Py_BuildValue("i", mqtt_pubrel(&self->broker, message_id));
}

static PyObject*
Mqtt_subscribe(Mqtt* self, PyObject* args, PyObject* kwargs)
{
	if(self->connected <= 0) // Not connected
	{
		// TODO: Exception
		return NULL;
	}

	static char* kwlist[] = {"topic", NULL};

	int result;
	char* topic = NULL;
	uint16_t msg_id;

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &topic))
	{
		// TODO: Exception
		return NULL;
	}

	result = mqtt_subscribe(&self->broker, topic, &msg_id);
	// TODO: if result < 0: raise Exception

	return Py_BuildValue("i", msg_id);
}

static void
Mqtt_dealloc(Mqtt* self)
{
	if(self->connected > 0)
	{
		Mqtt_disconnect(self);
	}
	self->ob_type->tp_free((PyObject*)self);
}

static PyMemberDef Mqtt_members[] = {
	{ NULL }
};

static PyMethodDef Mqtt_methods[] = {
	{ "connect", (PyCFunction) Mqtt_connect, METH_NOARGS, "MQTT connect." },
	{ "disconnect", (PyCFunction) Mqtt_disconnect, METH_NOARGS, "MQTT disconnect." },
	{ "ping", (PyCFunction) Mqtt_ping, METH_NOARGS, "MQTT ping." },
	{ "publish", (PyCFunction) Mqtt_publish, METH_KEYWORDS, "MQTT publish." },
	{ "pubrel", (PyCFunction) Mqtt_pubrel, METH_KEYWORDS, "MQTT pubrel." },
	{ "subscribe", (PyCFunction) Mqtt_subscribe, METH_KEYWORDS, "MQTT subscribe." },

	{ NULL }
};

PyTypeObject MqttType = {
	PyObject_HEAD_INIT(NULL)
	0,											/* ob_size */
	"libemqtt.Mqtt",							/* tp_name */
	sizeof(Mqtt),								/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)Mqtt_dealloc,					/* tp_dealloc */
	0,											/* tp_print*/
	0,											/* tp_getattr */
	0,											/* tp_setattr */
	0,											/* tp_compare */
	0,											/* tp_repr */
	0,											/* tp_as_number */
	0,											/* tp_as_sequence */
	0,											/* tp_as_mapping */
	0,											/* tp_hash */
	0,											/* tp_call */
	0,											/* tp_str */
	0,											/* tp_getattro */
	0,											/* tp_setattro */
	0,											/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/* tp_flags*/
	"Mqtt objects",								/* tp_doc */
	0,											/* tp_traverse */
	0,											/* tp_clear */
	0,											/* tp_richcompare */
	0,											/* tp_weaklistoffset */
	0,											/* tp_iter */
	0,											/* tp_iternext */
	Mqtt_methods,								/* tp_methods */
	Mqtt_members,								/* tp_members */
	0,											/* tp_getset */
	0,											/* tp_base */
	0,											/* tp_dict */
	0,											/* tp_descr_get */
	0,											/* tp_descr_set */
	0,											/* tp_dictoffset */
	(initproc)Mqtt_init,						/* tp_init */
	0,											/* tp_alloc */
	0,											/* tp_new */
};
