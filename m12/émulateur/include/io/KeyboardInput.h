#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H
#include <atomic>
#include "circuit/Keyboard.h"

class KeyboardInput{
	public:
		void KeyboardAzertyWindow(){
			int bs=ImGui::CalcTextSize(" ").x*5+2*ImGui::GetStyle().FramePadding.x;
			bool ctrl=this->getKey(0x9D);
			bool shift=this->getKey(0xAF);
			bool activated=(ctrl!=shift);
			ctrl=ctrl&activated;
			shift=shift&activated;
			
			ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.5,0.5,0.5,0.2));
			ImGui::Begin("##keyboard_azerty",NULL,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6,0.6,0.6,1));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1,0.1,0.1,1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2,0.2,0.2,1));
			
			activated=this->getKey(0xB3);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("1",ImVec2(bs,0));
			else ImGui::Button("&",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xB3,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xB3,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xB1);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("2",ImVec2(bs,0));
			else ImGui::Button("é",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xB1,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xB1,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xA7);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("3",ImVec2(bs,0));
			else ImGui::Button("\"",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xA7,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xA7,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xA1);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("4",ImVec2(bs,0));
			else ImGui::Button("'",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xA1,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xA1,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x91);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("5",ImVec2(bs,0));
			else ImGui::Button("(",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x91,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x91,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x81);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("6",ImVec2(bs,0));
			else ImGui::Button("-",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x81,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x81,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x71);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("7",ImVec2(bs,0));
			else ImGui::Button("è",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x71,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x71,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x61);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("8",ImVec2(bs,0));
			else ImGui::Button("!",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x61,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x61,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x51);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("9",ImVec2(bs,0));
			else ImGui::Button("ç",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x51,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x51,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x53);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("0",ImVec2(bs,0));
			else ImGui::Button("à",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x53,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x53,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x37);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("°",ImVec2(bs,0));
			else ImGui::Button(")",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x37,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x37,false);
			
			ImGui::BeginGroup();
			
			activated=this->getKey(0xBB);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("A",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xBB,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xBB,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xB7);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Z",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xB7,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xB7,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xB9);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("E",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xB9,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xB9,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xA9);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("R",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xA9,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xA9,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x97);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("T",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x97,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x97,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x87);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Y",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x87,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x87,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x77);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("U",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x77,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x77,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x67);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("Tab",ImVec2(bs,0));
			else ImGui::Button("I",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x67,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x67,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x69);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("O",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x69,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x69,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x57);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("P",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x57,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x57,false);
			
			activated=this->getKey(0xBF);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Q",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xBF,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xBF,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xBD);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("S",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xBD,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xBD,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0xAB);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("D",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xAB,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xAB,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x99);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("F",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x99,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x99,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x8B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("G",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x8B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x8B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x89);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("BS",ImVec2(bs,0));
			else ImGui::Button("H",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x89,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x89,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x79);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("LF",ImVec2(bs,0));
			else ImGui::Button("J",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x79,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x79,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x6B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("K",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x6B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x6B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x59);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("L",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x59,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x59,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x3B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("M",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x3B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x3B,false);
			
			ImGui::EndGroup();
			
			ImGui::SameLine();
			
			activated=this->getKey(0x39);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("\\--",ImVec2(bs,2*(ImGui::CalcTextSize(" ").y+2*ImGui::GetStyle().FramePadding.y)+ImGui::GetStyle().ItemSpacing.y));
			else if (ctrl) ImGui::Button("E.Pg",ImVec2(bs,2*(ImGui::CalcTextSize(" ").y+2*ImGui::GetStyle().FramePadding.y)+ImGui::GetStyle().ItemSpacing.y));
			else ImGui::Button("<-'",ImVec2(bs,2*(ImGui::CalcTextSize(" ").y+2*ImGui::GetStyle().FramePadding.y)+ImGui::GetStyle().ItemSpacing.y));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x39,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x39,false);
			
			activated=this->getKey(0xAF);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("^##shift",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()){
				this->setKey(0xAF,!this->getKey(0xAF));
			}
			
			ImGui::SameLine();
			
			activated=this->getKey(0xAD);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("W",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xAD,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xAD,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x9B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("Can",ImVec2(bs,0));
			else ImGui::Button("X",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x9B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x9B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x8D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("C",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x8D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x8D,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x8F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("V",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x8F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x8F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x7D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("B",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x7D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x7D,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x7B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("N",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x7B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x7B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x6F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("?",ImVec2(bs,0));
			else ImGui::Button(",",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x6F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x6F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x6D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button(".",ImVec2(bs,0));
			else ImGui::Button(";",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x6D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x6D,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x5B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Sup.L",ImVec2(bs,0));
			else ImGui::Button("^",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x5B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x5B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x3D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("/",ImVec2(bs,0));
			else ImGui::Button(":",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x3D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x3D,false);
			
			activated=this->getKey(0x9F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Mn/Mj",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x9F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x9F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x9D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Ctrl",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()){
				this->setKey(0x9D,!this->getKey(0x9D));
			}
			
			ImGui::SameLine();
			
			activated=this->getKey(0x7F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("##space",ImVec2(6*bs+5*ImGui::GetStyle().ItemSpacing.x,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x7F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x7F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x5D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Sup.C",ImVec2(bs,0));
			else if (ctrl) ImGui::Button("Del",ImVec2(bs,0));
			else ImGui::Button("<",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x5D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x5D,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x5F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Ins.L",ImVec2(bs,0));
			else ImGui::Button("v",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x5F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x5F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x3F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Ins.C",ImVec2(bs,0));
			else ImGui::Button(">",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x3F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x3F,false);
			
			ImGui::PopStyleColor(4);
			
			ImGui::End();
			ImGui::PopStyleColor();
		}
		
		void KeyboardTeletelWindow(){
			int bs=ImGui::CalcTextSize(" ").x*5+2*ImGui::GetStyle().FramePadding.x;
			int bl=ImGui::CalcTextSize(" ").x*10+2*ImGui::GetStyle().FramePadding.x;
			bool ctrl=this->getKey(0x9D);
			bool shift=this->getKey(0xAF);
			bool activated=(ctrl!=shift);
			ctrl=ctrl&activated;
			shift=shift&activated;
			
			ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.5,0.5,0.5,0.2));
			ImGui::Begin("##keyboard_teletel",NULL,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6,0.6,0.6,1));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1,0.1,0.1,1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2,0.2,0.2,1));
			ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0,0.5));
			
			activated=this->getKey(0xB5);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("Brk",ImVec2(bl,0));
			else ImGui::Button("Connex/Fin",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xB5,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xB5,false);
			
			ImGui::SameLine();
			ImGui::Text("  ");
			ImGui::SameLine();
			ImGui::BeginGroup();
			
			activated=this->getKey(0xA5);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Esc",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0xA5,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0xA5,false);
			
			activated=this->getKey(0xA3);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Fnct",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()){
				this->setKey(0xA3,!this->getKey(0xA3));
			}
			
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			
			activated=this->getKey(0x95);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("^",ImVec2(bl,0));
			else ImGui::Button("Sommaire",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x95,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x95,false);
			
			activated=this->getKey(0x93);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("¨",ImVec2(bl,0));
			else ImGui::Button("Guide",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x93,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x93,false);
			
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			
			activated=this->getKey(0x85);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("£",ImVec2(bl,0));
			else ImGui::Button("Annulation",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x85,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x85,false);
			
			activated=this->getKey(0x83);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("ù",ImVec2(bl,0));
			else ImGui::Button("Correction",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x83,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x83,false);
			
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			
			activated=this->getKey(0x75);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("OE",ImVec2(bl,0));
			else ImGui::Button("Retour",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x75,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x75,false);
			
			activated=this->getKey(0x73);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("@",ImVec2(bl,0));
			else ImGui::Button("Suite",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x73,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x73,false);
			
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			
			activated=this->getKey(0x65);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("oe",ImVec2(bl,0));
			else if (ctrl) ImGui::Button("{",ImVec2(bl,0));
			else ImGui::Button("Répétition",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x65,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x65,false);
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0.7,0.6,0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1,0.8,0.7,1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2,0.9,0.8,1));
			activated=this->getKey(0x63);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (ctrl) ImGui::Button("}",ImVec2(bl,0));
			else ImGui::Button("Envoi",ImVec2(bl,0));
			if (activated) ImGui::PopStyleColor();
			ImGui::PopStyleColor(3);
			if(ImGui::IsItemActivated()) this->setKey(0x63,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x63,false);
			
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::Text("  ");
			ImGui::SameLine();
			ImGui::PopStyleVar();
			
			activated=this->getKey(0x55);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("o-",ImVec2(bs,0));
			else ImGui::Button("O",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x55,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x55,false);
			
			ImGui::PopStyleColor(4);
			
			ImGui::End();
			ImGui::PopStyleColor();
		}
		
		void KeyboardPhoneWindow(){
			int bs=ImGui::CalcTextSize(" ").x*5+2*ImGui::GetStyle().FramePadding.x;
			bool shift=this->getKey(0xAF)&!this->getKey(0x9D);
			bool activated;
			
			ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.5,0.5,0.5,0.2));
			ImGui::Begin("##keyboard_phone",NULL,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6,0.6,0.6,1));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0.5));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1,0.1,0.1,1));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2,0.2,0.2,1));
			
			activated=this->getKey(0x25);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("--/--",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x25,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x25,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x35);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("<=> | <]))",ImVec2(2*bs+ImGui::GetStyle().ItemSpacing.x,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x35,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x35,false);
			
			activated=this->getKey(0x33);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Impr.",ImVec2(bs,0));
			else ImGui::Button("(_)",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x33,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x33,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x15);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Bis",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x15,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x15,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x45);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("[V]>",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x45,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x45,false);
			
			activated=this->getKey(0x23);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("<]+",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x23,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x23,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x13);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("<]-",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x13,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x13,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x31);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Mem",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x31,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x31,false);
			
			activated=this->getKey(0x27);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("<",ImVec2(bs,0));
			else ImGui::Button("1",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x27,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x27,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x17);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button(">",ImVec2(bs,0));
			else ImGui::Button("2",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x17,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x17,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x21);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("\\",ImVec2(bs,0));
			else ImGui::Button("3",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x21,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x21,false);
			
			activated=this->getKey(0x19);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("$",ImVec2(bs,0));
			else ImGui::Button("4",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x19,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x19,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x29);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("%",ImVec2(bs,0));
			else ImGui::Button("5",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x29,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x29,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x11);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("§",ImVec2(bs,0));
			else ImGui::Button("6",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x11,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x11,false);
			
			activated=this->getKey(0x2B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("+",ImVec2(bs,0));
			else ImGui::Button("7",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x2B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x2B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x1B);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("=",ImVec2(bs,0));
			else ImGui::Button("8",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x1B,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x1B,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x2D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("_",ImVec2(bs,0));
			else ImGui::Button("9",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x2D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x2D,false);
			
			activated=this->getKey(0x1F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("[",ImVec2(bs,0));
			else ImGui::Button("*",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x1F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x1F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x2F);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("^",ImVec2(bs,0));
			else ImGui::Button("0",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x2F,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x2F,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x1D);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("]",ImVec2(bs,0));
			else ImGui::Button("#",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x1D,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x1D,false);
			
			ImGui::PopStyleColor(4);
			
			ImGui::End();
			ImGui::PopStyleColor();
			
		}
		
		void setKeyboard(Keyboard* kb){
			this->p_keyboard=kb;
		}
		
		void InputTranslate(bool focus, int scancode, int action, int mods){
			if (focus||action==GLFW_RELEASE){//||action==GLFW_RELEASE temporary fix to avoid being stuck when ctrl+click on imgui slider
				if (action==GLFW_PRESS||action==GLFW_RELEASE){
					bool keyPressed=(action==GLFW_PRESS);
					bool altgr=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==(GLFW_MOD_CONTROL|GLFW_MOD_ALT));
					KeyboardState kbs;
					kbs.fnct=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==GLFW_MOD_ALT);
					kbs.ctrl=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==GLFW_MOD_CONTROL);
					kbs.shift=(bool)(mods&GLFW_MOD_SHIFT);
					kbs.key=0;
					
					if (scancode==29||scancode==285||scancode==56||scancode==312||scancode==42||scancode==54){
						kbs.key=this->currentKeyboardState.key;
					}
					else if (keyPressed){
						if (altgr){
							switch (scancode){
								//compatibility
								case 11://à
									kbs.key=0x73;//Suite
									kbs.ctrl=true;
									break;
									
								case 5://'
									kbs.key=0x27;//numpad 1
									kbs.ctrl=true;
									break;
									
								case 13://=
									kbs.key=0x21;//numpad 3
									kbs.ctrl=true;
									break;
									
								case 7://-
									kbs.key=0x17;//numpad 2
									kbs.ctrl=true;
									break;
									
								case 3://é
									kbs.key=0x19;//numpad 4
									kbs.ctrl=true;
									break;
									
								case 8://è
									kbs.key=0x29;//numpad 5
									kbs.ctrl=true;
									break;
									
								case 4://"
									kbs.key=0x1D;//#
									break;
									
								case 6://(
									kbs.key=0x1F;// *
									kbs.shift=true;
									break;
									
								case 10://ç
									kbs.key=0x2F;//numpad 0
									kbs.shift=true;
									break;
									
								case 12://)
									kbs.key=0x1D;//#
									kbs.shift=true;
									break;
									
								case 9://_
									kbs.key=0x21;//numpad 3
									kbs.shift=true;
									break;
							}
						}
						else{
							switch (scancode){
								case 16:kbs.key=0xBB;break;//A
								case 17:kbs.key=0xB7;break;//Z
								case 18:kbs.key=0xB9;break;//E
								case 19:kbs.key=0xA9;break;//R
								case 20:kbs.key=0x97;break;//T
								case 21:kbs.key=0x87;break;//Y
								case 22:kbs.key=0x77;break;//U
								case 23:kbs.key=0x67;break;//I
								case 24:kbs.key=0x69;break;//O
								case 25:kbs.key=0x57;break;//P
								case 30:kbs.key=0xBF;break;//Q
								case 31:kbs.key=0xBD;break;//S
								case 32:kbs.key=0xAB;break;//D
								case 33:kbs.key=0x99;break;//F
								case 34:kbs.key=0x8B;break;//G
								case 35:kbs.key=0x89;break;//H
								case 36:kbs.key=0x79;break;//J
								case 37:kbs.key=0x6B;break;//K
								case 38:kbs.key=0x59;break;//L
								case 39:kbs.key=0x3B;break;//M
								case 44:kbs.key=0xAD;break;//W
								case 45:kbs.key=0x9B;break;//X
								case 46:kbs.key=0x8D;break;//C
								case 47:kbs.key=0x8F;break;//V
								case 48:kbs.key=0x7D;break;//B
								case 49:kbs.key=0x7B;break;//N
								case 57:kbs.key=0x7F;break;//espace
								case 284:
								case 28:kbs.key=0x39;break;//entrée
								case 50:kbs.key=0x6F;break;//,
								case 83:
									kbs.shift=true;
									[[fallthrough]];
								case 51:kbs.key=0x6D;break;//;
								case 309:
									kbs.shift=true;
									[[fallthrough]];
								case 52:kbs.key=0x3D;break;//:
								
								case 82:kbs.key=0x2F;break;//numpad 0
								case 79:kbs.key=0x27;break;//numpad 1
								case 80:kbs.key=0x17;break;//numpad 2
								case 81:kbs.key=0x21;break;//numpad 3
								case 75:kbs.key=0x19;break;//numpad 4
								case 76:kbs.key=0x29;break;//numpad 5
								case 77:kbs.key=0x11;break;//numpad 6
								case 71:kbs.key=0x2B;break;//numpad 7
								case 72:kbs.key=0x1B;break;//numpad 8
								case 73:kbs.key=0x2D;break;//numpad 9
								
								case 60:kbs.key=0x95;break;//F2=Sommaire
								case 61:kbs.key=0x93;break;//F3=Guide
								case 62:kbs.key=0x85;break;//F4=Annulation
								case 63:kbs.key=0x83;break;//F5=Correction
								case 64:kbs.key=0x75;break;//F6=Retour
								case 65:kbs.key=0x73;break;//F7=Suite
								case 66:kbs.key=0x65;break;//F8=Répétition
								case 67:kbs.key=0x63;break;//F9=Envoi
								
								case 41:kbs.key=0x55;break;//²=on/off            ///////////////////////////////////////////////////////////////////////
								case 14:kbs.key=0x31;break;//backspace=mem       ///////////////////////////////////////////////////////////////////////
								case 15:kbs.key=0xB5;break;//tab=Connex/Fin      ///////////////////////////////////////////////////////////////////////
								case 1:kbs.key=0xA5;break;//échap=Esc
								case 58:kbs.key=0x9F;break;//min/maj
						
								case 328:kbs.key=0x5B;break;//flèche haut
								case 331:kbs.key=0x5D;break;//flèche gauche
								case 336:kbs.key=0x5F;break;//flèche bas
								case 333:kbs.key=0x3F;break;//flèche droite
								
								case 2:kbs.key=0xB3;break;//&
								case 3:kbs.key=0xB1;break;//é
								case 4:kbs.key=0xA7;break;//"
								case 5:kbs.key=0xA1;break;//'
								case 6:kbs.key=0x91;break;//(
								case 74:
									kbs.shift=false;
									[[fallthrough]];
								case 7:kbs.key=0x81;break;//-
								case 8:kbs.key=0x71;break;//è
								case 10:kbs.key=0x51;break;//ç
								case 11:kbs.key=0x53;break;//à
								case 12:kbs.key=0x37;break;//)
								
								//compatibility
								case 9://_
									if (kbs.ctrl||kbs.shift) kbs.key=0x61;// !
									else{
										kbs.key=0x2D;//numpad 9
										kbs.shift=true;
									}
									break;
								
								case 53:// !
									if (kbs.shift) kbs.key=0x11;//numpad 6
									else kbs.key=0x61;// !
									break;
								
								case 55:
									kbs.shift=false;
									[[fallthrough]];
								case 43:// *
									if (!kbs.shift){
										kbs.key=0x1F;// *
									}
									break;
									
								case 26://^
									if (kbs.shift){
										kbs.key=0x93;//Guide
										kbs.shift=false;
										kbs.ctrl=true;
									}
									else{
										kbs.key=0x95;//Sommaire
										kbs.ctrl=true;
									}
									break;
									
								case 27://$
									if (kbs.shift){
										kbs.key=0x85;//Annulation
										kbs.shift=false;
										kbs.ctrl=true;
									}
									else{
										kbs.key=0x19;//numpad 4
										kbs.shift=true;
									}
									break;
									
								case 40://ù
									if (kbs.shift) kbs.key=0x29;//numpad 5
									else{
										kbs.key=0x83;//Correction
										kbs.ctrl=true;
									}
									break;
								
								case 78:
									kbs.shift=true;
									[[fallthrough]];
								case 13://=
									if (kbs.shift) kbs.key=0x2B;//numpad 7
									else{
										kbs.key=0x1B;//numpad 8
										kbs.shift=true;
									}
									break;
									
								case 86://<
									if (kbs.shift) kbs.key=0x17;//numpad 2
									else{
										kbs.key=0x27;//numpad 1
										kbs.shift=true;
									}
									break;
							}
						}
					}
					this->keyboardTransition(kbs);
					/*switch (scancode){
						case 16:this->setKey(0xBB,keyPressed);break;//A
						case 17:this->setKey(0xB7,keyPressed);break;//Z
						case 18:this->setKey(0xB9,keyPressed);break;//E
						case 19:this->setKey(0xA9,keyPressed);break;//R
						case 20:this->setKey(0x97,keyPressed);break;//T
						case 21:this->setKey(0x87,keyPressed);break;//Y
						case 22:this->setKey(0x77,keyPressed);break;//U
						case 23:this->setKey(0x67,keyPressed);break;//I
						case 24:this->setKey(0x69,keyPressed);break;//O
						case 25:this->setKey(0x57,keyPressed);break;//P
						case 30:this->setKey(0xBF,keyPressed);break;//Q
						case 31:this->setKey(0xBD,keyPressed);break;//S
						case 32:this->setKey(0xAB,keyPressed);break;//D
						case 33:this->setKey(0x99,keyPressed);break;//F
						case 34:this->setKey(0x8B,keyPressed);break;//G
						case 35:this->setKey(0x89,keyPressed);break;//H
						case 36:this->setKey(0x79,keyPressed);break;//J
						case 37:this->setKey(0x6B,keyPressed);break;//K
						case 38:this->setKey(0x59,keyPressed);break;//L
						case 39:this->setKey(0x3B,keyPressed);break;//M
						case 44:this->setKey(0xAD,keyPressed);break;//W
						case 45:this->setKey(0x9B,keyPressed);break;//X
						case 46:this->setKey(0x8D,keyPressed);break;//C
						case 47:this->setKey(0x8F,keyPressed);break;//V
						case 48:this->setKey(0x7D,keyPressed);break;//B
						case 49:this->setKey(0x7B,keyPressed);break;//N
						case 57:this->setKey(0x7F,keyPressed);break;//espace
						case 28:this->setKey(0x39,keyPressed);break;//entrée
						case 50:this->setKey(0x6F,keyPressed);break;//,
						case 51:this->setKey(0x6D,keyPressed);break;//;
						case 52:this->setKey(0x3D,keyPressed);break;//:
						
						case 82:this->setKey(0x2F,keyPressed);break;//numpad 0
						case 79:this->setKey(0x27,keyPressed);break;//numpad 1
						case 80:this->setKey(0x17,keyPressed);break;//numpad 2
						case 81:this->setKey(0x21,keyPressed);break;//numpad 3
						case 75:this->setKey(0x19,keyPressed);break;//numpad 4
						case 76:this->setKey(0x29,keyPressed);break;//numpad 5
						case 77:this->setKey(0x11,keyPressed);break;//numpad 6
						case 71:this->setKey(0x2B,keyPressed);break;//numpad 7
						case 72:this->setKey(0x1B,keyPressed);break;//numpad 8
						case 73:this->setKey(0x2D,keyPressed);break;//numpad 9
						
						case 2:this->setKey(0xB3,keyPressed);break;//&
						case 3:this->setKey(0xB1,keyPressed);break;//é
						case 4:this->setKey(0xA7,keyPressed);break;//"
						case 5:this->setKey(0xA1,keyPressed);break;//'
						case 6:this->setKey(0x91,keyPressed);break;//(
						case 7:this->setKey(0x81,keyPressed);break;//-
						case 8:this->setKey(0x71,keyPressed);break;//è
						case 9:this->setKey(0x61,keyPressed);break;//_=!
						case 10:this->setKey(0x51,keyPressed);break;//ç
						case 11:this->setKey(0x53,keyPressed);break;//à
						case 12:this->setKey(0x37,keyPressed);break;//)
						
						case 43:this->setKey(0x1F,keyPressed);break;// *
						case 40:this->setKey(0x1D,keyPressed);break;//ù=#
						
						case 328:this->setKey(0x5B,keyPressed);break;//flèche haut
						case 331:this->setKey(0x5D,keyPressed);break;//flèche gauche
						case 336:this->setKey(0x5F,keyPressed);break;//flèche bas
						case 333:this->setKey(0x3F,keyPressed);break;//flèche droite
						
						case 54:
						case 42:this->setKey(0xAF,keyPressed);break;//shift
						case 58:this->setKey(0x9F,keyPressed);break;//min/maj
						case 285:
						case 29:this->setKey(0x9D,keyPressed);break;//ctrl
						
						case 56:this->setKey(0xA3,keyPressed);break;//alt=fnct
						case 1:this->setKey(0xA5,keyPressed);break;//échap=Esc
						
						case 41:this->setKey(0x55,keyPressed);break;//²=on/off
						case 13:this->setKey(0x31,keyPressed);break;//==mem
						case 15:this->setKey(0xB5,keyPressed);break;//tab=Connex/Fin
						case 78:this->setKey(0x23,keyPressed);break;//numpad +=HP+
						case 74:this->setKey(0x13,keyPressed);break;//numpad -=HP-
						case 14:this->setKey(0x35,keyPressed);break;//backspace=HP
						case 55:this->setKey(0x1F,keyPressed);break;//numpad *=*
						case 309:this->setKey(0x1D,keyPressed);break;//numpad /=#
						//.../... bis repertoire annuaire decrochage
						
						case 60:this->setKey(0x95,keyPressed);break;//F2=Sommaire
						case 61:this->setKey(0x93,keyPressed);break;//F3=Guide
						case 62:this->setKey(0x85,keyPressed);break;//F4=Annulation
						case 63:this->setKey(0x83,keyPressed);break;//F5=Correction
						case 64:this->setKey(0x75,keyPressed);break;//F6=Retour
						case 65:this->setKey(0x73,keyPressed);break;//F7=Suite
						case 66:this->setKey(0x65,keyPressed);break;//F8=Répétition
						case 67:this->setKey(0x63,keyPressed);break;//F9=Envoi
						
						case 0x153:this->setKey(0x33,keyPressed);break;
						default:printf("key pressed: %02X\n",scancode);break;
					}*/
				}
			}
		}
	private:
		std::atomic_bool keyState[88]={false};
		Keyboard* p_keyboard=NULL;
		
		struct KeyboardState{
			bool fnct;
			bool ctrl;
			bool shift;
			unsigned char key=0;//simulate keyboard 1KRO + fnct/ctrl/shift / not the same as the real hardware but needed for compatibility
		};
		KeyboardState currentKeyboardState;
		
		void setKey(unsigned char code,bool state){
			if (state!=this->keyState[(code-0x10)>>1].load(std::memory_order_relaxed)){
				if (this->p_keyboard!=NULL){
					this->p_keyboard->queueKey(code,state);
				}
			}
			this->keyState[(code-0x10)>>1].store(state,std::memory_order_relaxed);
		}
		bool getKey(unsigned char code){
			return this->keyState[(code-0x10)>>1].load(std::memory_order_relaxed);
		}
		void keyboardTransition(KeyboardState to){
			if (this->currentKeyboardState.key!=to.key&&this->currentKeyboardState.key>=0x11&&this->currentKeyboardState.key<=0xBF){
				this->setKey(this->currentKeyboardState.key,false);
			}
			if (this->currentKeyboardState.fnct!=to.fnct){
				this->setKey(0xA3,to.fnct);
			}
			if (this->currentKeyboardState.ctrl!=to.ctrl){
				this->setKey(0x9D,to.ctrl);
			}
			if (this->currentKeyboardState.shift!=to.shift){
				this->setKey(0xAF,to.shift);
			}
			if (this->currentKeyboardState.key!=to.key&&to.key>=0x11&&to.key<=0xBF){
				this->setKey(to.key,true);
			}
			this->currentKeyboardState=to;
		}
};
#endif