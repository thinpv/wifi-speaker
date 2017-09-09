/*
 * audio_player.c
 *
 *  Created on: 12.03.2017
 *      Author: michaelboeckling
 */

#include "audio_player.h"

#include <stdlib.h>

#include "audio_render.h"
#include "spiram_fifo.h"
#include "freertos/FreeRTOS.h"

#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "data_buffer.h"

#define TAG "audio_player"
#define PRIO_MAD configMAX_PRIORITIES - 2

static component_status_t player_status = UNINITIALIZED;

static int t;
static player_command_t command;
static player_command_t decoder_command;
static component_status_t decoder_status;

/* Writes bytes into the FIFO queue, starts decoder task if necessary. */
int audio_stream_consumer(const char *recv_buf, ssize_t bytes_read)
{
    // don't bother consuming bytes if stopped
    if(command == CMD_STOP) {
        decoder_command = CMD_STOP;
        command = CMD_NONE;
        return -1;
    }

    if (bytes_read > 0) {
    	//buffer_write_data(recv_buf, bytes_read);
        spiRamFifoWrite(recv_buf, bytes_read);
    }

    int bytes_in_buf = spiRamFifoFill();
    uint8_t fill_level = (bytes_in_buf * 100) / spiRamFifoLen();
    //uint8_t fill_level = getBufferPercent();

    // seems 4k is enough to prevent initial buffer underflow
    uint8_t min_fill_lvl = 50;//player->buffer_pref == BUF_PREF_FAST ? 20 : 90;
    bool enough_buffer = fill_level > min_fill_lvl;

    bool early_start = (bytes_in_buf > 1028);// && player->media_stream->eof);
	if (decoder_status != RUNNING && (enough_buffer /*|| early_start*/))
	{
		if (xTaskCreate(&render_audio, "render_audio", 10000, NULL, PRIO_MAD,
				NULL) != pdPASS)
		{
			ESP_LOGE(TAG, "ERROR creating decoder task! Out of memory?");
			return -1;
		} else {
			ESP_LOGI(TAG, "Start render_audio fill_level: %u%%", fill_level);
			decoder_status = RUNNING;
		};
	}

    t = (t + 1) & 255;
    if (t == 1) {
        ESP_LOGI(TAG, "Buffer fill %u%%, %d bytes", fill_level, bytes_in_buf);
    }

    return 0;
}

void audio_player_init()
{
    player_status = INITIALIZED;
}

void audio_player_destroy()
{
    //renderer_destroy();
    player_status = UNINITIALIZED;
}

void audio_player_start()
{
    //renderer_start();
    player_status = RUNNING;
}

void audio_player_stop()
{
    //renderer_stop();
    command = CMD_STOP;
    // player_status = STOPPED;
}

component_status_t get_player_status()
{
    return player_status;
}

