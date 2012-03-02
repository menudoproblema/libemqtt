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
#include <unistd.h>
#include <libemqtt.h>


int main() {
	mqtt_broker_handle_t broker;
	int result, count = 0;

	mqtt_broker_init(&broker, "192.168.10.40", 1883, "libemqtt sub");

	result = mqtt_connect(&broker);
	printf("Connect: %d\n", result);

	result = mqtt_subscribe(&broker, "hello/emqtt", NULL);
	printf("Subscribe: %d\n", result);

	// Keep alive for 10 pings
	while(count < 10){
		printf("Ping: %d\n", count);
		count++;
		mqtt_ping(&broker);
		sleep(1);
	}

	mqtt_disconnect(&broker);

	return 0;
}
