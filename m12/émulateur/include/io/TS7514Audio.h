#ifndef TS7514IO_H
#define TS7514IO_H

#include "circuit/TS7514.h"
#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"
#include <atomic>

struct WLOConf{
	ma_waveform buzzer;
	ma_audio_buffer* pwlo_buffer=NULL;
	ma_lpf filter;
	float wlo_volume=1.;
};

void TS7514_WLO_init(WLOConf* wloc, ma_uint32 sampleRate){
	
	ma_waveform_config wf_config = ma_waveform_config_init(ma_format_f32,1,sampleRate,ma_waveform_type_square,0,2982);
	ma_waveform_init(&wf_config, &(wloc->buzzer));
	
	ma_lpf_config lpf_config = ma_lpf_config_init(ma_format_f32, 1, sampleRate, 3000, 1);
	ma_lpf_init(&lpf_config, NULL, &(wloc->filter));
	
	ma_audio_buffer_config config = ma_audio_buffer_config_init(ma_format_f32,1,0,NULL,NULL);
	ma_audio_buffer_alloc_and_init(&config,&(wloc->pwlo_buffer));
}

void TS7514_WLO_uninit(WLOConf* wloc){
	ma_waveform_uninit(&(wloc->buzzer));
	ma_lpf_uninit(&(wloc->filter),NULL);
	ma_audio_buffer_uninit_and_free(wloc->pwlo_buffer);
}

void TS7514_WLO(TS7514* modem, WLOConf* wloc, void* pOutput, ma_uint32 frameCount){
	
	static ma_uint64 availableFrames=0;//resize wlo buffer if needed
	ma_audio_buffer_seek_to_pcm_frame(wloc->pwlo_buffer, 0);
	ma_audio_buffer_get_available_frames(wloc->pwlo_buffer,&availableFrames);
	if (frameCount>availableFrames){
		ma_audio_buffer_uninit_and_free(wloc->pwlo_buffer);
		ma_audio_buffer_config config = ma_audio_buffer_config_init(ma_format_f32,1,frameCount,NULL,NULL);
		ma_audio_buffer_alloc_and_init(&config,&(wloc->pwlo_buffer));
	}
	static void* pbuffer;
	availableFrames=frameCount;
	ma_audio_buffer_map(wloc->pwlo_buffer,&pbuffer,&availableFrames);
	
	if (modem!=NULL){
		const unsigned char RWLO=modem->REG[modem->RWLO].load(std::memory_order_acquire);
		switch (RWLO&0x0C){
			case 0x00:
			{
				const float A[4]={0.316227766,0.1,0.028183829,0.007943282};
				ma_silence_pcm_frames(pbuffer,availableFrames,ma_format_f32,1);//TODO////////////////////////////////////////////////////Tx
				break;
			}
			case 0x04:
			{
				const float A[4]={1.,0.316227766,0.1,0.028183829};
				ma_silence_pcm_frames(pbuffer,availableFrames,ma_format_f32,1);//TODO////////////////////////////////////////////////////Rx
				break;
			}
			case 0x08:
			{
				const float A[4]={1.,0.316227766,0.1,0.028183829};
				//printf("%f %f %i %f\n",A[RWLO&3],wloc->buzzer.config.amplitude,wloc->buzzer.config.sampleRate,wloc->buzzer.config.frequency);
				ma_waveform_set_amplitude(&(wloc->buzzer),A[RWLO&3]);
				ma_waveform_read_pcm_frames(&(wloc->buzzer), pbuffer, availableFrames,NULL);
				//for (int i=0;i<availableFrames;i++) printf("%f ",((float*)pbuffer)[i]);
				//printf("\n");
				break;
			}
			case 0x0C:
				ma_silence_pcm_frames(pbuffer,availableFrames,ma_format_f32,1);
				break;
		}
		
		ma_lpf_process_pcm_frames(&(wloc->filter), pbuffer, pbuffer, availableFrames);
		ma_mix_pcm_frames_f32((float*)pOutput, (const float*)pbuffer, availableFrames,1,wloc->wlo_volume);
		
		ma_audio_buffer_unmap(wloc->pwlo_buffer,availableFrames);
	}
	
}

#endif