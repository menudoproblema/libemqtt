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

 * You should have received a copy of the GNU General Public License
 * along with python-emqtt.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#include <Python.h>
#include <structmember.h>
#include <libemqtt.h>

typedef struct {
	PyObject_HEAD
	PyObject *socket;
	short connected; // < 1 means not connected
	mqtt_broker_handle_t broker;
} MqttBroker;



int send_packet(void *socket_info, const void *buf, unsigned int count)
{
	MqttBroker *self = (MqttBroker *)socket_info;
	PyObject *packet = PyString_FromStringAndSize(buf, count);
	PyObject *result = PyObject_CallMethod(self->socket, "send", "S", packet);
	int bytes = PyInt_AsLong(result);
	Py_DECREF(packet);
	return bytes;
}



PyObject *Mqtt_init(MqttBroker *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {"socket", "clientid", "username", "password", NULL};

	char *clientid = "python-emqtt";
	char *username = NULL;
	char *password = NULL;

	if(!PyArg_ParseTupleAndKeywords(args, kwargs, "O|sss", kwlist, &self->socket, &clientid, &username, &password))
	{
		// TODO: Exception
		return NULL;
	}

	mqtt_init(&self->broker, clientid);
	mqtt_init_auth(&self->broker, username, password);

	self->broker.socket_info = (void *)self;
	self->broker.send = send_packet;

	self->connected = 0;

	return Py_BuildValue("");
}

PyObject *Mqtt_connect(MqttBroker *self)
{
	printf("Connect\n");
	if(self->connected <= 0) // Prevent reconnect
	{
		self->connected = mqtt_connect(&self->broker);
		return Py_BuildValue("i", self->connected);
	}
	return Py_BuildValue("");
}

PyObject *Mqtt_disconnect(MqttBroker *self)
{
	printf("Disconnect\n");
	if(self->connected > 0)
	{
		int result = mqtt_disconnect(&self->broker);
		self->connected = 0;
		return Py_BuildValue("i", result);
	}
	return Py_BuildValue("");
}

void Mqtt_dealloc(MqttBroker *self)
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

	{ NULL }
};

static PyTypeObject MqttType = {
	PyObject_HEAD_INIT(NULL)
	0,											/* ob_size */
	"emqtt.Mqtt",								/* tp_name */
	sizeof(MqttBroker),							/* tp_basicsize */
	0,											/* tp_itemsize */
	(destructor)Mqtt_dealloc,					/* tp_dealloc */
	0,											/* tp_print */
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

// Module definition
/*
static PyModuleDef moduledef = {
	 PyModuleDef_HEAD_INIT,
	 "emqtt",
	 "python-emqtt",
	 -1,
	 NULL,		 // methods
	 NULL,
	 NULL,		 // traverse
	 NULL,		 // clear
	 NULL
};
*/
#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initclient(void)
{
    PyObject* m;

    MqttType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&MqttType) < 0)
        return;

    m = Py_InitModule3("emqtt.client", Mqtt_methods, "Embedded MQTT client.");

    Py_INCREF(&MqttType);
    PyModule_AddObject(m, "Mqtt", (PyObject *)&MqttType);
}

/*
PyInit_emqtt(void)
{
	PyObject* mod; = (PyModule_Create(&moduledef);
	if (mod == NULL) {
		return NULL;
	}

	MqttType.tp_new = PyType_GenericNew;
	if (PyType_Ready(&MqttType) < 0) {
		Py_DECREF(mod);
		return NULL;
	}

	Py_INCREF(&MqttType);
	PyModule_AddObject(mod, "Mqtt", (PyObject *)&MqttType);

	 return mod;
}
*/
