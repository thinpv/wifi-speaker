
#if !defined(AFX_MTSCLIENT_H__5FC267AB_3019_11D6_AA7C_00C026A39668__INCLUDED_)
#define AFX_MTSCLIENT_H__5FC267AB_3019_11D6_AA7C_00C026A39668__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"

typedef struct tWAVEFORMATEX2
{
	WORD        wFormatTag;         /* format type */
	WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
	DWORD       nSamplesPerSec;     /* sample rate */
	DWORD       nAvgBytesPerSec;    /* for buffer estimation */
	WORD        nBlockAlign;        /* block size of data */
	WORD        wBitsPerSample;     /* number of bits per sample of mono data */
	WORD        cbSize;             /* the count in bytes of the size of */
	WORD        fillFullStruct;
	/* extra information (after cbSize) */
} WAVEFORMATEX2;

typedef enum {
	UN_KNOW,
	SEND_HEADER,
	SEND_DATA,
	SEND_ACK
}tcp_command;

#endif // !defined(AFX_MTSCLIENT_H__5FC267AB_3019_11D6_AA7C_00C026A39668__INCLUDED_)
