/*
 * audio_render.c
 *
 *  Created on: Aug 27, 2017
 *      Author: thind
 */

#include "audio_render.h"

#include <string.h>
#include "playerconfig.h"
//#include "esp_intr_alloc.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "spiram_fifo.h"

#define TAG "audio_render"

static renderer_config_t *renderer_instance = NULL;
static component_status_t renderer_status = UNINITIALIZED;
static QueueHandle_t i2s_event_queue;

extern WAVEFORMATEX wfx;

renderer_config_t *create_renderer_config()
{
    renderer_config_t *renderer_config = calloc(1, sizeof(renderer_config_t));
    renderer_config->bit_depth = I2S_BITS_PER_SAMPLE_16BIT;
    renderer_config->i2s_num = I2S_NUM_0;
    renderer_config->sample_rate = 44100;
    renderer_config->sample_rate_modifier = 1.0;
    renderer_config->output_mode = DAC_BUILT_IN;
    return renderer_config;
}

/* init renderer sink */
void renderer_init(renderer_config_t *config)
{
    // update global
    renderer_instance = config;
    renderer_status = INITIALIZED;

    ESP_LOGI(TAG, "init I2S mode %d, port %d, %d bit, %d Hz", config->output_mode, config->i2s_num, config->bit_depth, config->sample_rate);
    init_i2s(config);
    i2s_set_sample_rates(renderer_instance->i2s_num, renderer_instance->sample_rate);
    //if(config->output_mode == I2S_MERUS) {
    //    init_ma120(0x50); // setup ma120x0p and initial volume
    //}
}

