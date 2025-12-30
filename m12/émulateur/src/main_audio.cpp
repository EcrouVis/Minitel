#include "thread_messaging.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include <cstdio>
#include <atomic>

struct mBuzzer{
	std::atomic<float>* p_buzzer_amplitude=NULL;
	ma_waveform wf;
	ma_lpf lpf;
	ma_pcm_rb rb;
};

void phone_line_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	
	mBuzzer* p_buzzer=(mBuzzer*)pDevice->pUserData;
	
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
	mBuzzer* p_buzzer=(mBuzzer*)pDevice->pUserData;
	if (p_buzzer->p_buzzer_amplitude!=NULL){
		float A=p_buzzer->p_buzzer_amplitude->load(std::memory_order_acquire);
		ma_waveform_set_amplitude(&(p_buzzer->wf),A);
		ma_waveform_read_pcm_frames(&(p_buzzer->wf), pOutput, frameCount,NULL);
		ma_lpf_process_pcm_frames(&(p_buzzer->lpf), pOutput, pOutput, frameCount);
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
	
	ma_waveform waveform;
	ma_waveform_config wf_config = ma_waveform_config_init(ma_format_f32,1,48000,ma_waveform_type_square,0,2982);
	ma_waveform_init(&wf_config, &waveform);
	
	ma_lpf lpf;
	ma_lpf_config lpf_config = ma_lpf_config_init(ma_format_f32, 1, 48000, 3000, 1);
	ma_lpf_init(&lpf_config, NULL, &lpf);
	
	mBuzzer buzzer;
	buzzer.wf=waveform;
	buzzer.lpf=lpf;
	
	ma_pcm_rb_init(ma_format_f32, 1, 512, NULL, NULL, &(buzzer.rb));
	ma_pcm_rb_set_sample_rate(&(buzzer.rb), 48000);
	
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
	
	ma_device_config device2Config;
    ma_device device2;
	device2Config = ma_device_config_init(ma_device_type_playback);
	
    device2Config.playback.format   = ma_format_f32;
    device2Config.playback.channels = 1;
	
    device2Config.sampleRate        = 48000;
	
    device2Config.dataCallback      = audio_data_callback;
	device2Config.periodSizeInFrames = 64;
	device2Config.wasapi.usage = ma_wasapi_usage_pro_audio;
    device2Config.pUserData         = &buzzer;

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
				case BUZZER_AMPLITUDE:
					buzzer.p_buzzer_amplitude=(std::atomic<float>*)ms.p;
					break;
					
			}
		}
	}
	
	ma_device_uninit(&device);
}