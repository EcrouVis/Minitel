#include "thread_messaging.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include <cstdio>
#include <atomic>

struct mBuzzer{
	std::atomic<float>* p_buzzer_amplitude=NULL;
	ma_waveform wf;
	
};

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	//printf("audio %u\n",frameCount);
	mBuzzer* p_buzzer=(mBuzzer*)pDevice->pUserData;
	if (p_buzzer->p_buzzer_amplitude!=NULL){
		float A=p_buzzer->p_buzzer_amplitude->load(std::memory_order_acquire);
		ma_waveform_set_amplitude(&(p_buzzer->wf),A);
		ma_waveform_read_pcm_frames(&(p_buzzer->wf), pOutput, frameCount,NULL);
	}
}

void thread_audio_main(Mailbox* p_mb_circuit,Mailbox* p_mb_audio,GlobalState* p_gState){
	
	ma_waveform waveform;

	ma_waveform_config config = ma_waveform_config_init(
		ma_format_f32,
		1,
		48000,
		ma_waveform_type_square,
		0,
		2982
	);
	
	ma_waveform_init(&config, &waveform);
	
	ma_result result;
    ma_context context;
    ma_device_info* pPlaybackDeviceInfos;
    ma_uint32 playbackDeviceCount;
    ma_device_info* pCaptureDeviceInfos;
    ma_uint32 captureDeviceCount;
    ma_uint32 iDevice;

    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.\n");
        return;
    }

    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
    if (result != MA_SUCCESS) {
        printf("Failed to retrieve device information.\n");
        return;
    }

    printf("Playback Devices\n");
    for (iDevice = 0; iDevice < playbackDeviceCount; ++iDevice) {
        printf("    %u: %s\n", iDevice, pPlaybackDeviceInfos[iDevice].name);
    }

    printf("Capture Devices\n");
    for (iDevice = 0; iDevice < captureDeviceCount; ++iDevice) {
        printf("    %u: %s\n", iDevice, pCaptureDeviceInfos[iDevice].name);
    }
	
    ma_context_uninit(&context);
	
	mBuzzer buzzer;
	buzzer.wf=waveform;
	
	ma_device_config deviceConfig;
    ma_device device;
    //deviceConfig = ma_device_config_init(ma_device_type_duplex);
	deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1;
    //deviceConfig.capture.format   = ma_format_f32;
    //deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate        = 48000;//48000 is a multiple of 1200 and 75 and 48000 is commonly used
    deviceConfig.dataCallback      = data_callback;
	//deviceConfig.periodSizeInFrames = 64;
    deviceConfig.pUserData         = &buzzer;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return;
    }

    printf("Device Name: %s\n", device.playback.name);
	
	if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        return;
    }
	
	while (!p_gState->shutdown.load(std::memory_order_relaxed)){
		thread_message ms;
		while (p_mb_audio->receive(&ms)){
					printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
			switch(ms.cmd){
				case BUZZER_AMPLITUDE:
					buzzer.p_buzzer_amplitude=(std::atomic<float>*)ms.p;
					printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
					break;
					
			}
		}
	}
	
	ma_device_uninit(&device);
}