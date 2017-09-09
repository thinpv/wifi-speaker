/*
 * data_buffer.c
 *
 *  Created on: Aug 29, 2017
 *      Author: thind
 */

#include "data_buffer.h"
#include "esp_log.h"
#include <string.h>

#define TAG "data_buffer"

data_buffer_s data_buffer[NUMBER_OF_BUFFER];
static int current_write_buffer = 0;
static int current_read_buffer = 0;
static int buffer_used = 0;

void init_data_buffer()
{
	int i;
	for(i=0;i<NUMBER_OF_BUFFER;i++){
		data_buffer[i].used = 0;
		data_buffer[i].data_buffer_state = B_UNPREPARED;
	}
}

void buffer_write_data(char* data, int size)
{
	int remain;
	data_buffer_s* current = &data_buffer[current_write_buffer];
	while (size > 0) {
		if(current->data_buffer_state == B_PREPARED)
		{
			ESP_LOGI(TAG, "buffer overload");
		}
		while(current->data_buffer_state == B_PREPARED);
		if (size < LENGTH_OF_BUFFER - current->used) {
			memcpy(current->buffer + current->used, data, size);
			current->used += size;
			break;
		}
		remain = LENGTH_OF_BUFFER - current->used;
		memcpy(current->buffer + current->used, data, remain);
		current->used = 0;
		current->data_buffer_state = B_PREPARED;
		size -= remain;
		data += remain;
		current_write_buffer++;
		current_write_buffer %= NUMBER_OF_BUFFER;
		current = &data_buffer[current_write_buffer];
		current->used = 0;
		buffer_used++;
	}
}

int buffer_read_data(char* data, int size)
{
	int remain = 0;
	int data_read = 0;
	data_buffer_s* current = &data_buffer[current_read_buffer];
	while (size > 0) {
		//if(current->data_buffer_state == B_UNPREPARED)
			//return data_read;
			//ESP_LOGI(TAG, "waiting for data");
		while(current->data_buffer_state == B_UNPREPARED);
		if (size < LENGTH_OF_BUFFER - current->used) {
			memcpy(data, current->buffer + current->used, size);
			current->used += size;
			return data_read + size;
		}
		remain = LENGTH_OF_BUFFER - current->used;
		memcpy(data, current->buffer + current->used, remain);
		current->used = 0;
		current->data_buffer_state = B_UNPREPARED;
		size -= remain;
		data += remain;
		current_read_buffer++;
		current_read_buffer %= NUMBER_OF_BUFFER;
		current = &data_buffer[current_read_buffer];
		current->used = 0;
		data_read += remain;
		buffer_used--;
	}
	return 0;
}

int getBufferPercent()
{
	return (buffer_used*100)/NUMBER_OF_BUFFER;
}
