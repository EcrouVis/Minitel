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

#include <filesystem>
#include <cstdlib>

#include <vector>

#include "license.h"
#include "io/NotificationServer.h"
#include "MemoryWindow.h"
#include "MainMenuWindow.h"
#include "Parameters.h"
#include "io/TS9347Renderer.h"
#include "io/KeyboardIndicator.h"
#include "io/KeyboardInput.h"
#include "circuit/Keyboard.h"
#include "circuit/clocks.h"

#include "cJSON/cJSON.h"

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include "io/TS7514Audio.h"
#include "circuit/TS7514.h"

#include "FileAccess.h"

struct audioContext{
	WLOConf* wloc=NULL;
	Clocks* pCLKs=NULL;
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
		 
			glfwMakeContextCurrent(this->window);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			glfwSwapInterval(1);
			
			//imgui
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io=ImGui::GetIO();
			io.IniFilename=NULL;
			
			ImGui_ImplGlfw_InitForOpenGL(window,true);
			ImGui_ImplOpenGL3_Init();
		 
			//emulator display
			GLuint vertex_buffer;
			glGenBuffers(1, &vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			
			//load parameters
			const char *fn_config="config.json";
			if (std::filesystem::is_regular_file(fn_config)){
				FILE *f=fopen(fn_config,"rb");
				fseek(f,0,SEEK_END);
				long fsize=ftell(f);
				fseek(f,0,SEEK_SET);
				char* config_raw=(char*)malloc(fsize);
				fread(config_raw,fsize,1,f);
				fclose(f);
				
				cJSON* config=cJSON_ParseWithLength(config_raw,fsize);
				if (config==NULL){
					const char* e=cJSON_GetErrorPtr();
					if (e!=NULL){
						printf("Error while parsing config.json before: %s\n",e);
						this->Notification.notify(5,ImVec4(1,0,0,1));
					}
				}
				else{
					thread_message ms;
					//UI config
					cJSON* subconfig=cJSON_GetObjectItemCaseSensitive(config,"UI");
					
					cJSON* c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"show_menu");
					if (cJSON_IsBool(c_param)) this->PARAMETERS.imgui.show_menu=cJSON_IsTrue(c_param);
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"idle");
					if (cJSON_IsBool(c_param)) this->PARAMETERS.imgui.idle=cJSON_IsTrue(c_param);
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"DPI");
					if (cJSON_IsNumber(c_param)&&(c_param->valuedouble>=1)) ImGui::GetStyle().FontScaleDpi=c_param->valuedouble;
					
					//emulation config
					subconfig=cJSON_GetObjectItemCaseSensitive(config,"Emulation");
					
					/*bool start_emu=true;
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"RAM_file");
					if (cJSON_IsString(c_param)){
						if (std::filesystem::is_regular_file(c_param->valuestring)){
							ms.cmd=LOAD_ERAM;
							ms.p=malloc(strlen(c_param->valuestring)+1);
							strcpy((char*)ms.p,c_param->valuestring);
							((char*)ms.p)[strlen(c_param->valuestring)]=0;
							this->p_mb_circuit->send(&ms);
						}
						else if (strcmp(c_param->valuestring,"")==0){
							ms.cmd=LOAD_ERAM;
							ms.p=NULL;
							this->p_mb_circuit->send(&ms);
						}
						else start_emu=false;
					}
					else if (cJSON_IsNull(c_param)){
						ms.cmd=LOAD_ERAM;
						ms.p=NULL;
						this->p_mb_circuit->send(&ms);
					}
					else start_emu=false;
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"ROM_file");
					if (cJSON_IsString(c_param)&&std::filesystem::is_regular_file(c_param->valuestring)){
						ms.cmd=LOAD_EROM;
						ms.p=malloc(strlen(c_param->valuestring)+1);
						strcpy((char*)ms.p,c_param->valuestring);
						((char*)ms.p)[strlen(c_param->valuestring)]=0;
						this->p_mb_circuit->send(&ms);
					}
					else start_emu=false;
					
					c_param=cJSON_GetObjectItemCaseSensitive(subconfig,"auto_start");
					if (cJSON_IsBool(c_param)&&cJSON_IsTrue(c_param)&&start_emu){
						ms.cmd=EMU_ON;
						this->p_mb_circuit->send(&ms);	
					}*/
					
