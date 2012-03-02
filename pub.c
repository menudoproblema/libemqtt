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

#include <stdio.h>
#include <string.h>
#include <libemqtt.h>


int main(int argc, char **argv) {
	int result;
	uint16_t alive = 2; // Alive for 2 seconds
	char msg[128];
	mqtt_broker_handle_t broker;

	broker.port = 1883;
	strcpy(broker.hostname, "127.0.0.1");
	strcpy(broker.clientid, "libemqtt pub");

	result = mqtt_connect(&broker, alive);
	printf("Connect: %d\n", result);

	mqtt_publish(&broker, "hello/emqtt", msg);

	mqtt_disconnect(&broker);
	return 0;
}

