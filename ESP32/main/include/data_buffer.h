/*
 * data_buffer.h
 *
 *  Created on: Aug 29, 2017
 *      Author: thind
 */

#ifndef _INCLUDE_DATA_BUFFER_H_
#define _INCLUDE_DATA_BUFFER_H_

#define NUMBER_OF_BUFFER	5
#define LENGTH_OF_BUFFER	5120

typedef enum{
	B_PREPARED,
	B_UNPREPARED
} data_buffer_state_e;

typedef struct _data_buffer_s {
	char buffer[LENGTH_OF_BUFFER];
	int used;
	data_buffer_state_e data_buffer_state;
} data_buffer_s;

void init_data_buffer();
void buffer_write_data(char* data, int size);
int buffer_read_data(char* data, int size);
int getBufferPercent();

#endif /* _INCLUDE_DATA_BUFFER_H_ */
