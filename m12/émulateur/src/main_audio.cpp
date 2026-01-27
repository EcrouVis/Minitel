#include "thread_messaging.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include <cstdio>
#include <atomic>
#include <thread>

#include "io/TS7514Audio.h"

struct audioContext{
	WLOConf* wloc;
	TS7514* modem=NULL;
};

void phone_line_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	
	//audio to speaker
	/*ma_uint32 framesToWrite=frameCount;
	void* pMappedBuffer;
	ma_pcm_rb_acquire_write(&(p_buzzer->rb), &framesToWrite, &pMappedBuffer);
	if (framesToWrite!=0){
		ma_copy_pcm_frames(pMappedBuffer, pInput, framesToWrite, ma_format_f32, 1);//in
		ma_mix_pcm_frames_f32((float*)pMappedBuffer, (const float*)pOutput, framesToWrite,1,1);//out
	}
	ma_pcm_rb_commit_write(&(p_buzzer->rb), framesToWrite);*/
	//ma_copy_and_apply_volume_factor_pcm_frames
	//ma_mix_pcm_frames_f32
}

void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	//buzzer
	audioContext* AC=(audioContext*)pDevice->pUserData;
	if (AC->modem!=NULL){
		TS7514_WLO(AC->modem, AC->wloc, pOutput, frameCount);
	}
	//audio from phone line
	/*ma_uint32 framesToRead=frameCount;
	void* pMappedBuffer;
	ma_pcm_rb_acquire_read(&(p_buzzer->rb), &framesToRead, &pMappedBuffer);
	if (framesToRead!=0){
		//ma_copy_pcm_frames(pOutput, pMappedBuffer, framesToRead, ma_format_f32, 1);
		ma_mix_pcm_frames_f32((float*)pOutput, (const float*)pMappedBuffer, framesToRead,1,1);
	}
	ma_pcm_rb_commit_read(&(p_buzzer->rb), framesToRead);*/
}

void thread_audio_main(Mailbox* p_mb_circuit,Mailbox* p_mb_audio,GlobalState* p_gState){
	
	WLOConf wloc;
	
	TS7514_WLO_init(&wloc,48000);
	
	audioContext AC;
	AC.wloc=&wloc;
	
	/*ma_pcm_rb_init(ma_format_f32, 1, 512, NULL, NULL, &(buzzer.rb));
	ma_pcm_rb_set_sample_rate(&(buzzer.rb), 48000);*/
	
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
	
	//ma_context_config contextConfig;
	
	
	ma_device_config deviceConfig;
    ma_device device;
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
	
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 1;
	
    deviceConfig.capture.format   = ma_format_f32;
    deviceConfig.capture.channels = 1;
	
    deviceConfig.sampleRate        = 48000;//48000 is a multiple of 1200 and 75 and 48000 is commonly used
	
    deviceConfig.dataCallback      = phone_line_data_callback;
	deviceConfig.periodSizeInFrames = 64;//audio glitch if >100 frames ???
	deviceConfig.wasapi.usage = ma_wasapi_usage_pro_audio;
    deviceConfig.pUserData         = NULL;

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
	
	ma_device_config device2Config;
    ma_device device2;
	device2Config = ma_device_config_init(ma_device_type_playback);
	
    device2Config.playback.format   = ma_format_f32;
    device2Config.playback.channels = 1;
	
    device2Config.sampleRate        = 48000;
	
    device2Config.dataCallback      = audio_data_callback;
	device2Config.periodSizeInFrames = 64;
	device2Config.wasapi.usage = ma_wasapi_usage_pro_audio;
    device2Config.pUserData         = &AC;

    if (ma_device_init(NULL, &device2Config, &device2) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        return;
    }

    printf("Device Name: %s\n", device2.playback.name);
	
	if (ma_device_start(&device2) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device2);
        return;
    }
	
	while (!p_gState->shutdown.load(std::memory_order_relaxed)){
		thread_message ms;
		while (p_mb_audio->receive(&ms)){
			switch(ms.cmd){
				case MODEM:
					AC.modem=(TS7514*)ms.p;
					break;
					
			}
		}
		
		std::this_thread::yield();
	}
	
	TS7514_WLO_uninit(&wloc);
	
	ma_device_uninit(&device);
}