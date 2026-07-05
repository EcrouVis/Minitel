#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H
#include <atomic>
#include "circuit/Keyboard.h"

constexpr int M12_KEY_ESCAPE=256;

constexpr int M12_KEY_F2=291;
constexpr int M12_KEY_F3=292;
constexpr int M12_KEY_F4=293;
constexpr int M12_KEY_F5=294;
constexpr int M12_KEY_F6=295;
constexpr int M12_KEY_F7=296;
constexpr int M12_KEY_F8=297;
constexpr int M12_KEY_F9=298;

constexpr int M12_KEY_SQUARE=96;
constexpr int M12_KEY_1=49;
constexpr int M12_KEY_2=50;
constexpr int M12_KEY_3=51;
constexpr int M12_KEY_4=52;
constexpr int M12_KEY_5=53;
constexpr int M12_KEY_6=54;
constexpr int M12_KEY_7=55;
constexpr int M12_KEY_8=56;
constexpr int M12_KEY_9=57;
constexpr int M12_KEY_0=48;
constexpr int M12_KEY_DEGREE=45;
constexpr int M12_KEY_PLUS=61;
constexpr int M12_KEY_BACKSPACE=259;

constexpr int M12_KEY_TAB=258;
constexpr int M12_KEY_A=81;
constexpr int M12_KEY_Z=87;
constexpr int M12_KEY_E=69;
constexpr int M12_KEY_R=82;
constexpr int M12_KEY_T=84;
constexpr int M12_KEY_Y=89;
constexpr int M12_KEY_U=85;
constexpr int M12_KEY_I=73;
constexpr int M12_KEY_O=79;
constexpr int M12_KEY_P=80;
constexpr int M12_KEY_DIAERESIS=91;
constexpr int M12_KEY_POUND=93;
constexpr int M12_KEY_ENTER=257;

constexpr int M12_KEY_CAPS_LOCK=280;
constexpr int M12_KEY_Q=65;
constexpr int M12_KEY_S=83;
constexpr int M12_KEY_D=68;
constexpr int M12_KEY_F=70;
constexpr int M12_KEY_G=71;
constexpr int M12_KEY_H=72;
constexpr int M12_KEY_J=74;
constexpr int M12_KEY_K=75;
constexpr int M12_KEY_L=76;
constexpr int M12_KEY_M=59;
constexpr int M12_KEY_PERCENT=39;
constexpr int M12_KEY_MU=92;

constexpr int M12_KEY_LEFT_SHIFT=340;
constexpr int M12_KEY_GREATER_THAN=162;
constexpr int M12_KEY_W=90;
constexpr int M12_KEY_X=88;
constexpr int M12_KEY_C=67;
constexpr int M12_KEY_V=86;
constexpr int M12_KEY_B=66;
constexpr int M12_KEY_N=78;
constexpr int M12_KEY_QUESTION=77;
constexpr int M12_KEY_PERIOD=44;
constexpr int M12_KEY_SLASH=46;
constexpr int M12_KEY_SECTION=47;
constexpr int M12_KEY_RIGHT_SHIFT=344;

constexpr int M12_KEY_LEFT_CONTROL=341;
constexpr int M12_KEY_ALT=342;
constexpr int M12_KEY_SPACE=32;
constexpr int M12_KEY_ALTGR=346;
constexpr int M12_KEY_RIGHT_CONTROL=345;
constexpr int M12_KEY_UP=265;
constexpr int M12_KEY_LEFT=263;
constexpr int M12_KEY_DOWN=264;
constexpr int M12_KEY_RIGHT=262;

