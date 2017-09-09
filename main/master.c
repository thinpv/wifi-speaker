#include <errno.h>

#include "audio_render.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "spiram_fifo.h"
#include "esp_system.h"

#include "tcp_perf.h"
#include "data_buffer.h"

#define TAG "main"

static void init_hardware()
{
    nvs_flash_init();

    // init UI
    // ui_init(GPIO_NUM_32);

    //Initialize the SPI RAM chip communications and see if it actually retains some bytes. If it
    //doesn't, warn user.
    if (!spiRamFifoInit()) {
        printf("\n\nSPI RAM chip fail!\n");
        while(1);
    }

    ESP_LOGI(TAG, "hardware initialized");
}

void app_main(void)
{
	//init_hardware();
	init_data_buffer();

	ESP_LOGI(TAG, "EXAMPLE_ESP_WIFI_MODE_AP");
	//wifi_init_softap();
	wifi_init_sta();

	renderer_init(create_renderer_config());

	//listen_tcp_server();
	xTaskCreate(&listen_tcp_server, "listen_tcp_server", 4096, NULL, 20, NULL);
	//xTaskCreate(&prcess_character, "prcess_character", 8192, NULL, 20, NULL);
	//close_socket();
	while(1){
		ESP_LOGI(TAG, "RAM left %d", esp_get_free_heap_size());
		vTaskDelay(2000 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void error_handle()
{
	ESP_LOGI(TAG, "error_handle");
	while(1);
}
