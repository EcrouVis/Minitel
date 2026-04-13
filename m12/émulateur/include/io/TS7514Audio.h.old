#ifndef TS7514AUDIO_H
#define TS7514AUDIO_H

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"
#include <atomic>

struct TS7514AudioContext{
	//WLO
	std::atomic_uchar* RWLO=NULL;
	ma_waveform WLO_buzzer;
	ma_audio_buffer WLO_buffer;
	ma_lpf WLO_output_filter;
	float WLO_volume=1.;
};

struct WLOConf{
	std::atomic_uchar* pRWLO=NULL;
	ma_waveform buzzer;
	ma_audio_buffer* pwlo_buffer=NULL;
	ma_lpf filter;
	float wlo_volume=1.;
};

void TS7514_WLO_init(WLOConf* wloc, ma_uint32 sampleRate);

void TS7514_WLO_uninit(WLOConf* wloc);

void TS7514_WLO(WLOConf* wloc, void* pOutput, ma_uint32 frameCount);
	
struct ATOConf{
	ma_waveform sine1;
	ma_waveform sine2;
};
const float SOS_LPF_1[6]={0.0061693731349803035,0.006337652609703886,0.0061693731349803035,1.,-1.2639113019223365,0.4158586728754339};//change later
const float SOS_LPF_2[6]={1.,-0.607624405831732,1.0000000000000002,1.,-1.5563336870970133,0.7274755885585624};//change later
const float SOS_HPF[6]={0.9741031222142027,-1.9481263084935099,0.9741031222142029,1.,-1.9474555343252202,0.9488770185966936};//change later
//smoothing filter -> TODO

bool TS7514_ATxI_Resample_1228800_48000(bool b, float* ret);

struct RA2Conf{
	
};

#endif