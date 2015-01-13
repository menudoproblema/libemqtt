cmake_minimum_required(VERSION 2.8.4)
project(libemqtt)

include_directories(include)

set(SOURCE_FILES_SUB src/libemqtt.c client/sub.c include/libemqtt.h)
set(SOURCE_FILES_PUB src/libemqtt.c client/pub.c include/libemqtt.h)

add_executable(pub ${SOURCE_FILES_PUB})
add_executable(sub ${SOURCE_FILES_SUB})