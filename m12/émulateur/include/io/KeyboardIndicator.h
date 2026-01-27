#ifndef KEYBOARDINDICATOR_H
#define KEYBOARDINDICATOR_H
#include "imgui.h"
#include <atomic>
#include <cmath>
#include "circuit/Keyboard.h"
class KeyboardIndicator{
	public:
		KeyboardIndicator(){
			ImGuiIO& io=ImGui::GetIO();
			ImFontConfig config;
			this->font = io.Fonts->AddFontDefault(&config);
			config.MergeMode = true;
			io.Fonts->AddFontFromFileTTF("./ressources/Font Awesome 7 Free-Solid-900.otf", 0, &config);
		}
		void setKeyboard(Keyboard* kb){
			this->p_keyboard=kb;
		}
		void window(){
			if (this->p_keyboard==NULL) return;
			
			ImGui::SetNextWindowBgAlpha(0.75f);
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetMainViewport()->Size.x-50,10));//TODO: compute position
			ImGui::Begin("keyboard leds",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoScrollbar);
			double t=glfwGetTime();
			double n;
			const double f_blink=2;
			ImGui::PushFont(this->font);
			switch (this->p_keyboard->LED_POWER.load(std::memory_order_acquire)){
				case LED_ON:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.2,0.2,1));break;
				case LED_OFF:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.2,0.2,0.2,1));break;
				case LED_BLINK:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4((modf(t*f_blink,&n)<0.5)?1:0.2,0.2,0.2,1));break;
			}
			ImGui::Text("\uf011");
			ImGui::PopStyleColor();
			ImGui::Text(" ");
			switch (this->p_keyboard->LED_SPEAKER.load(std::memory_order_acquire)){
				case LED_ON:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.2,0.2,1));break;
				case LED_OFF:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.2,0.2,0.2,1));break;
			}
			ImGui::Text("\uf028");
			ImGui::PopStyleColor();
			ImGui::PopFont();
			ImGui::End();
		}
	private:
		ImFont* font;
		Keyboard* p_keyboard=NULL;
	
};
#endif