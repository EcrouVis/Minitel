#include "thread_messaging.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include <cstdio>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
	
}

void thread_audio_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_audio,GlobalState* p_gState){
	
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
	
	
	ma_device_config deviceConfig;
    ma_device device;
    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.playback.format   = ma_format_u8;
    deviceConfig.playback.channels = 1;
    deviceConfig.capture.format   = ma_format_u8;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate        = 44100;
    deviceConfig.dataCallback      = data_callback;
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
	
	while (!p_gState->shutdown.load(std::memory_order_relaxed)){
	}
	
	ma_device_uninit(&device);
}