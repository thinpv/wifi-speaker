/*
 * audio_player.h
 *
 *  Created on: 12.03.2017
 *      Author: michaelboeckling
 */

#ifndef INCLUDE_AUDIO_PLAYER_H_
#define INCLUDE_AUDIO_PLAYER_H_

#include <sys/types.h>
#include "audio_render.h"

int audio_stream_consumer(const char *recv_buf, ssize_t bytes_read);


typedef enum {
    CMD_NONE, CMD_START, CMD_STOP
} player_command_t;

typedef enum {
    BUF_PREF_FAST, BUF_PREF_SAFE
} buffer_pref_t;

typedef enum
{
    MIME_UNKNOWN = 1, OCTET_STREAM, AUDIO_AAC, AUDIO_MP4, AUDIO_MPEG
} content_type_t;

typedef struct {
    content_type_t content_type;
    bool eof;
} media_stream_t;

component_status_t get_player_status();

void audio_player_init();
void audio_player_start();
void audio_player_stop();
void audio_player_destroy();


#endif /* INCLUDE_AUDIO_PLAYER_H_ */
