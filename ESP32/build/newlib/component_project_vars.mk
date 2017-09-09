# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/newlib/platform_include $(PROJECT_PATH)/components/newlib/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/newlib -lnewlib $(PROJECT_PATH)/components/newlib/lib/libc.a $(PROJECT_PATH)/components/newlib/lib/libm.a
COMPONENT_LINKER_DEPS += $(PROJECT_PATH)/components/newlib/lib/libc.a $(PROJECT_PATH)/components/newlib/lib/libm.a
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += newlib
newlib-build: 
