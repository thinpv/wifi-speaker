/** @file paex_record.c
@ingroup examples_src
@brief Record input into an array; Save array to a file; Playback recorded data.
@author Phil Burk  http://www.softsynth.com
*/
/*
* $Id$
*
* This program uses the PortAudio Portable Audio Library.
* For more information see: http://www.portaudio.com
* Copyright (c) 1999-2000 Ross Bencina and Phil Burk
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* The text above constitutes the entire PortAudio license; however,
* the PortAudio community also makes the following non-binding requests:
*
* Any person wishing to distribute modifications to the Software is
* requested to send the modifications to the original developer so that
* they can be incorporated into the canonical version. It is also
* requested that these non-binding requests be included along with the
* license above.
*/

#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "AudioIO.h"
#include "wav.h"
#include "stdafx.h"



/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (48000)
#define FRAMES_PER_BUFFER (1024)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (2)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

/* Select sample format. */
#if 0
//paFloat32
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct tWAVEFORMATEX2
{
	WORD        wFormatTag;         /* format type */
	WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD       nSamplesPerSec;     /* sample rate */
	DWORD       nAvgBytesPerSec;    /* for buffer estimation */
	WORD        nBlockAlign;        /* block size of data */
	WORD        wBitsPerSample;     /* number of bits per sample of mono data */
	WORD        cbSize;             /* the count in bytes of the size of */
	WORD        frameIndex;		//bien tam
	/* extra information (after cbSize) */
} WAVEFORMATEX2;

typedef struct
{
	int				frameIndex;  /* Index into sample array. */
	SAMPLE			*audio_data;
	WAVEFORMATEX2	*wfx;
	SOCKET			*tcp_connectione;
	int				data_state;
	int				size_buffer;
}UserData;

SOCKET conn;
time_t  timev1, timev2;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	int count_data = 0, i;
	UserData *uData = (UserData*)userData;
	const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
	DWORD data_length;
	TCP_COMMAND tcp_command;
	//////////////////////////////////////
	/*count_data = 0;
	tcp_command = SEND_HEADER;
	memcpy((char*)uData->audio_data + count_data, (char*)&tcp_command, sizeof(TCP_COMMAND));//set command
	count_data += sizeof(TCP_COMMAND);

	data_length = sizeof(WAVEFORMATEX2);
	memcpy((char*)uData->audio_data + count_data, (char*)&data_length, sizeof(DWORD));//set header
	count_data += sizeof(DWORD);

	memcpy((char*)uData->audio_data + count_data, (char*)uData->wfx, sizeof(WAVEFORMATEX2));//set header
	count_data += sizeof(WAVEFORMATEX2);

	send(*uData->tcp_connectione, (const char*)uData->audio_data, count_data, 0);
	printf("sent tcp data\n");*/
	//////////////////////////////////////
	count_data = 0;
	tcp_command = SEND_DATA;
	memcpy((char*)uData->audio_data + count_data, (char*)&tcp_command, sizeof(TCP_COMMAND));//set command
	count_data += sizeof(TCP_COMMAND);

	//memcpy((char*)uData->audio_data + count_data, (char*)uData->wfx, sizeof(WAVEFORMATEX2));//set header
	//count_data += sizeof(WAVEFORMATEX2);
	////
	if (uData->data_state == 1)
	{
		printf("data overwrite\n");
	}
	
	if (NUM_CHANNELS == 2)
	{
		data_length = framesPerBuffer * 4;
	}
	else
	{
		data_length = framesPerBuffer * 2;
	}
	memcpy((char*)uData->audio_data + count_data, (char*)&data_length, sizeof(DWORD));//set header
	count_data += sizeof(DWORD);
	////

	memcpy((char*)uData->audio_data + count_data, (char*)inputBuffer, data_length);//set header
	count_data += data_length;
	/*if (inputBuffer == NULL)
	{
		for (i = 0; i<framesPerBuffer; i++)
		{
			uData->audio_data[count_data+i] = SAMPLE_SILENCE;  
			//(uData->(audio_data))++ = SAMPLE_SILENCE; 
			if (NUM_CHANNELS == 2)
				uData->audio_data[count_data+i] = SAMPLE_SILENCE; 
		}
	}
	else
	{
		for (i = 0; i<framesPerBuffer; i++)
		{
			uData->audio_data[count_data+i] = *rptr++;  
			if (NUM_CHANNELS == 2)
				uData->audio_data[count_data+i] = *rptr++;  
		}
	}*/
	//count_data += data_length;
	//send tcp
	
	uData->size_buffer = count_data;
	uData->data_state = 1;
	//send(*uData->tcp_connectione, (const char*)uData->audio_data, count_data, 0);
	//printf("sent tcp data\n");
	return paContinue;
}

