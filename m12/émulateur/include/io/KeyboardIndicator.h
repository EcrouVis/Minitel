#ifndef KEYBOARDINDICATOR_H
#define KEYBOARDINDICATOR_H
#include "imgui.h"
#include <atomic>
#include <cmath>
#include "circuit/Keyboard.h"
#include "FontAwesomeIcon.h"
class KeyboardIndicator{
	public:
		void Init(){
			ImGuiIO& io=ImGui::GetIO();
			ImFontConfig config;
			io.Fonts->AddFontDefaultBitmap(&config);
			config.MergeMode = false;
			config.GlyphMinAdvanceX=13;
			config.GlyphMaxAdvanceX=13;
			config.SizePixels=13;
			this->font=io.Fonts->AddFontFromMemoryCompressedTTF(FontAwesomeIcon_compressed_data,FontAwesomeIcon_compressed_size,0,&config);
		}
		void setKeyboard(Keyboard* kb){
			this->p_keyboard=kb;
		}
		bool isSignaling(){
			if (this->p_keyboard==NULL) return false;
			return this->p_keyboard->LED_POWER.load(std::memory_order_acquire)!=Keyboard::LED_OFF||this->p_keyboard->LED_SPEAKER.load(std::memory_order_acquire)!=Keyboard::LED_OFF;
		}
		void window(){
			if (this->p_keyboard==NULL) return;
			
			ImGui::SetNextWindowBgAlpha(0.50f);
			ImVec2 pos=ImGui::GetMainViewport()->Size;
			pos[0]-=10;
			pos[1]=10;
			ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1, 0));
			ImGui::Begin("keyboard leds",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoNav);
			double t=glfwGetTime();
			double n;
			ImGui::PushFont(this->font);
			switch (this->p_keyboard->LED_POWER.load(std::memory_order_acquire)){
				case Keyboard::LED_ON:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.2,0.2,1));break;
				case Keyboard::LED_OFF:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.2,0.2,0.2,1));break;
				case Keyboard::LED_BLINK_FAST:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4((modf(t*2,&n)<0.5)?1:0.2,0.2,0.2,1));break;
				case Keyboard::LED_BLINK_SLOW:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4((modf(t*0.5,&n)<0.5)?1:0.2,0.2,0.2,1));break;
			}
			ImGui::Text("\uf011");
			ImGui::PopStyleColor();
			ImGui::NewLine();
			switch (this->p_keyboard->LED_SPEAKER.load(std::memory_order_acquire)){
				case Keyboard::LED_ON:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.2,0.2,1));break;
				case Keyboard::LED_OFF:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.2,0.2,0.2,1));break;
				case Keyboard::LED_BLINK_FAST:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4((modf(t*2,&n)<0.5)?1:0.2,0.2,0.2,1));break;
				case Keyboard::LED_BLINK_SLOW:ImGui::PushStyleColor(ImGuiCol_Text,ImVec4((modf(t*0.5,&n)<0.5)?1:0.2,0.2,0.2,1));break;
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