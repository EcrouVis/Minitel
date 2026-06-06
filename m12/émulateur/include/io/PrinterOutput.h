#ifndef PRINTEROUTPUT_H
#define PRINTEROUTPUT_H
#include "imgui.h"
#include "Parameters.h"
#include "io/NotificationServer.h"
#include "GLFW/glfw3.h"
void PrinterOutput(GLFWwindow* window,NotificationServer* notif, P_Peri_Printer* config){
	if (config->last_print==NULL){
		config->show=false;
		return;
	}
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1,-1),ImVec2(-1,-1));
	ImGui::Begin("Imprimante",&(config->show),ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text(config->last_print);
	constexpr char btxt[]="Copier dans le presse-papier";
	int min_size=ImGui::CalcTextSize(btxt).x+2*ImGui::GetStyle().FramePadding.x;
	int w_size=ImGui::GetContentRegionAvail().x;
	if(ImGui::Button(btxt,ImVec2(w_size>min_size?w_size:min_size,0))){
		glfwSetClipboardString(window,config->last_print);
		notif->notify("Texte copié dans le presse-papier",false,ImVec4(0,1,1,1));
	}
	ImGui::End();
}
#endif