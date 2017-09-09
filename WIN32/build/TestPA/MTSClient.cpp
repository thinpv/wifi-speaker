/*
Author :- Nish [BusterBoy]
EMail :- nishinapp@yahoo.com
*/

#include "stdafx.h"
#include "MTSClient.h"
#include <cstdio>
#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include "wav.h"
#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;

#define DATA_LENGTH 5120

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	HANDLE hFile;/* file handle */
	WAVEFORMATEX2 wfx; /* look this up in your documentation */
	wav_hdr wavHeader;
	char buffer[DATA_LENGTH + 5]; /* intermediate buffer for reading */
	DWORD readBytes;
	DWORD length;

	/*
	* try and open the file
	*/
	if ((hFile = CreateFile(
		L"c:\\temp\\test2.wav",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL
		)) == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "%s: unable to open file '%s'\n", argv[0], argv[1]);
		ExitProcess(1);
	}

	/*
	* read header
	*/
	if (!ReadFile(hFile, (DWORD*)&wavHeader, sizeof(wav_hdr), &readBytes, NULL))
		printf("read header error\n");
	/*
	* set up the WAVEFORMATEX structure.
	*/
	cout << "Subchunk1Size    :" << wavHeader.Subchunk1Size << endl;
	wfx.nSamplesPerSec = wavHeader.SamplesPerSec; /* sample rate */
	wfx.wBitsPerSample = wavHeader.bitsPerSample; /* sample size */
	wfx.nChannels = wavHeader.NumOfChan; /* channels*/
	wfx.cbSize = 0; /* size of _extra_ info */
	wfx.wFormatTag = wavHeader.AudioFormat;
	wfx.nBlockAlign = wavHeader.blockAlign;
	wfx.nAvgBytesPerSec = wavHeader.bytesPerSec;

	//Client connect
	//const char* servername="127.0.0.1";
	const char* servername = "192.168.0.101";
	time_t  timev1, timev2;
	WSADATA wsaData;
	struct hostent *hp;
	unsigned int addr;
	struct sockaddr_in server;
	int wsaret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaret)
		return 0;

	SOCKET conn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (conn == INVALID_SOCKET)
		return 0;
	if (inet_addr(servername) == INADDR_NONE)
	{
		printf("INADDR_NONE\n");
		hp=gethostbyname(servername);
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

	tcp_command received_command = UN_KNOW;
	/*
	* send format wav
	*/
	buffer[0] = SEND_HEADER;
	DWORD temp = sizeof(wfx);
	printf("sizeof(wfx): %d", sizeof(wfx));
	memcpy(buffer + 1, &temp, sizeof(DWORD));//set length
	memcpy(buffer + 5, &wfx, sizeof(wfx));//set length
	length = sizeof(wfx)+5;
	send(conn, buffer, length, 0);

	buffer[0] = 0;
	while (true)//waiting ACK
	{
		recv(conn, buffer, 512, 0);
		if (buffer[0] == SEND_ACK)
		{
			break;
		}
	}
	printf("start send data\n");
	/*
	* playback loop
	*/
	time(&timev1);
	int count = 0;
	while (1) 
	{
		buffer[0] = SEND_DATA;
		if (!ReadFile(hFile, buffer + 5, DATA_LENGTH, &readBytes, NULL))
			break;
		if (readBytes == 0)
			break;
		memcpy(buffer + 1, &readBytes, sizeof(DWORD));//set length
		length = readBytes + 5;
		if (count % 10 == 0)
			printf("count: %d\n", count);
		if (readBytes < DATA_LENGTH)
		{
			printf("at end of buffer\n");
			memset(buffer + readBytes, 0, sizeof(buffer)-readBytes);
			printf("after memcpy\n");
			time(&timev2);
			printf("time: %d seconds\n", timev2 - timev1);
		}
	
		send(conn, buffer, length, 0);
		buffer[0] = 0;
		while (true)
		{
			recv(conn, buffer, 512, 0);
			if (buffer[0] == SEND_ACK)
			{
				break;
			}
		}
		count++;
	}

	CloseHandle(hFile);
	closesocket(conn);
	WSACleanup();
	getchar();
	return 0;
}
