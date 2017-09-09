/**********************************************************************

  Audacity: A Digital Audio Editor

  AudioIO.h

  Dominic Mazzoni

  Use the PortAudio library to play and record sound

**********************************************************************/

#ifndef __AUDIO_IO__
#define __AUDIO_IO__

#include <string>
#include "portaudio.h"

using namespace std;

class AudioIO final {

public:
	AudioIO(){};
	~AudioIO(){};

	int getRecordDevIndex();
	PaStreamParameters* getPaStreamParameters();
};


#endif

