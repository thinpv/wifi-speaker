# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/libsodium/libsodium/src/libsodium/include $(PROJECT_PATH)/components/libsodium/port_include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/libsodium -llibsodium
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += $(PROJECT_PATH)/components/libsodium/libsodium
COMPONENT_LIBRARIES += libsodium
libsodium-build: 
