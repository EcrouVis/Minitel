#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "license.h"
#include "GlobalState.h"

struct imguiMemoryView{
	int mem_size=0;
	bool show=false;
	bool follow_address=false;
	std::atomic_uchar* mem=NULL;
	std::atomic_uint* op=NULL;
};
struct imgui80C32SFRView{
	bool show=false;
	std::atomic_uchar* ACC=NULL;
	std::atomic_uchar* B=NULL;
	std::atomic_uchar* DPH=NULL;
	std::atomic_uchar* DPL=NULL;
	std::atomic_uchar* IE=NULL;
	std::atomic_uchar* IP=NULL;
	std::atomic_uchar* P0=NULL;
	std::atomic_uchar* P1=NULL;
	std::atomic_uchar* P2=NULL;
	std::atomic_uchar* P3=NULL;
	std::atomic_uchar* PCON=NULL;
	std::atomic_uchar* PSW=NULL;
	std::atomic_uchar* RCAP2H=NULL;
	std::atomic_uchar* RCAP2L=NULL;
	std::atomic_uchar* SBUF=NULL;
	std::atomic_uchar* SCON=NULL;
	std::atomic_uchar* SP=NULL;
	std::atomic_uchar* TCON=NULL;
	std::atomic_uchar* T2CON=NULL;
	std::atomic_uchar* TH0=NULL;
	std::atomic_uchar* TH1=NULL;
	std::atomic_uchar* TH2=NULL;
	std::atomic_uchar* TL0=NULL;
	std::atomic_uchar* TL1=NULL;
	std::atomic_uchar* TL2=NULL;
	std::atomic_uchar* TMOD=NULL;
};
struct imguiTS9347REGView{
	bool show=false;
	std::atomic_uchar* STATUS=NULL;
	std::atomic_uchar* COMMAND=NULL;
	std::atomic_uchar* R1=NULL;
	std::atomic_uchar* R2=NULL;
	std::atomic_uchar* R3=NULL;
	std::atomic_uchar* R4=NULL;
	std::atomic_uchar* R5=NULL;
	std::atomic_uchar* R6=NULL;
	std::atomic_uchar* R7=NULL;
	std::atomic_uchar* DOR=NULL;
	std::atomic_uchar* ROR=NULL;
	std::atomic_uchar* TGS=NULL;
	std::atomic_uchar* PAT=NULL;
	std::atomic_uchar* MAT=NULL;
};
struct imguiTS7514REGView{
	bool show=false;
	std::atomic_uchar* RPROG=NULL;
	std::atomic_uchar* RDTMF=NULL;
	std::atomic_uchar* RATE=NULL;
	std::atomic_uchar* RWLO=NULL;
	std::atomic_uchar* RPTF=NULL;
	std::atomic_uchar* RPRF=NULL;
	std::atomic_uchar* RHDL=NULL;
	std::atomic_uchar* RPRX=NULL;
};
struct P_ImGui{
	bool show_menu=true;
	bool idle=true;
	int window_position[2]={200,200};
	int window_size[2]={640,480};
	
};
struct P_Emulation{
	int state=-1;
};
struct P_Keyboard{
	bool num_lock=false;
	bool show_teletel_keys=false;
	ImVec2 teletel_keys_window_pos;
	bool show_phone_keys=false;
	ImVec2 phone_keys_window_pos;
	bool show_azerty_keys=false;
	ImVec2 azerty_keys_window_pos;
	bool auto_hide_indicator=true;
};
struct P_Peri_Websocket{
	void* p_peri=NULL;
};
struct P_Peri_Printer{
	std::atomic_bool* p_activated=NULL;
	std::atomic_bool* p_fr=NULL;
	bool show=false;
	char* last_print=NULL;
};
struct P_Peri{
	//bool plugged=false;
	bool notify_state=true;
	P_Peri_Websocket websocket;
	P_Peri_Printer printer;
};
struct P_Modem{
	bool plugged=false;
	int main_device=-1;
	bool notify_state=true;
};
struct P_Buzzer{
	float volume=100.;
	bool notify=true;
};
struct P_Speaker{
	float volume=100.;
};
struct P_CRT{
	bool rgb;
	float black_level;
	float curvature;
	float decay;
	bool scanline;
	float width_factor=2./3.;
	const float width=480;
	const float height=250;
	bool display_effects=false;
	bool error_loading_texture=false;
};
constexpr P_CRT CRT_retro_preset{false,12.5,0.225,1./3.,true};
constexpr P_CRT CRT_modern_preset{true,0.,0.,0.,false};
struct P_Other{
	std::atomic_bool* os_rtc=NULL;
	std::atomic_bool* auto_start=NULL;
};
struct P_IO{
	P_Keyboard keyboard;
	P_Peri peri;
	P_Modem modem;
	P_Buzzer buzzer;
	P_Speaker speaker;
	P_CRT crt=CRT_retro_preset;
	P_Other other;
};
struct P_Debug{
	imguiMemoryView eram;
	imguiMemoryView iram;
	imguiMemoryView erom;
	imguiMemoryView vram;
	imgui80C32SFRView sfr;
	imguiTS9347REGView vreg;
	imguiTS7514REGView mreg;
};
struct P_Info{
	const char* title="Minitel 12 Philips";
	const char* programmers="Yves Landemarre";
	License lib_licenses[6]={
		{lib_imgui,license_imgui},
		{lib_glfw,license_glfw},
		{lib_miniaudio,license_miniaudio},
		{lib_cjson,license_cjson},
		{lib_ixwebsocket,license_ixwebsocket},
		{lib_libressl,license_libressl}
	};
	License font_licenses[2]={
		{font_proggyclean,license_proggyclean},
		{font_fontawesome,license_fontawesome}
	};
};
struct Parameters{
	GlobalState* p_gState=NULL;
	P_ImGui imgui;
	P_Emulation emu;
	P_IO io;
	P_Debug debug;
	P_Info info;
};

#endif