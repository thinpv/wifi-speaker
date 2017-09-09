# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/coap/port/include $(PROJECT_PATH)/components/coap/port/include/coap $(PROJECT_PATH)/components/coap/libcoap/include $(PROJECT_PATH)/components/coap/libcoap/include/coap
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/coap -lcoap
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += $(PROJECT_PATH)/components/coap/libcoap
COMPONENT_LIBRARIES += coap
coap-build: 
