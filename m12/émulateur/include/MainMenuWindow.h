#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include "Parameters.h"
#include "circuit/SRAM_64k.h"
#include "circuit/ROM_256k.h"

#include <vector>
#include <fstream>
#include <cstdio>

#define MA_NO_DECODING
#define MA_NO_ENCODING
#include "miniaudio/miniaudio.h"

#include "thread_messaging.h"

const char* filenameCharFilter="/\\\"<>:|?*.";

const char* profileDir="./profils/";
std::vector<const char*> profileExt={".raw",".bin",".ram",".eram"};
const char* profileGuest="<Invité>";

const char* romDir="./rom/";
std::vector<const char*> romExt={".rom",".bin",".erom",".dmp",".dump",""};
const char* romUnavailable="<ROM introuvable>";

const char* resDir="./ressources/";
std::vector<const char*> fontExt={".ttf",".otf"};
const char* fontDefault="default";

ma_context initAudio(){
	ma_context context;

    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        printf("Failed to initialize context.\n");
    }
	return context;
}

static int FilterReservedLetters(ImGuiInputTextCallbackData* data){
	if (data->EventChar < 256 && data->EventChar > 31 && !strchr(filenameCharFilter, (char)data->EventChar)) return 0;
	return 1;
}

std::vector<const char*> initFileList(const char* last_file,const char* fallback_char,unsigned int* p_item){
	std::vector<const char*> items={fallback_char};
	*p_item=0;
	if (last_file!=NULL){
		char* last_file_cpy=(char*)malloc(strlen(last_file)+1);
		std::fill_n(last_file_cpy,strlen(last_file)+1,0);
		strcpy(last_file_cpy,last_file);
		items.push_back((const char*)last_file_cpy);
		*p_item=1;
	}
	return items;
}
void updateFileList(std::vector<const char*>* p_items,unsigned int* p_item,const char* dir,std::vector<const char*> file_ext,const int file_size=-1){
	if (*p_item>=p_items->size()) *p_item=0;
	char* last=(char*)malloc(strlen((*p_items)[*p_item])+1);
	strcpy(last,(*p_items)[*p_item]);
	last[strlen((*p_items)[*p_item])]=0;
	while (p_items->size()>1){
		free((void*)p_items->back());
		p_items->pop_back();
	}
	//update list
	for (const auto& entry : std::filesystem::directory_iterator(dir)) {
		if (entry.is_regular_file()){
			const char* ext=entry.path().extension().string().c_str();
			bool b_ext=false;
			for (const char* f_ext:file_ext){
				if (strcmp(ext,f_ext)==0){
					b_ext=true;
					break;
				}
			}
			if(b_ext&&(entry.file_size()==(unsigned int)file_size||file_size<0)){
				const char* name=entry.path().string().c_str();//.filename().stem()
				char* name_cpy=(char*)malloc(strlen(name)+1);
				strcpy(name_cpy,name);
				name_cpy[strlen(name)]=0;
				p_items->push_back((const char*)name_cpy);
			}
		}
	}
	//reselect same if possible
	*p_item=0;
	for (unsigned int i=0;i<p_items->size();i++){
		if (strcmp(last,(*p_items)[i])==0){
			*p_item=i;
			break;
		}
	}
	free((void*)last);
}
void addProfile(std::vector<const char*>* p_items,unsigned int* p_item,const char* profile){
	*p_item=p_items->size();
	size_t fpath_size=strlen(profile)+strlen(profileDir)+strlen(profileExt[0])+1;
	char* fpath=(char*)malloc(fpath_size);
	std::fill_n(fpath,fpath_size,0);
	strcpy(fpath,profileDir);
	strcat(fpath,profile);
	strcat(fpath,profileExt[0]);
	std::ofstream ofs(fpath,std::ios::binary|std::ios::out);
	ofs.seekp(ERAM_SIZE-1);
	ofs.write("",1);
	ofs.close();
	p_items->push_back((const char*)fpath);//free((void*)fpath);
	updateFileList(p_items,p_item,profileDir,profileExt,ERAM_SIZE);
}
void deleteProfile(std::vector<const char*>* p_items,unsigned int* p_item){
	std::remove((*p_items)[*p_item]);
	updateFileList(p_items,p_item,profileDir,profileExt,ERAM_SIZE);
}
bool profileFileExist(const char* profile){
	size_t fpath_size=strlen(profile)+strlen(profileDir)+strlen(profileExt[0])+1;
	char* fpath=(char*)malloc(fpath_size);
	std::fill_n(fpath,fpath_size,0);
	strcpy(fpath,profileDir);
	strcat(fpath,profile);
	strcat(fpath,profileExt[0]);
	bool e=std::filesystem::exists(std::filesystem::path(fpath));
	free((void*)fpath);
	return e;
}
const char* profileNiceName(const char* profile){
	return (strcmp(profile,profileGuest)==0)?profile:std::filesystem::path(profile).filename().stem().string().c_str();
}
const char* romNiceName(const char* rom){
	return (strcmp(rom,romUnavailable)==0)?rom:std::filesystem::path(rom).filename().stem().string().c_str();
}

