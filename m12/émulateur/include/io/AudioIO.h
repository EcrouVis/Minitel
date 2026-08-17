#ifndef AUDIOIO_H
#define AUDIOIO_H

#include "imgui.h"
#include "miniaudio/miniaudio.h"

#include "Parameters.h"
#include "io/NotificationServer.h"

#include "circuit/clocks.h"

#include "circuit/AudioBuffer.h"
#include "circuit/SpeakerFilter.h"
#include "circuit/BuzzerFilter.h"
#include "circuit/PhoneLineBuffer.h"

#include <cstdlib>
#include <atomic>

class AudioIO{
	public:
		NotificationServer* Notification=NULL;
		Parameters* p_PARAMETERS=NULL;
		
		ma_context miniaudioContext;
		ma_result result=MA_NO_DATA_AVAILABLE;
		ma_device_info* pPlaybackDeviceInfos;
		ma_uint32 playbackDeviceCount;
		ma_device_info* pCaptureDeviceInfos;
		ma_uint32 captureDeviceCount;
		
		bool phoneOutput=false;
		ma_device_config audioDeviceConfig;
		ma_device audioDevice;
		ma_device_config phoneLineDeviceConfig;
		ma_device phoneLineDevice={0};//init memory -> ensure ma_device_state_uninitialized
		//shared data
		Clocks* pCLKs=NULL;
		SpeakerFilter* spkf=NULL;
		BuzzerFilter* bzf=NULL;
		AudioBuffer* ab=NULL;
		PhoneLineBuffer* plb=NULL;
		float samplesRemaining[UCHAR_MAX];
		unsigned char samplesRemainingIndex;
		unsigned long maxSamplesRemaining=1024;
		
