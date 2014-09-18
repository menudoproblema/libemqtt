LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= mqttpub

LOCAL_SRC_FILES:= \
	client/pub.c \

LOCAL_LDLIBS +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
		-O3 
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := libemqtt

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= mqttsub

LOCAL_SRC_FILES:= \
	client/sub.c \

LOCAL_LDLIBS +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
		-O3 
									
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES := libemqtt

include $(BUILD_SHARED_LIBRARY)


# Define vars for library that will be build statically.
include $(CLEAR_VARS)
LOCAL_MODULE := libemqtt
LOCAL_C_INCLUDES += \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/include
LOCAL_SRC_FILES :=  \
	src/libemqtt.c
# Optional compiler flags.
LOCAL_LDLIBS  +=  -llog -lz -ldl -landroid	
LOCAL_CFLAGS  +=  -Wall \
		-O3 
include $(BUILD_STATIC_LIBRARY)