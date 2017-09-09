# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/esp32/include
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/esp32 -lesp32 $(PROJECT_PATH)/components/esp32/libhal.a -L$(PROJECT_PATH)/components/esp32/lib -lcore -lrtc -lphy -lcoexist -lnet80211 -lpp -lwpa -lsmartconfig -lcoexist -lwps -lwpa2 -L $(PROJECT_PATH)/components/esp32/ld -T esp32_out.ld -u ld_include_panic_highint_hdl -T esp32.common.ld -T esp32.rom.ld -T esp32.peripherals.ld
COMPONENT_LINKER_DEPS += $(PROJECT_PATH)/components/esp32/lib/libcore.a $(PROJECT_PATH)/components/esp32/lib/librtc.a $(PROJECT_PATH)/components/esp32/lib/libphy.a $(PROJECT_PATH)/components/esp32/lib/libcoexist.a $(PROJECT_PATH)/components/esp32/lib/libnet80211.a $(PROJECT_PATH)/components/esp32/lib/libpp.a $(PROJECT_PATH)/components/esp32/lib/libwpa.a $(PROJECT_PATH)/components/esp32/lib/libsmartconfig.a $(PROJECT_PATH)/components/esp32/lib/libcoexist.a $(PROJECT_PATH)/components/esp32/lib/libwps.a $(PROJECT_PATH)/components/esp32/lib/libwpa2.a $(PROJECT_PATH)/components/esp32/ld/esp32.common.ld $(PROJECT_PATH)/components/esp32/ld/esp32.rom.ld $(PROJECT_PATH)/components/esp32/ld/esp32.peripherals.ld
COMPONENT_SUBMODULES += $(PROJECT_PATH)/components/esp32/lib
COMPONENT_LIBRARIES += esp32
esp32-build: 