		AudioIO(Parameters* p_PARAMETERS,NotificationServer* notif){
			this->p_PARAMETERS=p_PARAMETERS;
			this->Notification=notif;
			
			if (ma_context_init(NULL, 0, NULL, &(this->miniaudioContext)) != MA_SUCCESS) {
				printf("Failed to initialize context.\n");
				exit(-1);
			}
			this->updateAudioDevices();
			
			//see https://github.com/mackron/miniaudio/discussions/1084
			//see https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/low-latency-audio
			this->audioDeviceConfig = ma_device_config_init(ma_device_type_playback);
			this->audioDeviceConfig.playback.format   = ma_format_f32;
			this->audioDeviceConfig.playback.channels = 1;
			this->audioDeviceConfig.sampleRate        = 0;//48000;
			this->audioDeviceConfig.dataCallback      = this->audio_data_callback;
			this->audioDeviceConfig.noFixedSizedCallback=true;
			this->audioDeviceConfig.periodSizeInFrames = 512;
			this->audioDeviceConfig.wasapi.noAutoConvertSRC = true;
			this->audioDeviceConfig.pUserData         = this;

			if (ma_device_init(NULL, &this->audioDeviceConfig, &(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to open playback device.\n");
				exit(-1);
			}

			printf("Device Name: %s\n", this->audioDevice.playback.name);
			
			if (ma_device_start(&(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to start playback device.\n");
				ma_device_uninit(&(this->audioDevice));
				exit(-1);
			}
			
			this->phoneLineDeviceConfig = ma_device_config_init(ma_device_type_duplex);
			this->phoneLineDeviceConfig.playback.format   = ma_format_f32;
			this->phoneLineDeviceConfig.playback.channels = 1;
			this->phoneLineDeviceConfig.capture.format   = ma_format_f32;
			this->phoneLineDeviceConfig.capture.channels = 1;
			this->phoneLineDeviceConfig.sampleRate        = 0;
			this->phoneLineDeviceConfig.dataCallback      = this->phone_line_data_callback;
			//this->phoneLineDeviceConfig.noFixedSizedCallback=true;//don't play nice with ma_share_mode_exclusive if the audio thread skip samples (for wasapi)
			this->phoneLineDeviceConfig.periodSizeInFrames = 128;
			this->phoneLineDeviceConfig.wasapi.noAutoConvertSRC = true;
			this->phoneLineDeviceConfig.performanceProfile = ma_performance_profile_low_latency;//make audio thread prioritized to avoid dropping frames
			this->phoneLineDeviceConfig.wasapi.usage = ma_wasapi_usage_pro_audio;
			//this->phoneLineDeviceConfig.playback.shareMode = ma_share_mode_exclusive;//make playback device exclusive to avoid interferences with other programs /-> disabled because miniaudio could have desync issues at the moment
			this->phoneLineDeviceConfig.pUserData         = this;
			
		}
		~AudioIO(){
			if (ma_device_get_state(&(this->audioDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->audioDevice));//uninit audio before stoping -> don't read deleted buffer
			if (ma_device_get_state(&(this->phoneLineDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->phoneLineDevice));
			
			//wait emulator response
			while (this->p_PARAMETERS->p_gState->minitelOn.load(std::memory_order_relaxed)){
				if (pCLKs!=NULL) pCLKs->requestSamples(512,512);//don't wait that a timout occur in clocks.h ->speed up shutdown
			}
			
			ma_context_uninit(&(this->miniaudioContext));
		}
		
		void updateAudioDevices(){
			this->result=ma_context_get_devices(&(this->miniaudioContext), &(this->pPlaybackDeviceInfos), &(this->playbackDeviceCount), &(this->pCaptureDeviceInfos), &(this->captureDeviceCount));
			if ( this->result!= MA_SUCCESS) {
				printf("Failed to retrieve device information.\n");
			}
		}
		
		void initPhoneLine(){
			if (ma_device_get_state(&(this->audioDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->audioDevice));
			if (ma_device_get_state(&(this->phoneLineDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->phoneLineDevice));
			
			this->updateAudioDevices();
			this->phoneLineDeviceConfig.capture.pDeviceID=NULL;
			this->phoneLineDeviceConfig.playback.pDeviceID=NULL;
			for (ma_uint32 iDevice = 0; iDevice < this->captureDeviceCount; ++iDevice) {
				if (ma_device_id_equal(&(this->p_PARAMETERS->io.modem.audio.captureDeviceId),&(this->pCaptureDeviceInfos[iDevice].id))){
					this->phoneLineDeviceConfig.capture.pDeviceID=&(this->p_PARAMETERS->io.modem.audio.captureDeviceId);
					break;
				}
			}
			for (ma_uint32 iDevice = 0; iDevice < this->playbackDeviceCount; ++iDevice) {
				if (ma_device_id_equal(&(this->p_PARAMETERS->io.modem.audio.playbackDeviceId),&(this->pPlaybackDeviceInfos[iDevice].id))){
					this->phoneLineDeviceConfig.playback.pDeviceID=&(this->p_PARAMETERS->io.modem.audio.playbackDeviceId);
					break;
				}
			}
			if (this->phoneLineDeviceConfig.capture.pDeviceID==NULL){
				constexpr static char msg[]="Entrée audio non spécifiée.";
				this->Notification->notify(msg,false,ImVec4(1,0.5,0,1));
			}
			if (this->phoneLineDeviceConfig.playback.pDeviceID==NULL){
				constexpr static char msg[]="Sortie audio non spécifiée.";
				this->Notification->notify(msg,false,ImVec4(1,0.5,0,1));
			}
			if (ma_device_init(NULL, &(this->phoneLineDeviceConfig), &(this->phoneLineDevice)) != MA_SUCCESS) {
				constexpr static char msg[]="Erreur lors de l'initialisation du thread audio.";
				this->Notification->notify(msg,false,ImVec4(1,0,0,1));
				this->uninitPhoneLine();
				return;
			}
			if (ma_device_start(&(this->phoneLineDevice)) != MA_SUCCESS) {
				constexpr static char msg[]="Erreur lors de l'activation du thread audio.";
				this->Notification->notify(msg,false,ImVec4(1,0,0,1));
				this->uninitPhoneLine();
				return;
			}
			this->phoneOutput=true;
			this->pCLKs->setAudioSampleRate(this->phoneLineDevice.sampleRate);
			printf("Sync emulator to phone line sample rate @%iHz\n",this->phoneLineDevice.sampleRate);
		}
		void uninitPhoneLine(){
			this->phoneOutput=false;
			if (ma_device_get_state(&(this->phoneLineDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->phoneLineDevice));
			if (ma_device_get_state(&(this->audioDevice))!=ma_device_state_uninitialized) ma_device_uninit(&(this->audioDevice));
			
			if (ma_device_init(NULL, &this->audioDeviceConfig, &(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to open playback device.\n");
				exit(-1);
			}
			if (ma_device_start(&(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to start playback device.\n");
				ma_device_uninit(&(this->audioDevice));
				exit(-1);
			}
			this->pCLKs->setAudioSampleRate(this->audioDevice.sampleRate);
			printf("Sync emulator to audio sample rate @%iHz\n",this->audioDevice.sampleRate);
		}
		
		void IOSelectionWidget(){
			ssize_t captureDeviceIndex=-1;
			for (ma_uint32 iDevice = 0; iDevice < this->captureDeviceCount; ++iDevice) {
				if (ma_device_id_equal(&(this->p_PARAMETERS->io.modem.audio.captureDeviceId),&(this->pCaptureDeviceInfos[iDevice].id))){
					this->p_PARAMETERS->io.modem.audio.captureDeviceId=this->pCaptureDeviceInfos[iDevice].id;
					captureDeviceIndex=iDevice;
					break;
				}
			}
			
			ssize_t playbackDeviceIndex=-1;
			for (ma_uint32 iDevice = 0; iDevice < this->playbackDeviceCount; ++iDevice) {
				if (ma_device_id_equal(&(this->p_PARAMETERS->io.modem.audio.playbackDeviceId),&(this->pPlaybackDeviceInfos[iDevice].id))){
					this->p_PARAMETERS->io.modem.audio.playbackDeviceId=this->pPlaybackDeviceInfos[iDevice].id;
					playbackDeviceIndex=iDevice;
					break;
				}
			}
			
			bool audioPhoneLineUsed=(ma_device_get_state(&(this->phoneLineDevice))!=ma_device_state_uninitialized);
			if (audioPhoneLineUsed) ImGui::BeginDisabled();
				ImGui::Text("Entrée audio:");
				ImGui::Indent();
				bool ret=ImGui::BeginCombo("##phone_line_input", captureDeviceIndex<0?"":this->pCaptureDeviceInfos[captureDeviceIndex].name,0);
				if (ImGui::IsItemActivated()) this->updateAudioDevices();
				if (ret){
					for (ma_uint32 iDevice = 0; iDevice < this->captureDeviceCount; ++iDevice) {
						if (ImGui::Selectable( this->pCaptureDeviceInfos[iDevice].name)){
							this->p_PARAMETERS->io.modem.audio.captureDeviceId=this->pCaptureDeviceInfos[iDevice].id;
							this->updateAudioDevices();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Unindent();
				
				ImGui::Text("Sortie audio:");
				ImGui::Indent();
				ret=ImGui::BeginCombo("##phone_line_output", playbackDeviceIndex<0?"":this->pPlaybackDeviceInfos[playbackDeviceIndex].name,0);
				if (ImGui::IsItemActivated()) this->updateAudioDevices();
				if (ret){
					for (ma_uint32 iDevice = 0; iDevice < this->playbackDeviceCount; ++iDevice) {
						if (ImGui::Selectable( this->pPlaybackDeviceInfos[iDevice].name)){
							this->p_PARAMETERS->io.modem.audio.playbackDeviceId=this->pPlaybackDeviceInfos[iDevice].id;
							this->updateAudioDevices();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Unindent();
			if (audioPhoneLineUsed) ImGui::EndDisabled();
		}
		
		void BufferWidget(){
			if (ma_device_get_state(&(this->audioDevice))==ma_device_state_started){
				ImGui::Text("Tampon audio (échantillons restants après lecture):");
			}
			if (ma_device_get_state(&(this->phoneLineDevice))==ma_device_state_started){
				ImGui::Text("Tampon ligne télphonique (échantillons restants après lecture):");
			}
			ImGui::PlotLines("##audio_buffer", this->samplesRemaining, sizeof(this->samplesRemaining)/sizeof(this->samplesRemaining[0]),0,NULL,0,(float)this->maxSamplesRemaining, ImVec2(-1, 80.0f));
		}
	private:
		static void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){//playback
			AudioIO* AIO=(AudioIO*)pDevice->pUserData;
			
			if (AIO->ab!=NULL){
				AIO->samplesRemaining[AIO->samplesRemainingIndex++]=(float)AIO->ab->AudioOut((float*)pOutput,frameCount);
			}
			
			if (AIO->pCLKs!=NULL){
				AIO->pCLKs->requestSamples(frameCount,AIO->maxSamplesRemaining);
			}
		}
		static void phone_line_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){//duplex
			AudioIO* AIO=(AudioIO*)pDevice->pUserData;
			
			if (AIO->plb!=NULL){
				AIO->samplesRemaining[AIO->samplesRemainingIndex++]=(float)AIO->plb->AudioIO((float*)pInput,(float*)pOutput,frameCount);
			}
			
			if (AIO->pCLKs!=NULL){
				AIO->pCLKs->requestSamples(frameCount,AIO->maxSamplesRemaining);
			}
		}
};

#endif