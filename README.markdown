# Libemqtt

This program is a fork of liblwmqtt developed by Filipe Varela.
You can find the original project on <http://code.google.com/p/liblwmqtt/>.

libemqtt aims to be an embedded C client library for the MQTT protocol. It also
provides a binding for Python.

## Status

Under development. DO NOT USE in a serious development. API is not closed, so it
could be changed.

I hope to release the first version in April.


## Compile

### C Library

> $ make

### Python binding

> $ make python


## Install

### C Library

### Python binding

> $ sudo ln -fs /home/user/libemqtt/client/libemqtt.so /usr/lib/python2.7/emqtt/libemqtt.so



# Limitations

* Can not subscribe to multiple topics in the same MQTT message.



# Additional

For debugging MQTT development it's possible to use
Wireshark-MQTT (https://github.com/menudoproblema/Wireshark-MQTT)
