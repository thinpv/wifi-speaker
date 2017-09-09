#include "AudioIO.h"

#include <assert.h>

int AudioIO::getRecordDevIndex()
{
	string devName;// (devNameArg);
	// if we don't get given a device, look up the preferences
	if (devName.empty())
	{
		devName = "Speakers (2- High Definition Audio Device) (loopback)";
		//devName = "Headphones (2- High Definition Audio Device) (loopback)";
	}

	string hostName = "Windows WASAPI";//gPrefs->Read(wxT("/AudioIO/Host"), wxT(""));
	PaHostApiIndex hostCnt = Pa_GetHostApiCount();
	PaHostApiIndex hostNum;
	for (hostNum = 0; hostNum < hostCnt; hostNum++)
	{
		const PaHostApiInfo *hinfo = Pa_GetHostApiInfo(hostNum);
		if (hinfo && !hostName.compare(hinfo->name))// && wxString(wxSafeConvertMB2WX(hinfo->name)) == hostName)
		{
			for (PaDeviceIndex hostDevice = 0; hostDevice < hinfo->deviceCount; hostDevice++)
			{
				PaDeviceIndex deviceNum = Pa_HostApiDeviceIndexToDeviceIndex(hostNum, hostDevice);

				const PaDeviceInfo *dinfo = Pa_GetDeviceInfo(deviceNum);
				if (dinfo && dinfo->maxInputChannels > 0 && !devName.compare(dinfo->name))// && DeviceName(dinfo) == devName)
				{
					// this device name matches the stored one, and works.
					// So we say this is the answer and return it
					return deviceNum;
				}
			}

			// The device wasn't found so use the default for this host.
			// LL:  At this point, preferences and active no longer match.
			return hinfo->defaultInputDevice;
		}
	}

	// The host wasn't found, so use the default input device.
	// FIXME: TRAP_ERR PaErrorCode not handled well in getRecordDevIndex()
	PaDeviceIndex deviceNum = Pa_GetDefaultInputDevice();

	// Sometimes PortAudio returns -1 if it cannot find a suitable default
	// device, so we just use the first one available
	// PortAudio has an error reporting function.  We should log/report the error?
	//
	// LL:  At this point, preferences and active no longer match
	//
	//      And I can't imagine how far we'll get specifying an "invalid" index later
	//      on...are we certain "0" even exists?
	if (deviceNum < 0) {
		// JKC: This ASSERT will happen if you run with no config file
		// This happens once.  Config file will exist on the next run.
		// TODO: Look into this a bit more.  Could be relevant to blank Device Toolbar.
		assert(false);
		deviceNum = 0;
	}

	return deviceNum;
}

PaStreamParameters* AudioIO::getPaStreamParameters()
{
	PaStreamParameters captureParameters{};
	const PaDeviceInfo *captureDeviceInfo;
	// retrieve the index of the device set in the prefs, or a sensible
	// default if it isn't set/valid
	captureParameters.device = getRecordDevIndex();

	captureDeviceInfo = Pa_GetDeviceInfo(captureParameters.device);

	if (captureDeviceInfo == NULL)
		return nullptr;

	captureParameters.sampleFormat = paInt16;//paInt24//paFloat32

	captureParameters.hostApiSpecificStreamInfo = NULL;
	captureParameters.channelCount = 2;
	captureParameters.suggestedLatency = captureDeviceInfo->defaultHighInputLatency;
	//captureParameters.suggestedLatency = latencyDuration / 1000.0;
	return &captureParameters;
}