constexpr int M12_KEY_KP_0=320;
constexpr int M12_KEY_KP_1=321;
constexpr int M12_KEY_KP_2=322;
constexpr int M12_KEY_KP_3=323;
constexpr int M12_KEY_KP_4=324;
constexpr int M12_KEY_KP_5=325;
constexpr int M12_KEY_KP_6=326;
constexpr int M12_KEY_KP_7=327;
constexpr int M12_KEY_KP_8=328;
constexpr int M12_KEY_KP_9=329;
constexpr int M12_KEY_KP_DECIMAL=330;
constexpr int M12_KEY_KP_DIVIDE=331;
constexpr int M12_KEY_KP_MULTIPLY=332;
constexpr int M12_KEY_KP_SUBSTRACT=333;
constexpr int M12_KEY_KP_ADD=334;
constexpr int M12_KEY_KP_ENTER=335;
constexpr int M12_KEY_KP_EQUAL=336;

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
			
			activated=this->getKey(0x45);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			if (shift) ImGui::Button("Impr.",ImVec2(bs,0));
			else ImGui::Button("(_)",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x45,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x45,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x15);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("Bis",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x15,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x15,false);
			
			ImGui::SameLine();
			
			activated=this->getKey(0x33);
			if (activated) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
			ImGui::Button("[V]>",ImVec2(bs,0));
			if (activated) ImGui::PopStyleColor();
			if(ImGui::IsItemActivated()) this->setKey(0x33,true);
			if(ImGui::IsItemDeactivated()) this->setKey(0x33,false);
			
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
		
		void InputTranslate(bool focus, int key, int action, int mods){
			if (focus||action==GLFW_RELEASE){//||action==GLFW_RELEASE temporary fix to avoid being stuck when ctrl+click on imgui slider
				if (action==GLFW_PRESS||action==GLFW_RELEASE){
					bool keyPressed=(action==GLFW_PRESS);
					bool altgr=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==(GLFW_MOD_CONTROL|GLFW_MOD_ALT));
					KeyboardState kbs;
					kbs.fnct=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==GLFW_MOD_ALT);
					kbs.ctrl=((mods&(GLFW_MOD_CONTROL|GLFW_MOD_ALT))==GLFW_MOD_CONTROL);
					kbs.shift=(bool)(mods&GLFW_MOD_SHIFT);
					kbs.key=0;
					
					if (key==M12_KEY_LEFT_CONTROL||key==M12_KEY_RIGHT_CONTROL||key==M12_KEY_LEFT_SHIFT||key==M12_KEY_RIGHT_SHIFT||key==M12_KEY_ALT||key==M12_KEY_ALTGR){
						kbs.key=this->currentKeyboardState.key;
					}
					else if (keyPressed){
						if (altgr){
							switch (key){
								//compatibility
								case M12_KEY_0://à
									kbs.key=0x73;//Suite
									kbs.ctrl=true;
									break;
									
								case M12_KEY_4://'
									kbs.key=0x27;//numpad 1
									kbs.ctrl=true;
									break;
									
								case M12_KEY_PLUS://=
									kbs.key=0x21;//numpad 3
									kbs.ctrl=true;
									break;
									
								case M12_KEY_6://-
									kbs.key=0x17;//numpad 2
									kbs.ctrl=true;
									break;
									
								case M12_KEY_2://é
									kbs.key=0x19;//numpad 4
									kbs.ctrl=true;
									break;
									
								case M12_KEY_7://è
									kbs.key=0x29;//numpad 5
									kbs.ctrl=true;
									break;
									
								case M12_KEY_3://"
									kbs.key=0x1D;//#
									break;
									
								case M12_KEY_5://(
									kbs.key=0x1F;// *
									kbs.shift=true;
									break;
									
								case M12_KEY_9://ç
									kbs.key=0x2F;//numpad 0
									kbs.shift=true;
									break;
									
								case M12_KEY_DEGREE://)
									kbs.key=0x1D;//#
									kbs.shift=true;
									break;
									
								case M12_KEY_8://_
									kbs.key=0x21;//numpad 3
									kbs.shift=true;
									break;
									
								//special phone keys
								case M12_KEY_P://P
									kbs.key=0x23;//Volume +
									break;
									
								case M12_KEY_M://M
									kbs.key=0x13;//Volume -
									break;
									
								case M12_KEY_A://A
									kbs.key=0x45;//appel Annuaire
									break;
									
								case M12_KEY_Z://Z
									kbs.key=0x25;//appel n°51
									break;
									
								case M12_KEY_R://R
									kbs.key=0x33;//appel Répertoire
									break;
									
								case M12_KEY_B://B
									kbs.key=0x15;//appel Bis
									break;
									
								case M12_KEY_T://T
									kbs.key=0x35;//téléphone
									break;
									
								case M12_KEY_I://I
									kbs.key=0x45;//Impr.
									kbs.shift=true;
									break;
							}
						}
						else{
							switch (key){
								case M12_KEY_A:kbs.key=0xBB;break;//A
								case M12_KEY_Z:kbs.key=0xB7;break;//Z
								case M12_KEY_E:kbs.key=0xB9;break;//E
								case M12_KEY_R:kbs.key=0xA9;break;//R
								case M12_KEY_T:kbs.key=0x97;break;//T
								case M12_KEY_Y:kbs.key=0x87;break;//Y
								case M12_KEY_U:kbs.key=0x77;break;//U
								case M12_KEY_I:kbs.key=0x67;break;//I
								case M12_KEY_O:kbs.key=0x69;break;//O
								case M12_KEY_P:kbs.key=0x57;break;//P
								case M12_KEY_Q:kbs.key=0xBF;break;//Q
								case M12_KEY_S:kbs.key=0xBD;break;//S
								case M12_KEY_D:kbs.key=0xAB;break;//D
								case M12_KEY_F:kbs.key=0x99;break;//F
								case M12_KEY_G:kbs.key=0x8B;break;//G
								case M12_KEY_H:kbs.key=0x89;break;//H
								case M12_KEY_J:kbs.key=0x79;break;//J
								case M12_KEY_K:kbs.key=0x6B;break;//K
								case M12_KEY_L:kbs.key=0x59;break;//L
								case M12_KEY_M:kbs.key=0x3B;break;//M
								case M12_KEY_W:kbs.key=0xAD;break;//W
								case M12_KEY_X:kbs.key=0x9B;break;//X
								case M12_KEY_C:kbs.key=0x8D;break;//C
								case M12_KEY_V:kbs.key=0x8F;break;//V
								case M12_KEY_B:kbs.key=0x7D;break;//B
								case M12_KEY_N:kbs.key=0x7B;break;//N
								case M12_KEY_SPACE:kbs.key=0x7F;break;//espace
								case M12_KEY_KP_ENTER:
								case M12_KEY_ENTER:kbs.key=0x39;break;//entrée
								case M12_KEY_QUESTION:kbs.key=0x6F;break;//,
								case M12_KEY_KP_DECIMAL:
									kbs.shift=true;
									[[fallthrough]];
								case M12_KEY_PERIOD:kbs.key=0x6D;break;//;
								case M12_KEY_KP_DIVIDE:
									kbs.shift=true;
									[[fallthrough]];
								case M12_KEY_SLASH:kbs.key=0x3D;break;//:
								
								case M12_KEY_KP_0:kbs.key=0x2F;break;//numpad 0
								case M12_KEY_KP_1:kbs.key=0x27;break;//numpad 1
								case M12_KEY_KP_2:kbs.key=0x17;break;//numpad 2
								case M12_KEY_KP_3:kbs.key=0x21;break;//numpad 3
								case M12_KEY_KP_4:kbs.key=0x19;break;//numpad 4
								case M12_KEY_KP_5:kbs.key=0x29;break;//numpad 5
								case M12_KEY_KP_6:kbs.key=0x11;break;//numpad 6
								case M12_KEY_KP_7:kbs.key=0x2B;break;//numpad 7
								case M12_KEY_KP_8:kbs.key=0x1B;break;//numpad 8
								case M12_KEY_KP_9:kbs.key=0x2D;break;//numpad 9
								
								case M12_KEY_F2:kbs.key=0x95;break;//F2=Sommaire
								case M12_KEY_F3:kbs.key=0x93;break;//F3=Guide
								case M12_KEY_F4:kbs.key=0x85;break;//F4=Annulation
								case M12_KEY_F5:kbs.key=0x83;break;//F5=Correction
								case M12_KEY_F6:kbs.key=0x75;break;//F6=Retour
								case M12_KEY_F7:kbs.key=0x73;break;//F7=Suite
								case M12_KEY_F8:kbs.key=0x65;break;//F8=Répétition
								case M12_KEY_F9:kbs.key=0x63;break;//F9=Envoi
								
								case M12_KEY_SQUARE:kbs.key=0x55;break;//²=on/off            ///////////////////////////////////////////////////////////////////////
								case M12_KEY_BACKSPACE:kbs.key=0x31;break;//backspace=mem       ///////////////////////////////////////////////////////////////////////
								case M12_KEY_TAB:kbs.key=0xB5;break;//tab=Connex/Fin      ///////////////////////////////////////////////////////////////////////
								case M12_KEY_ESCAPE:kbs.key=0xA5;break;//échap=Esc
								case M12_KEY_CAPS_LOCK:kbs.key=0x9F;break;//min/maj
						
								case M12_KEY_UP:kbs.key=0x5B;break;//flèche haut
								case M12_KEY_LEFT:kbs.key=0x5D;break;//flèche gauche
								case M12_KEY_DOWN:kbs.key=0x5F;break;//flèche bas
								case M12_KEY_RIGHT:kbs.key=0x3F;break;//flèche droite
								
								case M12_KEY_1:kbs.key=0xB3;break;//&
								case M12_KEY_2:kbs.key=0xB1;break;//é
								case M12_KEY_3:kbs.key=0xA7;break;//"
								case M12_KEY_4:kbs.key=0xA1;break;//'
								case M12_KEY_5:kbs.key=0x91;break;//(
								case M12_KEY_KP_SUBSTRACT:
									kbs.shift=false;
									[[fallthrough]];
								case M12_KEY_6:kbs.key=0x81;break;//-
								case M12_KEY_7:kbs.key=0x71;break;//è
								case M12_KEY_9:kbs.key=0x51;break;//ç
								case M12_KEY_0:kbs.key=0x53;break;//à
								case M12_KEY_DEGREE:kbs.key=0x37;break;//)
								
								//compatibility
								case M12_KEY_8://_
									if (kbs.ctrl||kbs.shift) kbs.key=0x61;// !
									else{
										kbs.key=0x2D;//numpad 9
										kbs.shift=true;
									}
									break;
								
								case M12_KEY_SECTION:// !
									if (kbs.shift) kbs.key=0x11;//numpad 6
									else kbs.key=0x61;// !
									break;
								
								case M12_KEY_KP_MULTIPLY:
									kbs.shift=false;
									[[fallthrough]];
								case M12_KEY_MU:// *
									if (!kbs.shift){
										kbs.key=0x1F;// *
									}
									break;
									
								case M12_KEY_DIAERESIS://^
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
									
								case M12_KEY_POUND://$
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
									
								case M12_KEY_PERCENT://ù
									if (kbs.shift) kbs.key=0x29;//numpad 5
									else{
										kbs.key=0x83;//Correction
										kbs.ctrl=true;
									}
									break;
								
								case M12_KEY_KP_EQUAL:
									kbs.key=0x1B;//numpad 8
									kbs.shift=true;
									break;
								case M12_KEY_KP_ADD:
									kbs.shift=true;
									[[fallthrough]];
								case M12_KEY_PLUS://=
									if (kbs.shift) kbs.key=0x2B;//numpad 7
									else{
										kbs.key=0x1B;//numpad 8
										kbs.shift=true;
									}
									break;
									
								case M12_KEY_GREATER_THAN://<
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