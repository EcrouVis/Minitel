#include "thread_messaging.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"
#include "circuit/TS9347.h"
#include "circuit/80C32.h"
#include "circuit/CPLD.h"

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
#include "NotificationServer.h"
#include "MemoryWindow.h"
#include "MainMenuWindow.h"
#include "Parameters.h"
#include "io/video.h"
#include "circuit/Keyboard.h"
#include "circuit/clocks.h"

void imguiInit(GLFWwindow* window){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io=ImGui::GetIO();
	io.IniFilename=NULL;
	io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
	
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init();
	/*ImFont* font = io.Fonts->AddFontFromFileTTF("./ressources/VCR_OSD_MONO_1.001.ttf", 13);
	ImGui::PushFont(font);*/
}

void imguiStartFrame(Parameters* p_params,NotificationServer* p_notif,Mailbox* p_mb_circuit){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//bool use_font=(p_params->imgui.font!=NULL);
	//if (use_font) ImGui::PushFont(p_params->imgui.font);
	if (p_params->imgui.show_menu){
		//ImGui::ShowDemoWindow(&(p_params->imgui.show_menu));
		mainMenuWindow(p_params,p_mb_circuit);
	}
	if (p_params->debug.eram.mem!=NULL&&p_params->debug.eram.show) memoryWindow("RAM externe",&p_params->debug.eram);
	if (p_params->debug.erom.mem!=NULL&&p_params->debug.erom.show) memoryWindow("ROM externe",&p_params->debug.erom);
	if (p_params->debug.iram.mem!=NULL&&p_params->debug.iram.show) memoryWindow("RAM interne",&p_params->debug.iram);
	if (p_params->debug.sfr.show) sfr80C32Window(&p_params->debug.sfr);
	if (p_params->debug.vram.mem!=NULL&&p_params->debug.vram.show) memoryWindow("VRAM",&p_params->debug.vram);
	p_notif->notification_window();
	//if (use_font) ImGui::PopFont();
}

void imguiRender(){
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiShutdown(){
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



class M12Window{
	public:
		M12Window(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_GlobalState){
			this->PARAMETERS.p_gState=p_GlobalState;
			this->p_mb_circuit=p_mb_circuit;
			this->p_mb_video=p_mb_video;
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
			glfwSetCharCallback(this->window, this->char_callback);
			glfwSetWindowCloseCallback(this->window, this->window_close_callback);
		 
			glfwMakeContextCurrent(this->window);
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			glfwSwapInterval(1);
			
			imguiInit(this->window);
		 
			GLuint vertex_buffer;
			glGenBuffers(1, &vertex_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			
			this->p_TS9347out=new TS9347Renderer(this->window,&(this->PARAMETERS));
		}
		~M12Window(){
			imguiShutdown();
			delete this->p_TS9347out;
			glfwDestroyWindow(this->window);
			glfwTerminate();
			this->PARAMETERS.p_gState->shutdown.store(true,std::memory_order_relaxed);
		}
		void Loop(){
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
				
				imguiStartFrame(&(this->PARAMETERS),&(this->Notification),this->p_mb_circuit);
				
				this->p_TS9347out->render();
		 
				imguiRender();
		 
				glfwSwapBuffers(this->window);
			}
		}
	private:
		GLFWwindow* window;
		Parameters PARAMETERS;
		NotificationServer Notification;
		TS9347Renderer* p_TS9347out;
		Mailbox* p_mb_circuit;
		Mailbox* p_mb_video;
		
		static void error_callback(int error, const char* description){
			fprintf(stderr, "Error: %s\n", description);
		}
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
			M12Window* p_M12Window=(M12Window*)glfwGetWindowUserPointer(window);
			//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
			if (key==GLFW_KEY_F1&&action==GLFW_PRESS) p_M12Window->PARAMETERS.imgui.show_menu=!p_M12Window->PARAMETERS.imgui.show_menu;
			//imgui_notify(3,ImVec4(0.,1.,1.,1.));
			ImGuiIO& io=ImGui::GetIO();
			if (io.WantCaptureKeyboard){
				thread_message ms_p_kb;
				keyboard_message* kbm=new keyboard_message;
				kbm->focus=false;
				ms_p_kb.p=(void*)kbm;
				ms_p_kb.cmd=KEYBOARD_STATE_UPDATE;
				p_M12Window->p_mb_circuit->send(&ms_p_kb);
			}
			if ((!io.WantCaptureKeyboard)&&(action==GLFW_PRESS||action==GLFW_RELEASE)){
				printf("key %i %i\n",scancode,mods);
				thread_message ms_p_kb;
				keyboard_message* kbm=new keyboard_message;
				kbm->focus=true;
				kbm->scancode=scancode;
				kbm->action=action;
				kbm->mods=mods;
				ms_p_kb.p=(void*)kbm;
				ms_p_kb.cmd=KEYBOARD_STATE_UPDATE;
				p_M12Window->p_mb_circuit->send(&ms_p_kb);
			}
		}
		static void char_callback(GLFWwindow* window, unsigned int codepoint){
			/*ImGuiIO& io=ImGui::GetIO();
			if (!io.WantCaptureKeyboard){
				fprintf(stdout,"char %c\n",codepoint);
			}*/
		}
		static void window_close_callback(GLFWwindow* window){
			fprintf(stdout,"Close callback\n");
			/////////////////////////////////////////////////////
		}
};





void thread_video_main(Mailbox* p_mb_circuit,Mailbox* p_mb_video,GlobalState* p_gState){
	M12Window m12w=M12Window(p_mb_circuit,p_mb_video,p_gState);
	m12w.Loop();
}