void init_i2s(renderer_config_t *config)
{
    i2s_mode_t mode = I2S_MODE_MASTER | I2S_MODE_TX;
    i2s_comm_format_t comm_fmt = I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB;

    if(config->output_mode == DAC_BUILT_IN)
    {
        mode = mode | I2S_MODE_DAC_BUILT_IN;
        comm_fmt = I2S_COMM_FORMAT_I2S_MSB;
    }

    if(config->output_mode == PDM)
    {
        mode = mode | I2S_MODE_PDM;
    }

    /*
     * Allocate just enough to decode AAC+, which has huge frame sizes.
     *
     * Memory consumption formula:
     * (bits_per_sample / 8) * num_chan * dma_buf_count * dma_buf_len
     *
     * 16 bit: 32 * 256 = 8192 bytes
     * 32 bit: 32 * 256 = 16384 bytes
     */
    i2s_config_t i2s_config = {
            .mode = mode,          // Only TX
            .sample_rate = config->sample_rate,
            .bits_per_sample = config->bit_depth,
            .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,   // 2-channels
            .communication_format = comm_fmt,
            .dma_buf_count = 32,                            // number of buffers, 128 max.
            .dma_buf_len = 64,                          // size of each buffer
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1        // Interrupt level 1
    };

    i2s_pin_config_t pin_config = {
            .bck_io_num = GPIO_NUM_26,
            .ws_io_num = GPIO_NUM_25,
            .data_out_num = GPIO_NUM_22,
            .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(config->i2s_num, &i2s_config, 1, &i2s_event_queue);

    if((mode & I2S_MODE_DAC_BUILT_IN) || (mode & I2S_MODE_PDM))
    {
        i2s_set_pin(config->i2s_num, NULL);
        i2s_set_dac_mode(I2S_DAC_CHANNEL_BOTH_EN);
    }
    else {
        i2s_set_pin(config->i2s_num, &pin_config);
    }

    i2s_stop(config->i2s_num);
}

/**
 * I2S is MSB first (big-endian) two's complement (signed) integer format.
 * The I2S module receives and transmits left-channel data first.
 *
 * ESP32 is little-endian.
 */
void render_samples(char *buf, uint32_t buf_len, WAVEFORMATEX* buf_desc) {

	//ESP_LOGI(TAG, "buf_desc: bit_depth %d format %d num_chan %d sample_rate %d", buf_desc->bit_depth, buf_desc->buffer_format, buf_desc->num_channels, buf_desc->sample_rate);
	//ESP_LOGI(TAG, "renderer_instance: bit_depth %d, output_mode %d", renderer_instance->bit_depth, renderer_instance->output_mode);

	// handle changed sample rate
	if(renderer_instance->sample_rate != buf_desc->nSamplesPerSec)
	 {
		 ESP_LOGI(TAG, "changing sample rate from %d to %d", renderer_instance->sample_rate, (int)buf_desc->nSamplesPerSec);
		 //uint32_t rate = buf_desc->sample_rate * renderer_instance->sample_rate_modifier;
		 i2s_set_sample_rates(renderer_instance->i2s_num, buf_desc->nSamplesPerSec);
		 renderer_instance->sample_rate = buf_desc->nSamplesPerSec;
	 }
	//i2s_set_sample_rates(renderer_instance->i2s_num, buf_desc->nSamplesPerSec);

	uint8_t buf_bytes_per_sample = (buf_desc->wBitsPerSample / 8);
	uint32_t num_samples = buf_len / buf_bytes_per_sample / buf_desc->nChannels;
	//ESP_LOGE(TAG, "render_samples num_samples: %d, buf_len:%d", num_samples, buf_len);
	// formats match, we can write the whole block
	/*if (buf_desc->bit_depth == renderer_instance->bit_depth
	 && buf_desc->buffer_format == PCM_INTERLEAVED
	 && buf_desc->num_channels == 2
	 && renderer_instance->output_mode != DAC_BUILT_IN) {

	 // do not wait longer than the duration of the buffer
	 TickType_t max_wait = buf_desc->sample_rate / num_samples / 2;

	 // don't block, rather retry
	 int bytes_left = buf_len;
	 int bytes_written = 0;
	 while(bytes_left > 0 && renderer_status != STOPPED) {
	 bytes_written = i2s_write_bytes(renderer_instance->i2s_num, buf, bytes_left, 0);
	 bytes_left -= bytes_written;
	 buf += bytes_written;
	 }

	 return;
	 }*/

	// support only 16 bit buffers for now
	if (buf_desc->wBitsPerSample != 16) {
		ESP_LOGE(TAG, "unsupported decoder bit depth: %d",
				buf_desc->wBitsPerSample);
		return;
	}

	// pointer to left / right sample position
	char *ptr_l = buf;
	char *ptr_r = buf + buf_bytes_per_sample;
	uint8_t stride = buf_bytes_per_sample * 2;

	// right half of the buffer contains all the right channel samples
	/*if(buf_desc->buffer_format == PCM_LEFT_RIGHT)
	 {
	 ptr_r = buf + buf_len / 2;
	 stride = buf_bytes_per_sample;
	 }*/

	if (buf_desc->nChannels == 1) {
		ptr_r = ptr_l;
	}
	//ESP_LOGE(TAG, "render_samples start");
	int bytes_pushed = 0;
	TickType_t max_wait = 20 / portTICK_PERIOD_MS; // portMAX_DELAY = bad idea
	for (int i = 0; i < num_samples; i++) {
		if (renderer_status == STOPPED)
			break;

		//if(renderer_instance->output_mode == DAC_BUILT_IN)
		//{
		// assume 16 bit src bit_depth
		short left = *(short *) ptr_l;
		short right = *(short *) ptr_r;

		// The built-in DAC wants unsigned samples, so we shift the range
		// from -32768-32767 to 0-65535.
		left = left + 0x8000;
		right = right + 0x8000;

		uint32_t sample = (uint16_t) left;
		sample = (sample << 16 & 0xffff0000) | ((uint16_t) right);
		//ESP_LOGE(TAG, "render_samples i2s_push_sample");
		bytes_pushed = i2s_push_sample(renderer_instance->i2s_num,
				(const char*) &sample, max_wait);
		//ESP_LOGE(TAG, "render_samples i2s_push_sample done");
		/*}
		 else {

		 switch (renderer_instance->bit_depth)
		 {
		 case I2S_BITS_PER_SAMPLE_16BIT:
		 ; // workaround

		 // low - high / low - high
		 const char samp32[4] = {ptr_l[0], ptr_l[1], ptr_r[0], ptr_r[1]};

		 bytes_pushed = i2s_push_sample(renderer_instance->i2s_num, (const char*) &samp32, max_wait);
		 break;

		 case I2S_BITS_PER_SAMPLE_32BIT:
		 ; // workaround

		 const char samp64[8] = {0, 0, ptr_l[0], ptr_l[1], 0, 0, ptr_r[0], ptr_r[1]};
		 bytes_pushed = i2s_push_sample(renderer_instance->i2s_num, (const char*) &samp64, max_wait);
		 break;

		 default:
		 ESP_LOGE(TAG, "bit depth unsupported: %d", renderer_instance->bit_depth);
		 }
		 }*/

		// DMA buffer full - retry
		if (bytes_pushed == 0) {
			i--;
		} else {
			ptr_r += stride;
			ptr_l += stride;
		}
	}
	//ESP_LOGE(TAG, "end render_samples");
	/* takes too long
	 i2s_event_t evt = {0};
	 if(xQueueReceive(i2s_event_queue, &evt, 0)) {
	 if(evt.type == I2S_EVENT_TX_DONE) {
	 ESP_LOGE(TAG, "DMA Buffer Underflow");
	 }
	 }
	 */
}

void render_audio(void *pvParameters)
 {
	char render_buffer[1024];
	int bytes_to_read;
	ESP_LOGI(TAG, "render audio start!");
	while (1) {
		bytes_to_read = spiRamFifoRead(render_buffer, 1024);
		if (bytes_to_read < 1024) {
			ESP_LOGI(TAG, "end of data bytes_to_read:%d!", bytes_to_read);
			render_samples(render_buffer, bytes_to_read, &wfx);
			//set dac to 0
			//char zero_buffer[8] = {0};
			memset(render_buffer, 0x00, 16);
			render_samples(render_buffer, 16, &wfx);
			//return;
			break;
		}
		render_samples(render_buffer, 1024, &wfx);
		//vTaskDelay(1000 / portTICK_RATE_MS);
	}
	spiRamFifoReset();
	ESP_LOGI(TAG, "MAD decoder stack: %d", uxTaskGetStackHighWaterMark(NULL));
	ESP_LOGI(TAG, "render audio end!");
	vTaskDelete(NULL);
}