void sendCommandMEM(thread_mailbox* p_mb_circuit,const int cmd,const char* str=NULL){
	thread_message ms;
	ms.cmd=cmd;
	if (str==NULL){
		ms.p=NULL;
	}
	else{
		ms.p=malloc(strlen(str)+1);
		strcpy((char*)ms.p,str);
		((char*)ms.p)[strlen(str)]=0;
	}
	thread_send_message(p_mb_circuit,&ms);
}
	
void mainMenuWindow(Parameters* p_params,thread_mailbox* p_mb_circuit){
	ImGui::Begin("Menu",&(p_params->imgui.show_menu),ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Appuyez sur F1 pour afficher/cacher le menu");
	if (ImGui::CollapsingHeader("UI")){
		ImGui::Text("DPI");
		ImGui::SameLine();
		if(ImGui::Button("-##dpi")) ImGui::GetStyle().FontScaleDpi-=0.1;
		ImGui::SameLine();
		if(ImGui::Button("+##dpi")) ImGui::GetStyle().FontScaleDpi+=0.1;
	}
	if (ImGui::CollapsingHeader("Contrôle de l'émulateur")){
		if(p_params->p_gState->minitelOn.load(std::memory_order_relaxed)){
			if(ImGui::Button("Eteindre")){
				thread_message ms;
				ms.cmd=EMU_OFF;
				thread_send_message(p_mb_circuit,&ms);
			}
		}
		else{
			if(ImGui::Button("Allumer")){
				thread_message ms;
				ms.cmd=EMU_ON;
				thread_send_message(p_mb_circuit,&ms);
			}
		}
	}
	if (ImGui::CollapsingHeader("Système de fichiers")){
		bool disable_load_memory=p_params->p_gState->minitelOn.load(std::memory_order_relaxed);
		if (disable_load_memory) ImGui::BeginDisabled();
			
			ImGui::SeparatorText("Copie de la mémoire ROM du minitel:");
			static unsigned int rom = 0;
			static std::vector<const char*> roms=initFileList(p_params->fs.erom,romUnavailable,&rom);
			static bool rom_combo_state_previous=true;
			bool rom_combo_state=ImGui::BeginCombo("##rom", roms[rom], 0);
			if (rom_combo_state!=rom_combo_state_previous){
				updateFileList(&roms,&rom,romDir,romExt,EROM_SIZE);
				
				if(!rom_combo_state) sendCommandMEM(p_mb_circuit,LOAD_EROM,(rom==0)?NULL:roms[rom]);
				/*thread_message ms;
				ms.cmd=LOAD_EROM;
				if (rom==0){
					ms.p=NULL;
				}
				else{
					ms.p=malloc(strlen(roms[rom])+1);
					strcpy((char*)ms.p,roms[rom]);
					((char*)ms.p)[strlen(roms[rom])]=0;
				}
				thread_send_message(p_mb_circuit,&ms);*/
				
				rom_combo_state_previous=rom_combo_state;
			}
			if(rom_combo_state){
				for (unsigned int i=0;i<roms.size();i++) if(i!=0&&i!=rom&&ImGui::Selectable(roms[i])) rom=i;
				ImGui::EndCombo();
			}
			ImGui::SeparatorText("Profil - Sauvegarde de la mémoire RAM du minitel:");
			static unsigned int item = 0;
			static std::vector<const char*> items=initFileList(p_params->fs.eram,profileGuest,&item);
			static bool profile_combo_state_previous=true;
			bool profile_combo_state=ImGui::BeginCombo("##profile", items[item], 0);
			if (profile_combo_state!=profile_combo_state_previous){
				updateFileList(&items,&item,profileDir,profileExt,ERAM_SIZE);
				
				if(!profile_combo_state) sendCommandMEM(p_mb_circuit,LOAD_ERAM,(item==0)?NULL:items[item]);
				/*thread_message ms;
				ms.cmd=LOAD_ERAM;
				if (item==0){
					ms.p=NULL;
				}
				else{
					ms.p=malloc(strlen(items[item])+1);
					strcpy((char*)ms.p,items[item]);
					((char*)ms.p)[strlen(items[item])]=0;
				}
				thread_send_message(p_mb_circuit,&ms);*/
				
				profile_combo_state_previous=profile_combo_state;
			}
			if(profile_combo_state){
				for (unsigned int i=0;i<items.size();i++) if(i!=item&&ImGui::Selectable(items[i])) item=i;
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			static bool new_profile=false;
			static char profile[32] = "";
			if (ImGui::Button(new_profile?"-":"+")){
				new_profile=!new_profile;
				profile[0] = 0;
			}
			ImGui::SameLine();
			bool no_profile=(item==0);
			if (no_profile) ImGui::BeginDisabled();
			if (ImGui::Button("Supprimer")) {
				deleteProfile(&items,&item);
				sendCommandMEM(p_mb_circuit,LOAD_ERAM,NULL);
			}
			if (no_profile) ImGui::EndDisabled();
			if (new_profile){
				static bool disallow_profile=true;
				if(ImGui::InputTextWithHint("##input_profile","Nouveau profil", profile, IM_ARRAYSIZE(profile),ImGuiInputTextFlags_CallbackCharFilter,FilterReservedLetters)) disallow_profile=profileFileExist(profile);
				ImGui::SameLine();
				disallow_profile=disallow_profile||strlen(profile)<4;
				
				if (disallow_profile) ImGui::BeginDisabled();
				if(ImGui::Button("Créer")){
					addProfile(&items,&item,profile);
					sendCommandMEM(p_mb_circuit,LOAD_ERAM,(item==0)?NULL:items[item]);
					profile[0] = 0;
					new_profile=false;
				}
				if (disallow_profile) ImGui::EndDisabled();
			}
		
		if (disable_load_memory) ImGui::EndDisabled();
		
		ImGui::SeparatorText("Texture des caractères fixes:");
		
	}
	if (ImGui::CollapsingHeader("Entrées/Sorties")){
		static ma_context audio_context=initAudio();
		static ma_device_info* pPlaybackDeviceInfos;
		static ma_uint32 playbackDeviceCount;
		static ma_device_info* pCaptureDeviceInfos;
		static ma_uint32 captureDeviceCount;
		
		ImGui::SeparatorText("Clavier");
		
		ImGui::SeparatorText("Prise péri-informatique");
		static int peri_io=0;
		ImGui::RadioButton("Débranchée##peri", &peri_io, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Socket UNIX##peri", &peri_io, 1);
		if (peri_io==1){
			ImGui::Indent();
			ImGui::Text("Socket:");
			ImGui::Unindent();
		}
		ImGui::Checkbox("Afficher les notifications##peri",&(p_params->io.peri.notify_state));
		ImGui::TextDisabled("Le signal PT est considéré comme toujours actif.");
		
		ImGui::SeparatorText("Modem");
		static int modem_io=0;
		ImGui::RadioButton("Débranché##modem", &modem_io, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Socket UNIX##modem", &modem_io, 1);
		ImGui::SameLine();
		ImGui::RadioButton("Prise audio##modem", &modem_io, 2);
		if (modem_io==1){
			ImGui::Indent();
			ImGui::Text("Socket:");
			ImGui::Unindent();
		}
		if (modem_io==2){
			ImGui::Indent();
			ImGui::Text("Entrée:");
			static ma_uint32 modem_audio_i=0;
			static bool modem_i_combo_state_previous=true;
			bool modem_i_combo_state=ImGui::BeginCombo("##modem_i", (modem_audio_i>=captureDeviceCount)?"":pCaptureDeviceInfos[modem_audio_i].name, 0);
			if (modem_i_combo_state!=modem_i_combo_state_previous){
				ma_result result;
				if (modem_audio_i<captureDeviceCount){
					ma_device_id id=pCaptureDeviceInfos[modem_audio_i].id;
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
					modem_audio_i=0;
					for(ma_uint32 i=0;i<captureDeviceCount;i++){
						if(ma_device_id_equal(&(pCaptureDeviceInfos[i].id),&id)){
							modem_audio_i=i;
							break;
						}
					}
				}
				else{
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
				}
				if (result != MA_SUCCESS) {
					printf("Failed to retrieve device information.\n");
				}
				modem_i_combo_state_previous=modem_i_combo_state;
			}
			if(modem_i_combo_state){
				for (unsigned int i=0;i<captureDeviceCount;i++) if(i!=modem_audio_i&&ImGui::Selectable(pCaptureDeviceInfos[i].name)) modem_audio_i=i;
				ImGui::EndCombo();
			}
			ImGui::Text("Sortie:");
			static ma_uint32 modem_audio_o=0;
			static bool modem_o_combo_state_previous=true;
			bool modem_o_combo_state=ImGui::BeginCombo("##modem_o", (modem_audio_o>=playbackDeviceCount)?"":pPlaybackDeviceInfos[modem_audio_o].name, 0);
			if (modem_o_combo_state!=modem_o_combo_state_previous){
				ma_result result;
				if (modem_audio_o<playbackDeviceCount){
					ma_device_id id=pPlaybackDeviceInfos[modem_audio_o].id;
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
					modem_audio_o=0;
					for(ma_uint32 i=0;i<playbackDeviceCount;i++){
						if(ma_device_id_equal(&(pPlaybackDeviceInfos[i].id),&id)){
							modem_audio_o=i;
							break;
						}
					}
				}
				else{
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
				}
				if (result != MA_SUCCESS) {
					printf("Failed to retrieve device information.\n");
				}
				modem_o_combo_state_previous=modem_o_combo_state;
			}
			if(modem_o_combo_state){
				for (unsigned int i=0;i<playbackDeviceCount;i++) if(i!=modem_audio_o&&ImGui::Selectable(pPlaybackDeviceInfos[i].name)) modem_audio_o=i;
				ImGui::EndCombo();
			}
			ImGui::Unindent();
		}
		ImGui::Checkbox("Afficher les notifications##modem",&(p_params->io.modem.notify_state));
		
		ImGui::SeparatorText("Buzzer");
		static int buzzer_o=0;
		ImGui::RadioButton("Débranché##buzzer", &buzzer_o, 0);
		ImGui::SameLine();
		ImGui::RadioButton("Prise audio##buzzer", &buzzer_o, 1);
		if (buzzer_o==1){
			ImGui::Indent();
			ImGui::Text("Sortie:");
			static ma_uint32 buzzer_audio_o=0;
			static bool buzzer_o_combo_state_previous=true;
			bool buzzer_o_combo_state=ImGui::BeginCombo("##buzzer", (buzzer_audio_o>=playbackDeviceCount)?"":pPlaybackDeviceInfos[buzzer_audio_o].name, 0);
			if (buzzer_o_combo_state!=buzzer_o_combo_state_previous){
				ma_result result;
				if (buzzer_audio_o<playbackDeviceCount){
					ma_device_id id=pPlaybackDeviceInfos[buzzer_audio_o].id;
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
					buzzer_audio_o=0;
					for(ma_uint32 i=0;i<playbackDeviceCount;i++){
						if(ma_device_id_equal(&(pPlaybackDeviceInfos[i].id),&id)){
							buzzer_audio_o=i;
							break;
						}
					}
				}
				else{
					result = ma_context_get_devices(&audio_context, &pPlaybackDeviceInfos, &playbackDeviceCount, &pCaptureDeviceInfos, &captureDeviceCount);
				}
				if (result != MA_SUCCESS) {
					printf("Failed to retrieve device information.\n");
				}
				buzzer_o_combo_state_previous=buzzer_o_combo_state;
			}
			if(buzzer_o_combo_state){
				for (unsigned int i=0;i<playbackDeviceCount;i++) if(i!=buzzer_audio_o&&ImGui::Selectable(pPlaybackDeviceInfos[i].name)) buzzer_audio_o=i;
				ImGui::EndCombo();
			}
			ImGui::Unindent();
		}
		ImGui::Checkbox("Afficher les notifications##buzzer",&(p_params->io.buzzer.notify_state));
		
		ImGui::SeparatorText("CRT");
		ImGui::Text("Facteur d'étirement de l'image");
		ImGui::Indent();
		ImGui::SliderFloat("##crt_width", &(p_params->io.crt.width_factor), 0.5, 1.5, "x%.3f");
		ImGui::SameLine();
		if(ImGui::Button("Réinitialiser")) p_params->io.crt.width_factor=1.;
		ImGui::Unindent();
		ImGui::Checkbox("Sortie vidéo RGB",&(p_params->io.crt.rgb));
		ImGui::SameLine();
		ImGui::TextDisabled("(hack)");
		static bool crt_filter=false;
		ImGui::Checkbox("Filtre vidéo CRT",&crt_filter);
	}
	if (ImGui::CollapsingHeader("Débuggage")){
		static bool sbs=false;
		ImGui::Checkbox("Execution pas à pas",&sbs);
		p_params->p_gState->stepByStep.store(sbs,std::memory_order_relaxed);
		if (sbs){
			ImGui::Indent();
			if(ImGui::Button("Instruction suivante")){
				thread_message ms;
				ms.cmd=EMU_NEXT_STEP;
				thread_send_message(p_mb_circuit,&ms);
			}
			ImGui::Unindent();
		}
		if (p_params->debug.eram.mem!=NULL){
			ImGui::Checkbox("Afficher le contenu de la RAM externe",&(p_params->debug.eram.show));
		}
		if (p_params->debug.erom.mem!=NULL){
			ImGui::Checkbox("Afficher le contenu de la ROM externe",&(p_params->debug.erom.show));
		}
		if (p_params->debug.iram.mem!=NULL){
			ImGui::Checkbox("Afficher le contenu de la RAM interne",&(p_params->debug.iram.show));
		}
		ImGui::Text("Statistiques");
	}
	if (ImGui::CollapsingHeader("À propos")){
		ImGui::Text(p_params->info.title);
		ImGui::Text("Développé par:");
		ImGui::Text(p_params->info.programmer);
		ImGui::Text("Bibliothèques:");
		for (License l:p_params->info.lib_licenses){
			if (ImGui::TreeNode(l.title)){
				ImGui::TextWrapped(l.content);
				ImGui::TreePop();
			}
		}
		ImGui::Text("Font:");
		for (License l:p_params->info.font_licenses){
			if (ImGui::TreeNode(l.title)){
				ImGui::TextWrapped(l.content);
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

#endif