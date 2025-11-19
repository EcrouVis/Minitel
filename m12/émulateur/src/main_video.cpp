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


/*class M12Window(){
	public:
		M12Window(){
			
		}
		~M12Window(){
			imguiShutdown();
			glfwDestroyWindow(window);
			glfwTerminate();
		}
		void setGlobalState(GlobalState* p_GlobalState){
			this->p_gState=p_GlobalState;
		}
	private:
		GLFWwindow* window
		Parameters PARAMETERS;
		NotificationServer Notification;
		
		static void error_callback(int error, const char* description){
			fprintf(stderr, "Error: %s\n", description);
		}
		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
			M12Window* p_M12Window=glfwGetWindowUserPointer(window);
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
			if (key==GLFW_KEY_F1&&action==GLFW_PRESS) p_M12Window->PARAMETERS.imgui.show_menu=!p_M12Window->PARAMETERS.imgui.show_menu;
			imgui_notify(3,ImVec4(0.,1.,1.,1.));
		}
		static void char_callback(GLFWwindow* window, unsigned int codepoint){
			fprintf(stdout,"%c\n",codepoint);
		}
		static void window_close_callback(GLFWwindow* window){
			fprintf(stdout,"Close callback\n");
			/////////////////////////////////////////////////////
		}
}*/

bool show_imgui=true;

//vertex x y
//uniform A B C
//position x0=(6 or 12)*x y0=10*y h=10 w=(6 or 8)
//w=480 h=250

 
static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (key==GLFW_KEY_F1&&action==GLFW_PRESS) show_imgui=!show_imgui;
	//imgui_notify(3,ImVec4(0.,1.,1.,1.));
}
static void char_callback(GLFWwindow* window, unsigned int codepoint){
	if (!ImGui::GetIO().WantCaptureKeyboard){
		fprintf(stdout,"%c\n",codepoint);
	}
}

static void window_close_callback(GLFWwindow* window){
	fprintf(stdout,"Close callback\n");
	/////////////////////////////////////////////////////
}

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

