#include "thread_messaging.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"
#include "circuit/TS9347.h"
#include "circuit/80C32.h"
#include "circuit/CPLD.h"
#include "circuit/clocks.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <filesystem>
#include <ctime>
#include <cstdlib>

#include <vector>

#include "license.h"
#include "io/NotificationServer.h"
#include "MemoryWindow.h"
//#include "MainMenuWindow.h"
#include "Parameters.h"
#include "io/CRTRenderer.h"
#include "io/KeyboardIndicator.h"
#include "io/KeyboardInput.h"
#include "circuit/Keyboard.h"
#include "circuit/clocks.h"

#include "cJSON/cJSON.h"

#include "miniaudio/miniaudio.h"

//#include "io/TS7514Audio.h"
#include "circuit/SpeakerBuffer.h"
#include "circuit/BuzzerBuffer.h"
#include "circuit/TS7514.h"

#include "FileAccess.h"
#include "FileSelector.h"

//#include "circuit/DIN5/DIN5InterfaceLocalWebsocket.h"

struct audioContext{
	Clocks* pCLKs=NULL;
	SpeakerBuffer* spkb=NULL;
	BuzzerBuffer* bzb=NULL;
};



class M12Window{
	public:
		M12Window(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_GlobalState){
			this->PARAMETERS.p_gState=p_GlobalState;
			this->p_mb_circuit=p_mb_circuit;
			this->p_mb_video=p_mb_video;
			
			//glfw
			glfwSetErrorCallback(this->error_callback);
		 
			if (!glfwInit())
				exit(EXIT_FAILURE);
		 
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		 
			this->window = glfwCreateWindow(640, 480, this->PARAMETERS.info.title, NULL, NULL);
			if (!this->window)
			{
				glfwTerminate();
				exit(EXIT_FAILURE);
			}
		 
			glfwSetWindowUserPointer(this->window,this);
			glfwSetKeyCallback(this->window, this->key_callback);
			//glfwSetCharCallback(this->window, this->char_callback);
			glfwSetWindowCloseCallback(this->window, this->window_close_callback);
			glfwSetErrorCallback(this->window_error_callback);
		 
			glfwMakeContextCurrent(this->window);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			glfwSwapInterval(1);
			
			//imgui
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io=ImGui::GetIO();
			io.IniFilename=NULL;
			
			ImGui_ImplGlfw_InitForOpenGL(this->window,true);
			ImGui_ImplOpenGL3_Init();
		 
			//emulator display
			/*GLuint vertex_buffer;
			glGenBuffers(1, &vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);*/
			
			//RAM and ROM selection callbacks
			this->RAMS.setCallback([this](char* f){
				if (f==NULL) unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->eram));
				else loadRAM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->eram),f);
			});
			this->ROMS.setCallback([this](char* f){
				if (f==NULL){
					unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->erom));
					this->RAMS.setDir("./profils");
				}
				else{
					loadROM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->erom),f);
					std::filesystem::path p="./profils";
					p/=std::filesystem::path(f).filename();
					this->RAMS.setDir(p.string().c_str());
				}
			});
			
			//load parameters
			const char *fn_config="./config.json";
			if (std::filesystem::is_regular_file(fn_config)){
				FILE *f=fopen(fn_config,"r");
				fseek(f,0,SEEK_END);
				long fsize=ftell(f);
				fseek(f,0,SEEK_SET);
				char* config_raw=(char*)malloc(fsize);
				fread(config_raw,fsize,1,f);
				fclose(f);
				
				this->JSONConfig=cJSON_ParseWithLength(config_raw,fsize);
				free(config_raw);
				
				if (this->JSONConfig==NULL){
					const char* e=cJSON_GetErrorPtr();
					if (e!=NULL){
						printf("Error while parsing config.json before: %s\n",e);
						this->Notification.notify(5,ImVec4(1,0,0,1));
					}
				}
				else{
					//UI config
					cJSON* subconfig=cJSON_GetObjectItemCaseSensitive(this->JSONConfig,"UI");
					
					cJSON* c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"show menu");
					if (cJSON_IsBool(c_param)) this->PARAMETERS.imgui.show_menu=cJSON_IsTrue(c_param);
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"idle");
					if (cJSON_IsBool(c_param)) this->PARAMETERS.imgui.idle=cJSON_IsTrue(c_param);
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"DPI");
					if (cJSON_IsNumber(c_param)&&(c_param->valuedouble>=1)) ImGui::GetStyle().FontScaleDpi=c_param->valuedouble;
					
					//IO
					subconfig=cJSON_GetObjectItemCaseSensitive(this->JSONConfig,"IO");
					
					//Buzzer
					cJSON* subconfig2=cJSON_GetObjectItemCaseSensitive(subconfig,"Buzzer");
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig2,"notification");
					if (cJSON_IsBool(c_param)){
						this->PARAMETERS.io.buzzer.notify=cJSON_IsTrue(c_param);
					}
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig2,"volume");
					if (cJSON_IsNumber(c_param)) this->PARAMETERS.io.buzzer.volume=c_param->valuedouble;
					
					//Speaker
					subconfig2=cJSON_GetObjectItemCaseSensitive(subconfig,"Speaker");
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig2,"volume");
					if (cJSON_IsNumber(c_param)) this->PARAMETERS.io.speaker.volume=c_param->valuedouble;
					
					//CRT
					subconfig2=cJSON_GetObjectItemCaseSensitive(subconfig,"CRT");
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig2,"rgb");
					if (cJSON_IsBool(c_param)){
						this->PARAMETERS.io.crt.rgb=cJSON_IsTrue(c_param);
					}
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig2,"width factor");
					if (cJSON_IsNumber(c_param)){
						this->PARAMETERS.io.crt.width_factor=c_param->valuedouble;
					}
					
					//Other
					//"os rtc" loaded later
					
					//emulation config -> last
					subconfig=cJSON_GetObjectItemCaseSensitive(this->JSONConfig,"Emulation");
					
					//load rom
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"ROM file");
					if (cJSON_IsString(c_param)){
						this->ROMS.Select(c_param->valuestring);
					}
					//load ram
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"RAM file");
					if (cJSON_IsString(c_param)){
						if (strcmp(c_param->valuestring,"")==0){
							this->RAMS.Select(NULL);
						}
						this->RAMS.Select(c_param->valuestring);
					}
					else{
						this->RAMS.Select(NULL);
					}
					//start if possible later when emulator ready
				}
			}
			
			this->p_CRTout=new CRTRenderer(this->window,&(this->PARAMETERS));
			this->keyboardIndicator=new KeyboardIndicator();
			
			this->Notification.notify(4,ImVec4(0,1,1,1));
			
			//audio
			//see https://github.com/mackron/miniaudio/discussions/1084
			//see https://learn.microsoft.com/en-us/windows-hardware/drivers/audio/low-latency-audio
			this->audioDeviceConfig = ma_device_config_init(ma_device_type_playback);
			
			this->audioDeviceConfig.playback.format   = ma_format_f32;
			this->audioDeviceConfig.playback.channels = 1;
			
			this->audioDeviceConfig.sampleRate        = 0;//48000;
			
			this->audioDeviceConfig.dataCallback      = this->audio_data_callback;
			this->audioDeviceConfig.noFixedSizedCallback=true;
			this->audioDeviceConfig.periodSizeInFrames = 512;
			//this->audioDeviceConfig.wasapi.usage = ma_wasapi_usage_pro_audio;
			this->audioDeviceConfig.wasapi.noAutoConvertSRC = true;
			this->audioDeviceConfig.pUserData         = &(this->AC);

			if (ma_device_init(NULL, &(this->audioDeviceConfig), &(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to open playback device.\n");
				return;
			}

			printf("Device Name: %s\n", this->audioDevice.playback.name);
			
			if (ma_device_start(&(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to start playback device.\n");
				ma_device_uninit(&(this->audioDevice));
				return;
			}
		}
		~M12Window(){
			//save config
			if (this->JSONConfig!=NULL) cJSON_Delete(this->JSONConfig);
			
			cJSON* JSONConfigOut = cJSON_CreateObject();
			cJSON* JSONSubconfig1=cJSON_AddObjectToObject(JSONConfigOut,"UI");
			cJSON* JSONSubconfig2=NULL;
			if (JSONSubconfig1!=NULL){
				cJSON_AddBoolToObject(JSONSubconfig1,"show menu",this->PARAMETERS.imgui.show_menu);
				cJSON_AddBoolToObject(JSONSubconfig1,"idle",this->PARAMETERS.imgui.idle);
				cJSON_AddNumberToObject(JSONSubconfig1,"DPI",ImGui::GetStyle().FontScaleDpi);
			}
			JSONSubconfig1=cJSON_AddObjectToObject(JSONConfigOut,"Emulation");
			if (JSONSubconfig1!=NULL){
				cJSON_AddBoolToObject(JSONSubconfig1,"auto start",this->PARAMETERS.p_gState->minitelOn.load(std::memory_order_relaxed));
				if (this->ROMS.getSelected()==NULL) cJSON_AddStringToObject(JSONSubconfig1,"ROM file","");
				else cJSON_AddStringToObject(JSONSubconfig1,"ROM file",this->ROMS.getSelected());
				if (this->RAMS.getSelected()==NULL) cJSON_AddStringToObject(JSONSubconfig1,"RAM file","");
				else cJSON_AddStringToObject(JSONSubconfig1,"RAM file",this->RAMS.getSelected());
			}
			JSONSubconfig1=cJSON_AddObjectToObject(JSONConfigOut,"IO");
			if (JSONSubconfig1!=NULL){
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"DIN");
				if (JSONSubconfig2!=NULL){
					
				}
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"Phone line");
				if (JSONSubconfig2!=NULL){
					
				}
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"Buzzer");
				if (JSONSubconfig2!=NULL){
					cJSON_AddBoolToObject(JSONSubconfig2,"notification",this->PARAMETERS.io.buzzer.notify);
					cJSON_AddNumberToObject(JSONSubconfig2,"volume",this->PARAMETERS.io.buzzer.volume);
				}
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"Speaker");
				if (JSONSubconfig2!=NULL){
					cJSON_AddNumberToObject(JSONSubconfig2,"volume",this->PARAMETERS.io.speaker.volume);
				}
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"CRT");
				if (JSONSubconfig2!=NULL){
					cJSON_AddBoolToObject(JSONSubconfig2,"rgb",this->PARAMETERS.io.crt.rgb);
					cJSON_AddNumberToObject(JSONSubconfig2,"width factor",this->PARAMETERS.io.crt.width_factor);
				}
				JSONSubconfig2=cJSON_AddObjectToObject(JSONSubconfig1,"Other");
				if (JSONSubconfig2!=NULL){
					if (this->PARAMETERS.io.other.os_rtc!=NULL) cJSON_AddBoolToObject(JSONSubconfig2,"os rtc",this->PARAMETERS.io.other.os_rtc->load(std::memory_order_relaxed));
				}
			}
			char* configString=cJSON_Print(JSONConfigOut);
			FILE *f=fopen("./config.json","w");
			fwrite(configString,sizeof(char),strlen(configString),f);
			fclose(f);
			free((void*)configString);
			cJSON_Delete(JSONConfigOut);
			
			//audio
			ma_device_uninit(&(this->audioDevice));//uninit audio before stoping -> don't read deleted buffer
			//shutdown emulator
			//M12Window* p_M12Window=(M12Window*)glfwGetWindowUserPointer(window);
			this->PARAMETERS.p_gState->shutdown.store(true,std::memory_order_relaxed);
			//this->AC.pCLKs->requestSamples(1);
			
			//wait emulator response / emulator will timeout then read this->PARAMETERS.p_gState->shutdown and exit
			while (this->PARAMETERS.p_gState->minitelOn.load(std::memory_order_relaxed)){}
			
			//RAM and ROM files
			unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->eram));
			unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->erom));
			
			//imgui
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			//emulation screen
			delete this->p_CRTout;
			//indicators
			delete this->keyboardIndicator;
			//glfw
			glfwDestroyWindow(this->window);
			glfwTerminate();
		}
		void Loop(){
			//check messages
			while (!glfwWindowShouldClose(this->window))
			{
				thread_message ms;
				while (this->p_mb_video->receive(&ms)){
					switch(ms.cmd){
						case ERAM:
							this->PARAMETERS.debug.eram.mem=((SRAM_64k*)ms.p)->RAM;
							this->PARAMETERS.debug.eram.op=&(((SRAM_64k*)ms.p)->last_memory_operation);
							this->PARAMETERS.debug.eram.mem_size=ERAM_SIZE;
							break;
						case EROM:
							this->PARAMETERS.debug.erom.mem=((ROM_256k*)ms.p)->eROM;
							this->PARAMETERS.debug.erom.op=&(((ROM_256k*)ms.p)->last_memory_operation);
							this->PARAMETERS.debug.erom.mem_size=EROM_SIZE;
							break;
						case VC:
						{
							this->PARAMETERS.debug.vram.mem=((TS9347wVRAM*)ms.p)->VRAM;
							this->PARAMETERS.debug.vram.mem_size=VRAM_SIZE;
							
							this->PARAMETERS.debug.vreg.STATUS=&(((TS9347wVRAM*)ms.p)->STATUS);
							this->PARAMETERS.debug.vreg.COMMAND=&(((TS9347wVRAM*)ms.p)->Rx[0]);
							this->PARAMETERS.debug.vreg.R1=&(((TS9347wVRAM*)ms.p)->Rx[1]);
							this->PARAMETERS.debug.vreg.R2=&(((TS9347wVRAM*)ms.p)->Rx[2]);
							this->PARAMETERS.debug.vreg.R3=&(((TS9347wVRAM*)ms.p)->Rx[3]);
							this->PARAMETERS.debug.vreg.R4=&(((TS9347wVRAM*)ms.p)->Rx[4]);
							this->PARAMETERS.debug.vreg.R5=&(((TS9347wVRAM*)ms.p)->Rx[5]);
							this->PARAMETERS.debug.vreg.R6=&(((TS9347wVRAM*)ms.p)->Rx[6]);
							this->PARAMETERS.debug.vreg.R7=&(((TS9347wVRAM*)ms.p)->Rx[7]);
							this->PARAMETERS.debug.vreg.DOR=&(((TS9347wVRAM*)ms.p)->DOR);
							this->PARAMETERS.debug.vreg.ROR=&(((TS9347wVRAM*)ms.p)->ROR);
							this->PARAMETERS.debug.vreg.TGS=&(((TS9347wVRAM*)ms.p)->TGS);
							this->PARAMETERS.debug.vreg.PAT=&(((TS9347wVRAM*)ms.p)->PAT);
							this->PARAMETERS.debug.vreg.MAT=&(((TS9347wVRAM*)ms.p)->MAT);
							
							const char* path="./ressources/TS9347_Texture_Character_Set_Datasheet.bmp";
							int width, height, nrChannels;
							unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 1);
							int nW=width/8;
							int nH=height/10;
							int n=288;
							unsigned char d[n*10]={0};
							if ((nW*nH)<n) n=nW*nH;
							if (nH>nW){
								for (int i=0;i<n;i++){//char i
									int j=(i%nH)*10*width+(i/nH)*8;//char 0,0 position
									for (int s=0;s<10;s++){//slice
										unsigned char sd=0;
										for (int k=0;k<8;k++){//pixel in slice
											sd=(sd>>1)|((data[j+s*width+k]==0)?0:0x80);
										}
										d[i*10+s]=sd;
									}
								}
							}
							else{
								for (int i=0;i<n;i++){//char i
									int j=(i%nW)*8+(i/nW)*10*width;//char 0,0 position
									for (int s=0;s<10;s++){//slice
										unsigned char sd=0;
										for (int k=0;k<8;k++){//pixel in slice
											sd=(sd>>1)|((data[j+s*width+k]==0)?0:0x80);
										}
										d[i*10+s]=sd;
									}
								}
							}
							
							((TS9347wVRAM*)ms.p)->setROMCharset(d);
								
							stbi_image_free(data);
							break;
						}
						case CRT_BUFFER:
							this->p_CRTout->setBuffer((CRTBuffer*)ms.p);
							break;
						case SPEAKER_BUFFER:
							this->AC.spkb=(SpeakerBuffer*)ms.p;
							this->AC.spkb->setVolumeLog(this->PARAMETERS.io.speaker.volume);
							break;
						case BUZZER_BUFFER:
							this->AC.bzb=(BuzzerBuffer*)ms.p;
							this->AC.bzb->setVolumeLog(this->PARAMETERS.io.buzzer.volume);
							break;
						case UC:
						{
							m80C32* uc=(m80C32*)ms.p;
							this->PARAMETERS.debug.iram.mem=uc->iRAM;
							this->PARAMETERS.debug.iram.op=&(uc->last_memory_operation);
							this->PARAMETERS.debug.iram.mem_size=IRAM_SIZE;
							this->PARAMETERS.debug.sfr.ACC=&(uc->SFR[uc->ACC&0x7F]);
							this->PARAMETERS.debug.sfr.B=&(uc->SFR[uc->B&0x7F]);
							this->PARAMETERS.debug.sfr.DPH=&(uc->SFR[uc->DPH&0x7F]);
							this->PARAMETERS.debug.sfr.DPL=&(uc->SFR[uc->DPL&0x7F]);
							this->PARAMETERS.debug.sfr.IE=&(uc->SFR[uc->IE&0x7F]);
							this->PARAMETERS.debug.sfr.IP=&(uc->SFR[uc->IP&0x7F]);
							this->PARAMETERS.debug.sfr.P0=&(uc->SFR[uc->P0&0x7F]);
							this->PARAMETERS.debug.sfr.P1=&(uc->SFR[uc->P1&0x7F]);
							this->PARAMETERS.debug.sfr.P2=&(uc->SFR[uc->P2&0x7F]);
							this->PARAMETERS.debug.sfr.P3=&(uc->SFR[uc->P3&0x7F]);
							this->PARAMETERS.debug.sfr.PCON=&(uc->SFR[uc->PCON&0x7F]);
							this->PARAMETERS.debug.sfr.PSW=&(uc->SFR[uc->PSW&0x7F]);
							this->PARAMETERS.debug.sfr.RCAP2H=&(uc->SFR[uc->RCAP2H&0x7F]);
							this->PARAMETERS.debug.sfr.RCAP2L=&(uc->SFR[uc->RCAP2L&0x7F]);
							this->PARAMETERS.debug.sfr.SBUF=&(uc->SFR[uc->SBUF&0x7F]);
							this->PARAMETERS.debug.sfr.SCON=&(uc->SFR[uc->SCON&0x7F]);
							this->PARAMETERS.debug.sfr.SP=&(uc->SFR[uc->SP&0x7F]);
							this->PARAMETERS.debug.sfr.TCON=&(uc->SFR[uc->TCON&0x7F]);
							this->PARAMETERS.debug.sfr.T2CON=&(uc->SFR[uc->T2CON&0x7F]);
							this->PARAMETERS.debug.sfr.TH0=&(uc->SFR[uc->TH0&0x7F]);
							this->PARAMETERS.debug.sfr.TH1=&(uc->SFR[uc->TH1&0x7F]);
							this->PARAMETERS.debug.sfr.TH2=&(uc->SFR[uc->TH2&0x7F]);
							this->PARAMETERS.debug.sfr.TL0=&(uc->SFR[uc->TL0&0x7F]);
							this->PARAMETERS.debug.sfr.TL1=&(uc->SFR[uc->TL1&0x7F]);
							this->PARAMETERS.debug.sfr.TL2=&(uc->SFR[uc->TL2&0x7F]);
							this->PARAMETERS.debug.sfr.TMOD=&(uc->SFR[uc->TMOD&0x7F]);
							break;
						}
						case CPLD:
							this->PARAMETERS.io.other.os_rtc=&((MBSL_4000FH5_5*)ms.p)->OS_RTC;
							
							//load parameter when accessible
							{
								if (this->JSONConfig!=NULL){
									cJSON* json_o=cJSON_GetObjectItemCaseSensitive(this->JSONConfig,"IO");
									json_o=cJSON_GetObjectItemCaseSensitive(json_o,"Other");
									json_o=cJSON_GetObjectItemCaseSensitive(json_o,"os rtc");
									if (cJSON_IsBool(json_o)){
										this->PARAMETERS.io.other.os_rtc->store(cJSON_IsTrue(json_o),std::memory_order_relaxed);
									}
								}
							}
							break;
						case KEYBOARD:
							this->keyboardIndicator->setKeyboard((Keyboard*)ms.p);
							break;
						case NOTIFICATION:
							this->Notification.notify((const char*)ms.p,true);
							//imgui_notify((const char*)ms.p,true);
							//fprintf(stdout,"notification from thread\n");
							break;
						case NOTIFICATION_BUZZER:
							if (this->PARAMETERS.io.buzzer.notify) this->Notification.notify(0,ImVec4(1,0.5,0,1));
							break;
						case NOTIFICATION_REBOOT:
							this->Notification.notify(1,ImVec4(0,1,1,1));
							break;
						case NOTIFICATION_RED:this->Notification.notify((const char*)ms.p,true,ImVec4(1,0,0,1));break;
						case NOTIFICATION_GREEN:this->Notification.notify((const char*)ms.p,true,ImVec4(0,1,0,1));break;
						case NOTIFICATION_BLUE:this->Notification.notify((const char*)ms.p,true,ImVec4(0,0,1,1));break;
						case NOTIFICATION_ORANGE:this->Notification.notify((const char*)ms.p,true,ImVec4(1,0.5,0,1));break;
						case NOTIFICATION_YELLOW:this->Notification.notify((const char*)ms.p,true,ImVec4(1,1,0,1));break;
						case NOTIFICATION_CYAN:this->Notification.notify((const char*)ms.p,true,ImVec4(0,1,1,1));break;
						case NOTIFICATION_PURPLE:this->Notification.notify((const char*)ms.p,true,ImVec4(1,0,1,1));break;
						case CRT_POWER_ON:break;
						case CRT_POWER_OFF:break;
						case MODEM:
							this->PARAMETERS.debug.mreg.RPROG=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RPROG]);
							this->PARAMETERS.debug.mreg.RDTMF=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RDTMF]);
							this->PARAMETERS.debug.mreg.RATE=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RATE]);
							this->PARAMETERS.debug.mreg.RWLO=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RWLO]);
							this->PARAMETERS.debug.mreg.RPTF=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RPTF]);
							this->PARAMETERS.debug.mreg.RPRF=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RPRF]);
							this->PARAMETERS.debug.mreg.RHDL=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RHDL]);
							this->PARAMETERS.debug.mreg.RPRX=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RPRX]);
							break;
						case CLOCK:
							this->AC.pCLKs=(Clocks*)ms.p;
							this->AC.pCLKs->setAudioSampleRate(this->audioDevice.sampleRate);
							printf("Sync emulator to audio sample rate @%iHz\n",this->audioDevice.sampleRate);
							this->AC.pCLKs->requestSamples(256,256);//TODO: 256->buffer length
							break;
						case EMULATOR_READY:
							{
								if (this->JSONConfig!=NULL){
									cJSON* json_subconfig=cJSON_GetObjectItemCaseSensitive(this->JSONConfig,"Emulation");
									cJSON* json_o=cJSON_GetObjectItemCaseSensitive(json_subconfig,"RAM file");
									bool start=true;
									if (cJSON_IsString(json_o)){
										start=start||!(strcmp(json_o->valuestring,"")==0||strcmp(json_o->valuestring,this->RAMS.getSelected())==0);
									}
									else start=false;
									if(this->ROMS.getSelected()==NULL) start=false;
									json_o=cJSON_GetObjectItemCaseSensitive(json_subconfig,"auto start");
									if (cJSON_IsBool(json_o)&&cJSON_IsTrue(json_o)&&start){
										ms.cmd=EMU_ON;
										this->p_mb_circuit->send(&ms);	
									}
								}
							}
							break;
						default:
							fprintf(stdout,"unknown cmd %i\n",ms.cmd);
							break;
					}
				}
				
				int width, height;
				glfwGetFramebufferSize(this->window, &width, &height);
				glClear(GL_COLOR_BUFFER_BIT);
				glViewport(0,0,width,height);
				
				//create frame imgui
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				//ImGui::ShowDemoWindow(NULL);
				if (this->PARAMETERS.imgui.show_menu) this->mainMenuWindow();
				if (this->PARAMETERS.debug.eram.mem!=NULL&&this->PARAMETERS.debug.eram.show) memoryWindow("RAM externe",&(this->PARAMETERS.debug.eram));
				if (this->PARAMETERS.debug.erom.mem!=NULL&&this->PARAMETERS.debug.erom.show) memoryWindow("ROM externe",&(this->PARAMETERS.debug.erom));
				if (this->PARAMETERS.debug.iram.mem!=NULL&&this->PARAMETERS.debug.iram.show) memoryWindow("RAM interne",&(this->PARAMETERS.debug.iram));
				if (this->PARAMETERS.debug.sfr.show) sfr80C32Window(&(this->PARAMETERS.debug.sfr));
				if (this->PARAMETERS.debug.vram.mem!=NULL&&this->PARAMETERS.debug.vram.show) memoryWindow("VRAM",&(this->PARAMETERS.debug.vram));
				if (this->PARAMETERS.debug.vreg.show) regTS9347Window(&(this->PARAMETERS.debug.vreg));
				if (this->PARAMETERS.debug.mreg.show) regTS7514Window(&(this->PARAMETERS.debug.mreg));
				this->Notification.notification_window();
				this->keyboardIndicator->window();
				
				//render frame emulator
				this->p_CRTout->render();
		 
				//render frame imgui
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		 
				glfwSwapBuffers(this->window);
				if (this->PARAMETERS.imgui.idle) glfwWaitEventsTimeout(0.05);
				else glfwPollEvents();
			}
		}
	private:
		GLFWwindow* window;
		Parameters PARAMETERS;
		const Parameters default_parameters;
		RAMSelector RAMS=RAMSelector("./profils/");
		ROMSelector ROMS=ROMSelector("./rom/");
		NotificationServer Notification;
		CRTRenderer* p_CRTout;
		KeyboardIndicator* keyboardIndicator;
		Mailbox* p_mb_circuit;
		Mailbox* p_mb_video;
		
		audioContext AC;
		ma_device_config audioDeviceConfig;
		ma_device audioDevice;
		cJSON* JSONConfig=NULL;
		
		void mainMenuWindow(){
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
			ImVec2 menuSize=ImGui::GetMainViewport()->Size;
			menuSize[0]*=0.8;
			menuSize[1]*=0.8;
			ImGui::SetNextWindowSize(menuSize, ImGuiCond_Always);
			ImGui::Begin("Menu",&(this->PARAMETERS.imgui.show_menu),ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove);
			ImGui::Text("Appuyez sur F1 pour afficher/cacher le menu");
			if (ImGui::BeginTabBar("MenuTabBar", ImGuiTabBarFlags_None)){
				
				if (ImGui::BeginTabItem("Emulation")){
					
					ImGui::SeparatorText("Contrôle de l'émulateur");
					if(this->PARAMETERS.p_gState->minitelOn.load(std::memory_order_relaxed)){
						ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0,0.8,0,1));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0,1,0,1));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0,1,0,1));
						if(ImGui::Button("(  O)")){
							thread_message ms;
							ms.cmd=EMU_OFF;
							this->p_mb_circuit->send(&ms);
						}
						ImGui::PopStyleColor(3);
						ImGui::SameLine();
						ImGui::Text("L'émulateur est en marche");
					}
					else{
						ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.8,0,0,1));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(1,0,0,1));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(1,0,0,1));
						if(ImGui::Button("(O  )")){
							thread_message ms;
							ms.cmd=EMU_ON;
							this->p_mb_circuit->send(&ms);
						}
						ImGui::PopStyleColor(3);
						ImGui::SameLine();
						ImGui::Text("L'émulateur est à l'arrêt");
					}
					
					bool disable_load_memory=this->PARAMETERS.p_gState->minitelOn.load(std::memory_order_relaxed);
					if (disable_load_memory) ImGui::BeginDisabled();
						
						ImGui::SeparatorText("Mémoire ROM du minitel");
						this->ROMS.widget();
						
						ImGui::SeparatorText("Profil - Mémoire RAM du minitel");
						this->RAMS.widget();
					
					if (disable_load_memory) ImGui::EndDisabled();
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Entrées/Sorties")){
					/*static ma_context audio_context=initAudio();
					static ma_device_info* pPlaybackDeviceInfos;
					static ma_uint32 playbackDeviceCount;
					static ma_device_info* pCaptureDeviceInfos;
					static ma_uint32 captureDeviceCount;*/
					
					ImGui::SeparatorText("Clavier");
					ImGui::Text("L'émulateur est conçu pour un clavier AZERTY avec un pavé numérique (de préférence) et verr num désactivé.");
					ImGui::Text("Les touches différentes du minitel sont les suivantes:");
					
					if (ImGui::BeginTable("keyTable",3,ImGuiTableFlags_Borders|ImGuiTableFlags_SizingStretchSame,ImVec2(0, 0))){
						ImGui::TableSetupColumn("Clavier", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("Pavé numérique", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("Correspondance", ImGuiTableColumnFlags_None);
						ImGui::TableHeadersRow();
						
						const char *key1[17]={"_","Escape","Alt","²","Tab","=","Backspace","F2","F3","F4","F5","F6","F7","F8","F9","*","ù"};
						const char *key2[17]={"","","","","","","","","","","","","","","","*","/"};
						const char *key3[17]={"!","Esc","Fnct","\'On/Off\'","Connex/Fin","Mem","\'Haut Parleur\'","Sommaire","Guide","Annulation","Correction","Retour","Suite","Répétition","Envoi","*","#"};
						for (int i=0;i<17;i++){
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text(key1[i]);
							ImGui::TableSetColumnIndex(1);
							ImGui::Text(key2[i]);
							ImGui::TableSetColumnIndex(2);
							ImGui::Text(key3[i]);
						}
						
						ImGui::EndTable();
					}
					ImGui::Text("F1 permet d'afficher ce menu.");
					ImGui::Text("F10 permet de faire une capture d'écran.");
					
					ImGui::SeparatorText("Prise péri-informatique");
					
					if (ImGui::CollapsingHeader("Connexion Websocket")){
						ImGui::Indent();
						ImGui::Text("Accès au service: Shift+Connexion/Fin W");
						ImGui::Text("Arrêt du service: Shift+Connexion/Fin x2");
						ImGui::TextDisabled("Certaines associations de paramètres peuvent ne pas être interprétés par le minitel.");
						ImGui::TextDisabled("Les Websockets sécurisés (wss) sont aussi pris en charge.");
						ImGui::Unindent();
					}
					//ImGui::Checkbox("Afficher les notifications##peri",&(this->PARAMETERS.io.peri.notify_state));
					
					ImGui::SeparatorText("Modem");
					/*static int modem_io=0;
					ImGui::RadioButton("Débranché##modem", &modem_io, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Socket UNIX##modem", &modem_io, 1);
					ImGui::SameLine();
					ImGui::RadioButton("Prise audio##modem", &modem_io, 2);
					if (modem_io==1){
						ImGui::Indent();
						ImGui::Text("Socket:");
						ImGui::Unindent();
					}
					if (modem_io==2){
						ImGui::Indent();
						ImGui::Text("Entrée:");
						static ma_uint32 modem_audio_i=0;
						static bool modem_i_combo_state_previous=true;
						bool modem_i_combo_state=ImGui::BeginCombo("##modem_i", (modem_audio_i>=captureDeviceCount)?"":pCaptureDeviceInfos[modem_audio_i].name, 0);
						if (modem_i_combo_state!=modem_i_combo_state_previous){
							ma_result result;
							if (modem_audio_i<captureDeviceCount){
								ma_device_id id=pCaptureDeviceInfos[modem_audio_i].id;
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
								modem_audio_i=0;
								for(ma_uint32 i=0;i<captureDeviceCount;i++){
									if(ma_device_id_equal(&(pCaptureDeviceInfos[i].id),&id)){
										modem_audio_i=i;
										break;
									}
								}
							}
							else{
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
							}
							if (result != MA_SUCCESS) {
								printf("Failed to retrieve device information.\n");
							}
							modem_i_combo_state_previous=modem_i_combo_state;
						}
						if(modem_i_combo_state){
							for (unsigned int i=0;i<captureDeviceCount;i++) if(i!=modem_audio_i&&ImGui::Selectable(pCaptureDeviceInfos[i].name)) modem_audio_i=i;
							ImGui::EndCombo();
						}
						ImGui::Text("Sortie:");
						static ma_uint32 modem_audio_o=0;
						static bool modem_o_combo_state_previous=true;
						bool modem_o_combo_state=ImGui::BeginCombo("##modem_o", (modem_audio_o>=playbackDeviceCount)?"":pPlaybackDeviceInfos[modem_audio_o].name, 0);
						if (modem_o_combo_state!=modem_o_combo_state_previous){
							ma_result result;
							if (modem_audio_o<playbackDeviceCount){
								ma_device_id id=pPlaybackDeviceInfos[modem_audio_o].id;
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
								modem_audio_o=0;
								for(ma_uint32 i=0;i<playbackDeviceCount;i++){
									if(ma_device_id_equal(&(pPlaybackDeviceInfos[i].id),&id)){
										modem_audio_o=i;
										break;
									}
								}
							}
							else{
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
							}
							if (result != MA_SUCCESS) {
								printf("Failed to retrieve device information.\n");
							}
							modem_o_combo_state_previous=modem_o_combo_state;
						}
						if(modem_o_combo_state){
							for (unsigned int i=0;i<playbackDeviceCount;i++) if(i!=modem_audio_o&&ImGui::Selectable(pPlaybackDeviceInfos[i].name)) modem_audio_o=i;
							ImGui::EndCombo();
						}
						ImGui::Unindent();
					}
					ImGui::Checkbox("Afficher les notifications##modem",&(this->PARAMETERS.io.modem.notify_state));*/
					
					ImGui::SeparatorText("Buzzer");
					/*static int buzzer_o=0;
					ImGui::RadioButton("Débranché##buzzer", &buzzer_o, 0);
					ImGui::SameLine();
					ImGui::RadioButton("Prise audio##buzzer", &buzzer_o, 1);
					if (buzzer_o==1){
						ImGui::Indent();
						ImGui::Text("Sortie:");
						static ma_uint32 buzzer_audio_o=0;
						static bool buzzer_o_combo_state_previous=true;
						bool buzzer_o_combo_state=ImGui::BeginCombo("##buzzer", (buzzer_audio_o>=playbackDeviceCount)?"":pPlaybackDeviceInfos[buzzer_audio_o].name, 0);
						if (buzzer_o_combo_state!=buzzer_o_combo_state_previous){
							ma_result result;
							if (buzzer_audio_o<playbackDeviceCount){
								ma_device_id id=pPlaybackDeviceInfos[buzzer_audio_o].id;
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
								buzzer_audio_o=0;
								for(ma_uint32 i=0;i<playbackDeviceCount;i++){
									if(ma_device_id_equal(&(pPlaybackDeviceInfos[i].id),&id)){
										buzzer_audio_o=i;
										break;
									}
								}
							}
							else{
								result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
							}
							if (result != MA_SUCCESS) {
								printf("Failed to retrieve device information.\n");
							}
							buzzer_o_combo_state_previous=buzzer_o_combo_state;
						}
						if(buzzer_o_combo_state){
							for (unsigned int i=0;i<playbackDeviceCount;i++) if(i!=buzzer_audio_o&&ImGui::Selectable(pPlaybackDeviceInfos[i].name)) buzzer_audio_o=i;
							ImGui::EndCombo();
						}
						ImGui::Unindent();
					}*/
					if (this->AC.bzb!=NULL){
						ImGui::Text("Volume:");
						ImGui::Indent();
						if (ImGui::SliderFloat("##buzzer_volume", &(this->PARAMETERS.io.buzzer.volume), 0., 1., "%.3f")) this->AC.bzb->setVolumeLog(this->PARAMETERS.io.buzzer.volume);
						ImGui::Unindent();
					}
					ImGui::Checkbox("Afficher les notifications##buzzer",&(this->PARAMETERS.io.buzzer.notify));
					
					ImGui::SeparatorText("Haut parleur");
					if (this->AC.spkb!=NULL){
						ImGui::Text("Volume:");
						ImGui::Indent();
						if (ImGui::SliderFloat("##speaker_volume", &(this->PARAMETERS.io.speaker.volume), 0., 1., "%.3f")) this->AC.spkb->setVolumeLog(this->PARAMETERS.io.speaker.volume);
						ImGui::Unindent();
					}
					
					ImGui::SeparatorText("CRT");
					ImGui::Text("Facteur d'étirement de l'image:");
					ImGui::Indent();
					ImGui::SliderFloat("##crt_width", &(this->PARAMETERS.io.crt.width_factor), 0.5, 1.5, "x%.3f");
					ImGui::SameLine();
					if(ImGui::Button("Réinitialiser")) this->PARAMETERS.io.crt.width_factor=this->default_parameters.io.crt.width_factor;
					ImGui::Unindent();
					ImGui::Checkbox("Sortie vidéo RGB",&(this->PARAMETERS.io.crt.rgb));
					ImGui::SameLine();
					ImGui::TextDisabled("(hack)");
					/*static bool crt_filter=false;
					ImGui::Checkbox("Filtre vidéo CRT",&crt_filter);*/
					
					ImGui::SeparatorText("Divers");
					if (this->PARAMETERS.io.other.os_rtc!=NULL){
						static bool os_rtc=this->PARAMETERS.io.other.os_rtc->load(std::memory_order_relaxed);
						ImGui::Checkbox("Utiliser la date de l'ordinateur pour le RTC",&(os_rtc));
						this->PARAMETERS.io.other.os_rtc->store(os_rtc,std::memory_order_relaxed);
						ImGui::SameLine();
						ImGui::TextDisabled("(hack)");
						
					}
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("UI")){
					ImGui::Text("DPI");
					ImGui::SameLine();
					if(ImGui::Button("-##dpi")) ImGui::GetStyle().FontScaleDpi=(ImGui::GetStyle().FontScaleDpi<=1)?1:(ImGui::GetStyle().FontScaleDpi-1);
					ImGui::SameLine();
					if(ImGui::Button("+##dpi")) ImGui::GetStyle().FontScaleDpi+=1;
					ImGui::Checkbox("Rafraichissement d'image dynamique",&(this->PARAMETERS.imgui.idle));
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Débuggage")){
					//static bool sbs=false;
					bool sbs=this->PARAMETERS.p_gState->stepByStep.load(std::memory_order_relaxed);
					ImGui::SeparatorText("Exécution");
					ImGui::Checkbox("Pauser l'exécution",&sbs);
					this->PARAMETERS.p_gState->stepByStep.store(sbs,std::memory_order_relaxed);
					if (sbs){
						ImGui::Indent();
						if(ImGui::Button("Instruction suivante")){
							thread_message ms;
							ms.cmd=EMU_NEXT_STEP;
							p_mb_circuit->send(&ms);
						}
						ImGui::Unindent();
					}
					
					ImGui::SeparatorText("Visualisation des espaces mémoires");
					if (this->PARAMETERS.debug.eram.mem!=NULL){
						ImGui::Checkbox("Afficher le contenu de la RAM externe",&(this->PARAMETERS.debug.eram.show));
					}
					if (this->PARAMETERS.debug.erom.mem!=NULL){
						ImGui::Checkbox("Afficher le contenu de la ROM externe",&(this->PARAMETERS.debug.erom.show));
					}
					if (this->PARAMETERS.debug.iram.mem!=NULL){
						ImGui::Checkbox("Afficher le contenu de la RAM interne",&(this->PARAMETERS.debug.iram.show));
					}
					ImGui::Checkbox("Afficher les registres spéciaux du microcontrôleur",&(this->PARAMETERS.debug.sfr.show));
					if (this->PARAMETERS.debug.vram.mem!=NULL){
						ImGui::Checkbox("Afficher le contenu de la RAM vidéo",&(this->PARAMETERS.debug.vram.show));
					}
					ImGui::Checkbox("Afficher les registres de la puce vidéo",&(this->PARAMETERS.debug.vreg.show));
					ImGui::Checkbox("Afficher les registres du modem",&(this->PARAMETERS.debug.mreg.show));
					
					ImGui::SeparatorText("Statistiques");
					ImGui::Text("Rafraichissement d'image: %.1f FPS",ImGui::GetIO().Framerate);
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("À propos")){
					ImGui::Text(this->PARAMETERS.info.title);
					ImGui::SeparatorText("Développeurs");
					ImGui::Text(this->PARAMETERS.info.programmers);
					ImGui::SeparatorText("Bibliothèques");
					for (License l:this->PARAMETERS.info.lib_licenses){
						if (ImGui::TreeNode(l.title)){
							ImGui::TextUnformatted(l.content);
							ImGui::TreePop();
						}
					}
					ImGui::SeparatorText("Polices de caractères");
					for (License l:this->PARAMETERS.info.font_licenses){
						if (ImGui::TreeNode(l.title)){
							ImGui::TextUnformatted(l.content);
							ImGui::TreePop();
						}
					}
					
					ImGui::EndTabItem();
				}
			
				ImGui::EndTabBar();
			}
			ImGui::End();
		}
		
		void takeScreenshot(){
			constexpr char dir[]="./captures d'écran/";
			if (!std::filesystem::is_directory(dir)) std::filesystem::create_directories(dir);
			time_t timestamp=time(NULL);
			struct tm* pTime=localtime(&timestamp);
			char filename[80];
			strftime(filename,80,"minitel_%F_%H_%M_%S.png",pTime);
			char* path=(char*)malloc(sizeof(dir)+strlen(filename)+1);
			strcpy(path,dir);
			strcat(path,filename);
			
			int width, height;
			glfwGetFramebufferSize(this->window, &width, &height);
			unsigned char* pixels=(unsigned char*)malloc(3*width*height*sizeof(unsigned char));
			glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			stbi_flip_vertically_on_write(1);
			stbi_write_png(path, width, height, 3, pixels, 3*width);
			free(pixels);
			
			constexpr char screenshot[]="Capture d'écran enregistrée dans le fichier:\n";
			char* notif=(char*)malloc(sizeof(screenshot)+strlen(path)+1);
			strcpy(notif,screenshot);
			strcat(notif,path);
			this->Notification.notify(notif,true,ImVec4(0,1,1,1));
			
			free(path);
		}
		
		static void error_callback(int error, const char* description){
			fprintf(stderr, "Error: %s\n", description);
		}
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
			M12Window* p_M12Window=(M12Window*)glfwGetWindowUserPointer(window);
			//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
			if (key==GLFW_KEY_F1&&action==GLFW_PRESS) p_M12Window->PARAMETERS.imgui.show_menu=!p_M12Window->PARAMETERS.imgui.show_menu;
			if (key==GLFW_KEY_F10&&action==GLFW_PRESS) p_M12Window->takeScreenshot();
			ImGuiIO& io=ImGui::GetIO();
			KeyboardInput(p_M12Window->p_mb_circuit,!io.WantCaptureKeyboard,scancode,action,mods);
		}
		//static void char_callback(GLFWwindow* window, unsigned int codepoint){}
		static void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
			audioContext* AC=(audioContext*)pDevice->pUserData;
			
			if (AC->pCLKs!=NULL){
				AC->pCLKs->requestSamples(frameCount,1024);//TODO: 512->buffer length / windows limitation in shared mode (480 @48000Hz) /limit max->800 @48000 (screen update @~60fps)
			}
			
			if (AC->spkb!=NULL){
				AC->spkb->AudioOut((float*)pOutput,frameCount);
			}
			if (AC->bzb!=NULL){
				AC->bzb->AudioOut((float*)pOutput,frameCount);
			}
		}
		static void window_close_callback(GLFWwindow* window){
			fprintf(stdout,"Close callback\n");
			/////////////////////////////////////////////////////
		}
		static void window_error_callback(int error, const char* description){
			fprintf(stderr, "Error: %s\n", description);
		}
};





void thread_video_main(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_gState){
	M12Window m12w=M12Window(p_mb_circuit,p_mb_video,p_gState);
	m12w.Loop();
}