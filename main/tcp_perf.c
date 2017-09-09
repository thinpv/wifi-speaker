/* tcp_perf Example

 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
 */

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"

#include "tcp_perf.h"

#include "wav.h"
#include "audio_player.h"
#include "audio_render.h"
#include "data_buffer.h"

#define TAG "tcp_perf:"

#define SIZE_OF_COMMAND		1
#define SIZE_OF_LENGTH		4
#define SIZE_OF_DATA		10240
#define NUMBER_BUFFER		20

typedef enum {
	UN_KNOW,
	SEND_HEADER,
	SEND_DATA,
	SEND_ACK
}TCP_COMMAND;

typedef struct _data_buffer {
	char buffer[SIZE_OF_DATA];
	int size;
} data_buffer;

WAVEFORMATEX wfx;

/* FreeRTOS event group to signal when we are connected to wifi */
EventGroupHandle_t tcp_event_group;

/*socket*/
static int server_socket = 0;
static struct sockaddr_in server_addr;
static struct sockaddr_in client_addr;
static unsigned int socklen = sizeof(client_addr);
static int connect_socket = 0;

static void audio_thread(void *pvParameters);
static void prcess_character(void *pvParameters);
extern void error_handle();

static esp_err_t event_handler(void *ctx, system_event_t *event) {
	switch (event->event_id) {
	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		esp_wifi_connect();
		xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(TAG, "got ip:%s\n",
				ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
				MAC2STR(event->event_info.sta_connected.mac),
				event->event_info.sta_connected.aid);
		xEventGroupSetBits(tcp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
				MAC2STR(event->event_info.sta_disconnected.mac),
				event->event_info.sta_disconnected.aid);
		xEventGroupClearBits(tcp_event_group, WIFI_CONNECTED_BIT);
		break;
	default:
		break;
	}
	return ESP_OK;
}

static int send_ack(int client)
{
	char buffer[] = {SEND_ACK, 0};
	if(send(client, buffer, sizeof(buffer), 0) != sizeof(buffer))
		return 0;
	return 1;
}

//use this esp32 as a tcp server. return ESP_OK:success ESP_FAIL:error
void listen_tcp_server(void *pvParameters) {
	ESP_LOGI(TAG, "server socket....port=%d\n", EXAMPLE_DEFAULT_PORT);
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		show_socket_error_reason("create_server", server_socket);
		error_handle();
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(server_socket, (struct sockaddr * )&server_addr,
			sizeof(server_addr)) < 0) {
		show_socket_error_reason("bind_server", server_socket);
		close(server_socket);
		error_handle();
	}

	if (listen(server_socket, 5) < 0) {
		show_socket_error_reason("listen_server", server_socket);
		close(server_socket);
		error_handle();
	}
	ESP_LOGI(TAG, "tcp connection established!");

	while (1) {
		int client = accept(server_socket, (struct sockaddr * )&client_addr,
				&socklen);
		if (client < 0) {
			show_socket_error_reason("accept_server", client);
			close(server_socket);
			ESP_LOGE(TAG, "close server socket");
			error_handle();
		}
		xTaskCreate(&audio_thread, "audio_thread", 4096, &client, 20, NULL);
		ESP_LOGI(TAG, "have a new connection: %d!", client);
	}
	vTaskDelete(NULL);
}

static void audio_thread(void *pvParameters) {
	int client = (int) *((int*) pvParameters);
	char buffer[1024];
	int render_status = 0;
	int length_receive;
	while(1)
	{
		length_receive = recv(client, (char*)&buffer, sizeof(buffer), 0);
		if(length_receive == -1){
			ESP_LOGE(TAG, "socket client %d close in get command", client);
			goto exit;
		}
		//audio_stream_consumer(buffer, length_receive);
		buffer_write_data(buffer, length_receive);
		if(getBufferPercent() > 50 && render_status == 0)
		//if(render_status == 0)
		{
			render_status = 1;
			ESP_LOGI(TAG, "creat prcess_character thread");
			xTaskCreate(&prcess_character, "prcess_character", 50000, NULL, 23, NULL);
			ESP_LOGI(TAG, "creat prcess_character thread done");
		}
	}
	exit:
	close(client);
	ESP_LOGI(TAG, "closed thread %d", client);
	vTaskDelete(NULL);
}

