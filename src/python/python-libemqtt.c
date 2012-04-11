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
#include <python-mqtt_packet.h>



// Module definition
#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

PyMODINIT_FUNC initlibemqtt(void)
{
	PyObject* m;

	MqttType.tp_new = PyType_GenericNew;
	if(PyType_Ready(&MqttType) < 0)
		return;

	MqttPacketType.tp_new = PyType_GenericNew;
	if(PyType_Ready(&MqttPacketType) < 0)
		return;

	m = Py_InitModule3("libemqtt", NULL, "Embedded MQTT library.");

	Py_INCREF(&MqttType);
	PyModule_AddObject(m, "Mqtt", (PyObject*)&MqttType);

	Py_INCREF(&MqttPacketType);
	PyModule_AddObject(m, "MqttPacket", (PyObject*)&MqttPacketType);

	PyModule_AddIntConstant(m, "CONNECT", MQTT_MSG_CONNECT);
	PyModule_AddIntConstant(m, "CONNACK", MQTT_MSG_CONNACK);
	PyModule_AddIntConstant(m, "PUBLISH", MQTT_MSG_PUBLISH);
	PyModule_AddIntConstant(m, "PUBACK", MQTT_MSG_PUBACK);
	PyModule_AddIntConstant(m, "PUBREC", MQTT_MSG_PUBREC);
	PyModule_AddIntConstant(m, "PUBREL", MQTT_MSG_PUBREL);
	PyModule_AddIntConstant(m, "PUBCOMP", MQTT_MSG_PUBCOMP);
	PyModule_AddIntConstant(m, "SUBSCRIBE", MQTT_MSG_SUBSCRIBE);
	PyModule_AddIntConstant(m, "SUBACK", MQTT_MSG_SUBACK);
	PyModule_AddIntConstant(m, "UNSUBSCRIBE", MQTT_MSG_UNSUBSCRIBE);
	PyModule_AddIntConstant(m, "UNSUBACK", MQTT_MSG_UNSUBACK);
	PyModule_AddIntConstant(m, "PINGREQ", MQTT_MSG_PINGREQ);
	PyModule_AddIntConstant(m, "PINGRESP", MQTT_MSG_PINGRESP);
	PyModule_AddIntConstant(m, "DISCONNECT", MQTT_MSG_DISCONNECT);

	PyModule_AddObject(m, "ConnectionError", PyErr_NewException("libemqtt.Mqtt", NULL, NULL));
}

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
	PyModule_AddObject(mod, "Mqtt", (PyObject*)&MqttType);

	 return mod;
}
*/
