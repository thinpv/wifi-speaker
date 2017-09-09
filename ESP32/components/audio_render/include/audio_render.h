/*
 * audio_render.h
 *
 *  Created on: Aug 27, 2017
 *      Author: thind
 */

#ifndef _INCLUDE_AUDIO_RENDER_H_
#define _INCLUDE_AUDIO_RENDER_H_

#include "wav.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/gpio.h"

typedef enum {
    UNINITIALIZED, INITIALIZED, RUNNING, STOPPED
} component_status_t;

typedef enum {
    I2S, I2S_MERUS, DAC_BUILT_IN, PDM
} output_mode_t;

typedef struct
{
    output_mode_t output_mode;
    int sample_rate;
    float sample_rate_modifier;
    i2s_bits_per_sample_t bit_depth;
    i2s_port_t i2s_num;
} renderer_config_t;

renderer_config_t *create_renderer_config();
void renderer_init(renderer_config_t *config);
void init_i2s(renderer_config_t *config);
void render_samples(char *buf, uint32_t buf_len, WAVEFORMATEX* buf_desc);
void data_cb(const uint8_t *data, uint32_t len);
void render_audio(void *pvParameters);

#endif /* _INCLUDE_AUDIO_RENDER_H_ */