void imguiStartFrame(Parameters* p_params,NotificationServer* p_notif,thread_mailbox* p_mb_circuit){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	//bool use_font=(p_params->imgui.font!=NULL);
	//if (use_font) ImGui::PushFont(p_params->imgui.font);
	if (p_params->imgui.show_menu){
		ImGui::ShowDemoWindow(&(p_params->imgui.show_menu));
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

void thread_video_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_video,GlobalState* p_gState){
	Parameters PARAMETERS;
	PARAMETERS.p_gState=p_gState;
	NotificationServer Notification;
	
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    GLFWwindow* window = glfwCreateWindow(640, 480, PARAMETERS.info.title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
	glfwSetWindowCloseCallback(window, window_close_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
	
	imguiInit(window);
 
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	
	TS9347Renderer TS9347out=TS9347Renderer(window,&PARAMETERS);
	
    while (!glfwWindowShouldClose(window))
    {
		thread_message ms;
		while (thread_receive_message(p_mb_video,&ms)>=0){
			switch(ms.cmd){
				case ERAM:
					PARAMETERS.debug.eram.mem=((SRAM_64k*)ms.p)->RAM;
					fprintf(stdout,"eram pointer %p\n",PARAMETERS.debug.eram.mem);
					PARAMETERS.debug.eram.op=&(((SRAM_64k*)ms.p)->last_memory_operation);
					fprintf(stdout,"eram address pointer %p\n",PARAMETERS.debug.eram.op);
					PARAMETERS.debug.eram.mem_size=ERAM_SIZE;
					break;
				case EROM:
					PARAMETERS.debug.erom.mem=((ROM_256k*)ms.p)->eROM;
					fprintf(stdout,"erom pointer %p\n",PARAMETERS.debug.erom.mem);
					PARAMETERS.debug.erom.op=&(((ROM_256k*)ms.p)->last_memory_operation);
					fprintf(stdout,"erom address pointer %p\n",PARAMETERS.debug.erom.op);
					PARAMETERS.debug.erom.mem_size=EROM_SIZE;
					break;
				case VC:
					PARAMETERS.debug.vram.mem=((TS9347wVRAM*)ms.p)->VRAM;
					TS9347out.setIC((TS9347wVRAM*)ms.p);
					fprintf(stdout,"vram pointer %p\n",PARAMETERS.debug.vram.mem);
					PARAMETERS.debug.vram.mem_size=VRAM_SIZE;
					break;
				case UC:
				{
					m80C32* uc=(m80C32*)ms.p;
					PARAMETERS.debug.iram.mem=uc->iRAM;
					fprintf(stdout,"iram pointer %p\n",PARAMETERS.debug.iram.mem);
					PARAMETERS.debug.iram.op=&(uc->last_memory_operation);
					fprintf(stdout,"iram address pointer %p\n",PARAMETERS.debug.iram.op);
					PARAMETERS.debug.iram.mem_size=IRAM_SIZE;
					PARAMETERS.debug.sfr.ACC=&(uc->SFR[uc->ACC&0x7F]);
					PARAMETERS.debug.sfr.B=&(uc->SFR[uc->B&0x7F]);
					PARAMETERS.debug.sfr.DPH=&(uc->SFR[uc->DPH&0x7F]);
					PARAMETERS.debug.sfr.DPL=&(uc->SFR[uc->DPL&0x7F]);
					PARAMETERS.debug.sfr.IE=&(uc->SFR[uc->IE&0x7F]);
					PARAMETERS.debug.sfr.IP=&(uc->SFR[uc->IP&0x7F]);
					PARAMETERS.debug.sfr.P0=&(uc->SFR[uc->P0&0x7F]);
					PARAMETERS.debug.sfr.P1=&(uc->SFR[uc->P1&0x7F]);
					PARAMETERS.debug.sfr.P2=&(uc->SFR[uc->P2&0x7F]);
					PARAMETERS.debug.sfr.P3=&(uc->SFR[uc->P3&0x7F]);
					PARAMETERS.debug.sfr.PCON=&(uc->SFR[uc->PCON&0x7F]);
					PARAMETERS.debug.sfr.PSW=&(uc->SFR[uc->PSW&0x7F]);
					PARAMETERS.debug.sfr.RCAP2H=&(uc->SFR[uc->RCAP2H&0x7F]);
					PARAMETERS.debug.sfr.RCAP2L=&(uc->SFR[uc->RCAP2L&0x7F]);
					PARAMETERS.debug.sfr.SBUF=&(uc->SFR[uc->SBUF&0x7F]);
					PARAMETERS.debug.sfr.SCON=&(uc->SFR[uc->SCON&0x7F]);
					PARAMETERS.debug.sfr.SP=&(uc->SFR[uc->SP&0x7F]);
					PARAMETERS.debug.sfr.TCON=&(uc->SFR[uc->TCON&0x7F]);
					PARAMETERS.debug.sfr.T2CON=&(uc->SFR[uc->T2CON&0x7F]);
					PARAMETERS.debug.sfr.TH0=&(uc->SFR[uc->TH0&0x7F]);
					PARAMETERS.debug.sfr.TH1=&(uc->SFR[uc->TH1&0x7F]);
					PARAMETERS.debug.sfr.TH2=&(uc->SFR[uc->TH2&0x7F]);
					PARAMETERS.debug.sfr.TL0=&(uc->SFR[uc->TL0&0x7F]);
					PARAMETERS.debug.sfr.TL1=&(uc->SFR[uc->TL1&0x7F]);
					PARAMETERS.debug.sfr.TL2=&(uc->SFR[uc->TL2&0x7F]);
					PARAMETERS.debug.sfr.TMOD=&(uc->SFR[uc->TMOD&0x7F]);
					break;
				}
				case CPLD:
					PARAMETERS.io.other.os_rtc=&((MBSL_4000FH5_5*)ms.p)->OS_RTC;
					break;
				case NOTIFICATION:
					Notification.notify((const char*)ms.p,true);
					//imgui_notify((const char*)ms.p,true);
					fprintf(stdout,"notification from thread\n");
					break;
				case NOTIFICATION_BUZZER:
					Notification.notify(0,ImVec4(1,0.5,0,1));
					//imgui_notify(0,ImVec4(1,0.5,0,1));
					fprintf(stdout,"buzzer\n");
					break;
				default:
					fprintf(stdout,"unknown cmd %i\n",ms.cmd);
					break;
			}
			glfwPostEmptyEvent();
		}
		if (PARAMETERS.imgui.idle) glfwWaitEventsTimeout(0.2);
		else glfwPollEvents();
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0,0,width,height);
		
		imguiStartFrame(&PARAMETERS,&Notification,p_mb_circuit);
		
		TS9347out.render();
 
		imguiRender();
 
        glfwSwapBuffers(window);
    }
 
	imguiShutdown();
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
	
	PARAMETERS.p_gState->shutdown.store(true,std::memory_order_relaxed);
}