					printf("cJSON is null? %i\n",cJSON_IsNull(config)==cJSON_NULL);
					char* string=cJSON_Print(config);
					printf("%s\n",string);
				}
			}
			
			this->p_TS9347out=new TS9347Renderer(this->window,&(this->PARAMETERS));
			this->keyboardIndicator=new KeyboardIndicator();
			
			this->Notification.notify(4,ImVec4(0,1,1,1));
	
			//TS7514_WLO_init(&(this->wloc),48000);
			
			//audio
			this->audioDeviceConfig = ma_device_config_init(ma_device_type_playback);
			
			this->audioDeviceConfig.playback.format   = ma_format_f32;
			this->audioDeviceConfig.playback.channels = 1;
			
			this->audioDeviceConfig.sampleRate        = 0;//48000;
			
			this->audioDeviceConfig.dataCallback      = this->audio_data_callback;
			this->audioDeviceConfig.periodSizeInFrames = 32;
			this->audioDeviceConfig.wasapi.usage = ma_wasapi_usage_pro_audio;
			this->audioDeviceConfig.wasapi.noAutoConvertSRC = true;
			this->audioDeviceConfig.pUserData         = &(this->AC);

			if (ma_device_init(NULL, &(this->audioDeviceConfig), &(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to open playback device.\n");
				return;
			}
			
			printf("Sample rate: %iHz\n",this->audioDevice.sampleRate);
			TS7514_WLO_init(&(this->wloc),this->audioDevice.sampleRate);

			printf("Device Name: %s\n", this->audioDevice.playback.name);
			
			if (ma_device_start(&(this->audioDevice)) != MA_SUCCESS) {
				printf("Failed to start playback device.\n");
				ma_device_uninit(&(this->audioDevice));
				return;
			}
		}
		~M12Window(){
			//wait emulator response
			while (this->PARAMETERS.p_gState->minitelOn.load(std::memory_order_relaxed)){}
			//imgui
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			//emulation screen
			delete this->p_TS9347out;
			//indicators
			delete this->keyboardIndicator;
			//glfw
			glfwDestroyWindow(this->window);
			glfwTerminate();
			//clock sync
			this->AC.pCLKs->setAudioSampleRate(0);//doesn't change anything but safer -> decouple emulator clock from audio clock
			//audio
			ma_device_uninit(&(this->audioDevice));
			TS7514_WLO_uninit(&(this->wloc));
			//RAM and ROM files
			unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->eram));
			unloadM(this->PARAMETERS.p_gState->p_thread_mutex,&(this->PARAMETERS.p_gState->erom));
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
							fprintf(stdout,"eram pointer %p\n",this->PARAMETERS.debug.eram.mem);
							this->PARAMETERS.debug.eram.op=&(((SRAM_64k*)ms.p)->last_memory_operation);
							fprintf(stdout,"eram address pointer %p\n",this->PARAMETERS.debug.eram.op);
							this->PARAMETERS.debug.eram.mem_size=ERAM_SIZE;
							break;
						case EROM:
							this->PARAMETERS.debug.erom.mem=((ROM_256k*)ms.p)->eROM;
							fprintf(stdout,"erom pointer %p\n",this->PARAMETERS.debug.erom.mem);
							this->PARAMETERS.debug.erom.op=&(((ROM_256k*)ms.p)->last_memory_operation);
							fprintf(stdout,"erom address pointer %p\n",this->PARAMETERS.debug.erom.op);
							this->PARAMETERS.debug.erom.mem_size=EROM_SIZE;
							break;
						case VC:
							this->PARAMETERS.debug.vram.mem=((TS9347wVRAM*)ms.p)->VRAM;
							this->p_TS9347out->setIC((TS9347wVRAM*)ms.p);
							fprintf(stdout,"vram pointer %p\n",this->PARAMETERS.debug.vram.mem);
							this->PARAMETERS.debug.vram.mem_size=VRAM_SIZE;
							break;
						case UC:
						{
							m80C32* uc=(m80C32*)ms.p;
							this->PARAMETERS.debug.iram.mem=uc->iRAM;
							fprintf(stdout,"iram pointer %p\n",this->PARAMETERS.debug.iram.mem);
							this->PARAMETERS.debug.iram.op=&(uc->last_memory_operation);
							fprintf(stdout,"iram address pointer %p\n",this->PARAMETERS.debug.iram.op);
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
							this->Notification.notify(0,ImVec4(1,0.5,0,1));
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
							this->wloc.pRWLO=&(((TS7514*)ms.p)->REG[((TS7514*)ms.p)->RWLO]);
							this->AC.wloc=&(this->wloc);
							printf("modem RWLO pointer: %p\n",this->AC.wloc->pRWLO);
							break;
						case CLOCK:
							this->AC.pCLKs=(Clocks*)ms.p;
							printf("Clocks pointer: %p\n",this->AC.pCLKs);
							this->AC.pCLKs->setAudioSampleRate(this->audioDevice.sampleRate);
							printf("Sync emulator to audio sample rate @%iHz\n",this->audioDevice.sampleRate);
							this->AC.pCLKs->requestSamples(256,256);//TODO: 256->buffer length
							break;
						default:
							fprintf(stdout,"unknown cmd %i\n",ms.cmd);
							break;
					}
					glfwPostEmptyEvent();
				}
				
				if (this->PARAMETERS.imgui.idle) glfwWaitEventsTimeout(0.05);
				else glfwPollEvents();
				int width, height;
				glfwGetFramebufferSize(this->window, &width, &height);
				glClear(GL_COLOR_BUFFER_BIT);
				glViewport(0,0,width,height);
				
				//create frame imgui
				ImGui_ImplOpenGL3_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
				if (this->PARAMETERS.imgui.show_menu){
					//ImGui::ShowDemoWindow(&(this->PARAMETERS.imgui.show_menu));
					mainMenuWindow(&(this->PARAMETERS),this->p_mb_circuit);
				}
				if (this->PARAMETERS.debug.eram.mem!=NULL&&this->PARAMETERS.debug.eram.show) memoryWindow("RAM externe",&(this->PARAMETERS.debug.eram));
				if (this->PARAMETERS.debug.erom.mem!=NULL&&this->PARAMETERS.debug.erom.show) memoryWindow("ROM externe",&(this->PARAMETERS.debug.erom));
				if (this->PARAMETERS.debug.iram.mem!=NULL&&this->PARAMETERS.debug.iram.show) memoryWindow("RAM interne",&(this->PARAMETERS.debug.iram));
				if (this->PARAMETERS.debug.sfr.show) sfr80C32Window(&(this->PARAMETERS.debug.sfr));
				if (this->PARAMETERS.debug.vram.mem!=NULL&&this->PARAMETERS.debug.vram.show) memoryWindow("VRAM",&(this->PARAMETERS.debug.vram));
				this->Notification.notification_window();
				this->keyboardIndicator->window();
				
				//render frame emulator
				this->p_TS9347out->render();
		 
				//render frame imgui
				ImGui::Render();
				ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		 
				glfwSwapBuffers(this->window);
			}
		}
	private:
		GLFWwindow* window;
		Parameters PARAMETERS;
		NotificationServer Notification;
		TS9347Renderer* p_TS9347out;
		KeyboardIndicator* keyboardIndicator;
		Mailbox* p_mb_circuit;
		Mailbox* p_mb_video;
		
		audioContext AC;
		WLOConf wloc;
		ma_device_config audioDeviceConfig;
		ma_device audioDevice;
		
		static void error_callback(int error, const char* description){
			fprintf(stderr, "Error: %s\n", description);
		}
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
			M12Window* p_M12Window=(M12Window*)glfwGetWindowUserPointer(window);
			//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
			if (key==GLFW_KEY_F1&&action==GLFW_PRESS) p_M12Window->PARAMETERS.imgui.show_menu=!p_M12Window->PARAMETERS.imgui.show_menu;
			ImGuiIO& io=ImGui::GetIO();
			KeyboardInput(p_M12Window->p_mb_circuit,!io.WantCaptureKeyboard,scancode,action,mods);
		}
		//static void char_callback(GLFWwindow* window, unsigned int codepoint){}
		static void audio_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){
			//buzzer
			audioContext* AC=(audioContext*)pDevice->pUserData;
			if (AC->wloc!=NULL){
				TS7514_WLO(AC->wloc, pOutput, frameCount);
			}
			if (AC->pCLKs!=NULL){
				AC->pCLKs->requestSamples(frameCount,256);//TODO: 256->buffer length
			}
		}
		static void window_close_callback(GLFWwindow* window){
			M12Window* p_M12Window=(M12Window*)glfwGetWindowUserPointer(window);
			p_M12Window->PARAMETERS.p_gState->shutdown.store(true,std::memory_order_relaxed);
			fprintf(stdout,"Close callback\n");
			/////////////////////////////////////////////////////
		}
};





void thread_video_main(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_gState){
	M12Window m12w=M12Window(p_mb_circuit,p_mb_video,p_gState);
	m12w.Loop();
}