int connectSocket()
{
	const char* servername="127.0.0.1";
	//const char* servername = "192.168.0.103";
	//const char* servername = "192.168.4.1";
	
	WSADATA wsaData;
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	int wsaret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaret)
		return 0;

	conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return 0;
	if (inet_addr(servername) == INADDR_NONE)
	{
		printf("INADDR_NONE\n");
		hp = gethostbyname(servername);
		server.sin_addr.s_addr = *((unsigned long*)hp->h_addr);
	}
	else
	{
		printf("!INADDR_NONE\n");
		//addr = inet_addr(servername);
		//hp = gethostbyaddr((char*)&addr, sizeof(addr), AF_INET);
		server.sin_addr.s_addr = inet_addr(servername);
	}
	/*if(hp==NULL)
	{
	printf("error#:%ld\n", WSAGetLastError());
	closesocket(conn);
	return 0;
	}*/
	//server.sin_addr.s_addr = inet_addr(servername); //*((unsigned long*)hp->h_addr);
	server.sin_family = AF_INET;
	server.sin_port = htons(4567);
	if (connect(conn, (struct sockaddr*)&server, sizeof(server)))
	{
		closesocket(conn);
		return 0;
	}
	return 1;
}

/*******************************************************************/
int main(void);
int main(void)
{
	PaStreamParameters  inputParameters;
	PaStream*           stream;
	PaError             err = paNoError;
	AudioIO				audioIO;
	UserData			uData;


	//unsigned __int64 freq;
	//QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	//double timerFrequency = (1.0 / freq);
	//unsigned __int64 startTime, endTime;

	printf("patest_record.c\n"); fflush(stdout);

	if (connectSocket() != 1)
		printf("connect socket fail\n");

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	inputParameters.device = audioIO.getRecordDevIndex();
	//inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default input device.\n");
		goto done;
	}
	inputParameters.channelCount = 2;                    /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	unsigned long numSamples = FRAMES_PER_BUFFER * NUM_CHANNELS;
	unsigned long numBytes = numSamples * sizeof(SAMPLE);
	SAMPLE* audio_data = (SAMPLE *)malloc(numBytes + sizeof(TCP_COMMAND) + sizeof(WAVEFORMATEX2));
	printf("size buffer:%d\n", numBytes + sizeof(TCP_COMMAND) + sizeof(WAVEFORMATEX2));
	printf("numBytes:%d\n", numBytes);
	printf("sizeof(TCP_COMMAND):%d\n", sizeof(TCP_COMMAND));
	printf("sizeof(WAVEFORMATEX2):%d\n", sizeof(WAVEFORMATEX2));
	WAVEFORMATEX2 wfx;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 48000;
	wfx.wBitsPerSample = 16;
	//wfx.
	uData.audio_data = audio_data;
	uData.frameIndex = 0;
	uData.wfx = &wfx;
	uData.tcp_connectione = &conn;
	uData.data_state = 0;

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		//NULL,
		//paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		paNoFlag,
		recordCallback,
		&uData);
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;
	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

	while ((err = Pa_IsStreamActive(stream)) == 1)
	{
		if (uData.data_state == 1)
		{
			//QueryPerformanceCounter((LARGE_INTEGER *)&startTime);
			send(*uData.tcp_connectione, (const char*)uData.audio_data, uData.size_buffer, 0);
			//QueryPerformanceCounter((LARGE_INTEGER *)&endTime);
			//double timeDifferenceInMilliseconds = ((endTime - startTime) * timerFrequency);
			//if (timeDifferenceInMilliseconds > 1){
			//	printf("time: %d seconds\n", timeDifferenceInMilliseconds);
			//}
			uData.data_state = 0;
		}
		//Pa_Sleep(1000);
		//printf("index = %d\n", wfx.frameIndex); fflush(stdout);
	}
	if (err < 0) goto done;

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

done:
	Pa_Terminate();
	if (err != paNoError)
	{
		fprintf(stderr, "An error occured while using the portaudio stream\n");
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		err = 1;          /* Always return 0 or 1, but no other return codes. */
	}
	getchar();
	return err;
}

