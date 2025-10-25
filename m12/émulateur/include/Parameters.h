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
struct P_ImGui{
	bool show_menu=true;
	ImFont* font=NULL;
};
struct P_Emulation{
	int state=-1;
};
struct P_FileSystem{
	const char* erom=NULL;
	const char* eram=NULL;
	const char* charset=NULL;
};
struct P_Keyboard{
	
};
struct P_Peri{
	bool plugged=false;
	bool notify_state=true;
};
struct P_Modem{
	bool plugged=false;
	int main_device=-1;
	bool notify_state=true;
};
struct P_Buzzer{
	int audio_device=-1;
	bool notify_state=true;
};
struct P_CRT{
	bool rgb=false;
	float width_factor=2/3.;
	const float width=480;
	const float height=250;
};
struct P_IO{
	P_Keyboard keyboard;
	P_Peri peri;
	P_Modem modem;
	P_Buzzer buzzer;
	P_CRT crt;
};
struct P_Debug{
	imguiMemoryView eram;
	//imguiMemoryView iram;
	imguiMemoryView erom;
};
struct P_Info{
	const char* title="Minitel 12 Philips";
	const char* programmer="";
	License lib_licenses[4]={
		{lib_imgui,license_imgui},
		{lib_glfw,license_glfw},
		{lib_miniaudio,license_miniaudio},
		{lib_cjson,license_cjson}
	};
	License font_licenses[1]={
		{font_proggyclean,license_proggyclean}
	};
};
struct Parameters{
	GlobalState* p_gState=NULL;
	P_ImGui imgui;
	P_Emulation emu;
	P_FileSystem fs;
	P_IO io;
	P_Debug debug;
	P_Info info;
};

#endif