static void prcess_character(void *pvParameters) {
	int read_byte = 0;
	int buffer[4096];
	TCP_COMMAND tcp_command;
	DWORD length_data;
	wfx.nChannels = 2;
	wfx.wBitsPerSample = 16;
	wfx.nSamplesPerSec = 48000;
	while (1) {
		//count = 0;
		read_byte = buffer_read_data((char*)&tcp_command, sizeof(TCP_COMMAND));
		switch(tcp_command){
		case UN_KNOW:
			break;
		case SEND_HEADER:
			read_byte = buffer_read_data((char*)&length_data, sizeof(DWORD));
			if (read_byte != sizeof(DWORD))
				ESP_LOGE(TAG, "get length header error");
			if (length_data != sizeof(WAVEFORMATEX))
				ESP_LOGE(TAG, "header data error");
			read_byte = buffer_read_data((char*) &wfx, length_data);
			if (read_byte != length_data)
				ESP_LOGE(TAG, "get header error");
			/*ESP_LOGI(TAG, "FormatTag:       %d", (int )wfx.wFormatTag);
			ESP_LOGI(TAG, "Channels:        %d", (int )wfx.nChannels);
			ESP_LOGI(TAG, "SamplesPerSec:   %d", (int )wfx.nSamplesPerSec);
			ESP_LOGI(TAG, "AvgBytesPerSec:  %d", (int )wfx.nAvgBytesPerSec);
			ESP_LOGI(TAG, "BlockAlign:      %d", (int )wfx.nBlockAlign);
			ESP_LOGI(TAG, "BitsPerSample:   %d", (int )wfx.wBitsPerSample);
			ESP_LOGI(TAG, "Size:            %d", (int )wfx.cbSize);*/
			break;
		case SEND_DATA:
			read_byte = buffer_read_data((char*) &length_data, sizeof(DWORD));
			if (read_byte != sizeof(DWORD))
				ESP_LOGE(TAG, "get length data error read_byte:%d", read_byte);
			if (read_byte > 4096)
				ESP_LOGE(TAG, "data length very large read_byte: %d", read_byte);
			if(length_data<0)
				ESP_LOGE(TAG, "data length error <0: %d", length_data);
			//ESP_LOGE(TAG, "length_data: %d", (int)length_data);
			read_byte = buffer_read_data((char*) buffer, (int)length_data);
			if (read_byte != length_data)
				ESP_LOGE(TAG, "get data error read_byte: %d, length_data: %d", read_byte, (int)length_data);
			render_samples(buffer, length_data, &wfx);
			break;
		case SEND_ACK:
			break;
		default:
			break;
		}
	}
	vTaskDelete(NULL);
}

//wifi_init_sta
void wifi_init_sta() {
	tcp_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	wifi_config_t wifi_config = { .sta = { .ssid = "DCNA", //EXAMPLE_DEFAULT_SSID,
			.password = "1000000000"        //EXAMPLE_DEFAULT_PWD
			}, };

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s \n",
			"DCNA"/*EXAMPLE_DEFAULT_SSID*/,"1000000000"/*EXAMPLE_DEFAULT_PWD*/);
}
//wifi_init_softap
void wifi_init_softap() {
	tcp_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
	;
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	wifi_config_t wifi_config = { .ap = { .ssid = EXAMPLE_DEFAULT_SSID,
			.ssid_len = 0, .max_connection = EXAMPLE_MAX_STA_CONN, .password =
					EXAMPLE_DEFAULT_PWD, .authmode = WIFI_AUTH_WPA_WPA2_PSK }, };
	if (strlen(EXAMPLE_DEFAULT_PWD) == 0) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s \n",
			EXAMPLE_DEFAULT_SSID, EXAMPLE_DEFAULT_PWD);
}

int get_socket_error_code(int socket) {
	int result;
	u32_t optlen = sizeof(int);
	int err = getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen);
	if (err == -1) {
		ESP_LOGE(TAG, "getsockopt failed:%s", strerror(err));
		return -1;
	}
	return result;
}

int show_socket_error_reason(const char *str, int socket) {
	int err = get_socket_error_code(socket);

	if (err != 0) {
		ESP_LOGW(TAG, "%s socket error %d %s", str, err, strerror(err));
	}

	return err;
}

int check_working_socket() {
	int ret;
#if EXAMPLE_ESP_TCP_MODE_SERVER
	ESP_LOGD(TAG, "check server_socket");
	ret = get_socket_error_code(server_socket);
	if (ret != 0) {
		ESP_LOGW(TAG, "server socket error %d %s", ret, strerror(ret));
	}
	if (ret == ECONNRESET) {
		return ret;
	}
#endif
	ESP_LOGD(TAG, "check connect_socket");
	ret = get_socket_error_code(connect_socket);
	if (ret != 0) {
		ESP_LOGW(TAG, "connect socket error %d %s", ret, strerror(ret));
	}
	if (ret != 0) {
		return ret;
	}
	return 0;
}

void close_socket() {
	//close(connect_socket);
	close(server_socket);
}

