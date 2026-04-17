#ifndef FILESELECTOR_H
#define FILESELECTOR_H
#include <filesystem>
#include <vector>
#include <algorithm>
#include <functional>
#include <fstream>
class ROMSelector{
	public:
		ROMSelector(const char* dir){
			this->setDir(dir);
		}
		~ROMSelector(){
			if (this->searchDir!=NULL) free((void*)this->searchDir);
			if (this->ROMSelected!=NULL) free((void*)this->ROMSelected);
			std::for_each(this->ROMCandidate.begin(), this->ROMCandidate.end(), [](char* f) { free(f); });
		}
		void setDir(const char* dir){
			if (this->searchDir!=NULL) free(this->searchDir);
			this->searchDir=(char*)malloc(strlen(dir)+1);
			strcpy(this->searchDir,dir);
			if (!std::filesystem::is_directory(this->searchDir)) std::filesystem::create_directories(this->searchDir);
			this->Select(NULL);
		}
		void Select(const char* f){
			if (f==NULL){
				if (this->ROMSelected!=NULL) free(this->ROMSelected);
				this->ROMSelected=NULL;
				//callback
				this->selectionCallback(this->ROMSelected);
			}
			else if (this->isCandidate(f)){
				if (this->ROMSelected!=NULL) free(this->ROMSelected);
				this->ROMSelected=(char*)malloc(strlen(f)+1);
				strcpy(this->ROMSelected,f);
				this->ROMSelected[strlen(f)]=0;
				//callback
				this->selectionCallback(this->ROMSelected);
			}
		}
		char* getSelected(){
			return this->ROMSelected;
		}
		
		void widget(){
			bool rom_combo_state;
			if (this->ROMSelected==NULL){
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0,0,1));
				rom_combo_state=ImGui::BeginCombo("##rom", "<ROM introuvable>", 0);
				ImGui::PopStyleColor();
			}
			else{
				rom_combo_state=ImGui::BeginCombo("##rom", std::filesystem::path(this->ROMSelected).filename().stem().string().c_str(), 0);
			}
			
			if (rom_combo_state!=this->rom_combo_state_previous){
				if(rom_combo_state) this->updateFileList();
				
				this->rom_combo_state_previous=rom_combo_state;
			}
			
			if(rom_combo_state){
				std::for_each(this->ROMCandidate.begin(), this->ROMCandidate.end(), [this](char* f) {
					if (ImGui::Selectable(std::filesystem::path(f).filename().stem().string().c_str())){
						this->Select(f);
					}
				});
				ImGui::EndCombo();
			}
		}
		
		void setCallback(std::function<void(char*)> func){
			this->selectionCallback=func;
		}
		
	private:
		char* searchDir=NULL;
		
		const char* ext[6]={".rom",".bin",".erom",".dmp",".dump",""};
		const size_t file_size=0x40000;
		
		char* ROMSelected=NULL;
		std::vector<char*> ROMCandidate;
		
		bool rom_combo_state_previous=false;
		
		std::function<void(char*)> selectionCallback=[](char* f){};
		
		bool isCandidate(const char* f){
			if (!std::filesystem::is_regular_file(f)) return false;
			
			if (std::filesystem::file_size(f)!=this->file_size) return false;
			
			for (long long unsigned int i=0;i<sizeof(this->ext)/sizeof(this->ext[0]);i++){
				if (strcmp(this->ext[i],std::filesystem::path(f).extension().string().c_str())==0) return true;
			}
			return false;
		}
		void updateFileList(){
			std::for_each(this->ROMCandidate.begin(), this->ROMCandidate.end(), [](char* f) { free(f); });
			this->ROMCandidate.clear();
			if (!std::filesystem::is_directory(this->searchDir)) std::filesystem::create_directories(this->searchDir);
			for (const auto& entry : std::filesystem::directory_iterator(this->searchDir)) {
				if ((this->ROMSelected==NULL||!std::filesystem::equivalent(entry.path(),this->ROMSelected))&&this->isCandidate(entry.path().string().c_str())){
					size_t s=strlen(entry.path().string().c_str());
					char* f=(char*)malloc(s+1);
					strcpy(f,entry.path().string().c_str());
					f[s]=0;
					this->ROMCandidate.push_back(f);
				}
			}
		}
};
class RAMSelector{
	public:
		RAMSelector(const char* dir){
			this->setDir(dir);
		}
		~RAMSelector(){
			if (this->searchDir!=NULL) free((void*)this->searchDir);
			if (this->RAMSelected!=NULL) free((void*)this->RAMSelected);
			std::for_each(this->RAMCandidate.begin(), this->RAMCandidate.end(), [](char* f) { free(f); });
		}
		void setDir(const char* dir){
			if (this->searchDir!=NULL) free(this->searchDir);
			this->searchDir=(char*)malloc(strlen(dir)+1);
			strcpy(this->searchDir,dir);
			if (!std::filesystem::is_directory(this->searchDir)) std::filesystem::create_directories(this->searchDir);
			this->Select(NULL);
		}
		void Select(const char* f){
			if (f==NULL){
				if (this->RAMSelected!=NULL) free(this->RAMSelected);
				this->RAMSelected=NULL;
				//callback
				this->selectionCallback(this->RAMSelected);
			}
			else if (this->isCandidate(f)){
				if (this->RAMSelected!=NULL) free(this->RAMSelected);
				this->RAMSelected=(char*)malloc(strlen(f)+1);
				strcpy(this->RAMSelected,f);
				this->RAMSelected[strlen(f)]=0;
				//callback
				this->selectionCallback(this->RAMSelected);
			}
		}
		char* getSelected(){
			return this->RAMSelected;
		}
		
		void widget(){
			bool ram_combo_state;
			if (this->RAMSelected==NULL){
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.5,0,1));
				ram_combo_state=ImGui::BeginCombo("##ram", "<Invité>", 0);
				ImGui::PopStyleColor();
			}
			else{
				ram_combo_state=ImGui::BeginCombo("##ram", std::filesystem::path(this->RAMSelected).filename().stem().string().c_str(), 0);
			}
			
			if (ram_combo_state!=this->ram_combo_state_previous){
				if(ram_combo_state) this->updateFileList();
				
				this->ram_combo_state_previous=ram_combo_state;
			}
			
			if(ram_combo_state){
				if (this->RAMSelected!=NULL){
					ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,0.5,0,1));
					if (ImGui::Selectable("<Invité>")) this->Select(NULL);
					ImGui::PopStyleColor();
				}
				
				std::for_each(this->RAMCandidate.begin(), this->RAMCandidate.end(), [this](char* f) {
					if (ImGui::Selectable(std::filesystem::path(f).filename().stem().string().c_str())){
						this->Select(f);
					}
				});
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			if (ImGui::Button(this->new_profile?"-":"+")){
				this->new_profile=!this->new_profile;
				this->new_profile_name[0] = 0;
				this->disallow_new_profile=true;
			}
			
			ImGui::SameLine();
			bool no_profile=(this->RAMSelected==NULL);
			if (no_profile) ImGui::BeginDisabled();
			if (ImGui::Button("Supprimer")) {
				this->deleteSelectedFile();
			}
			if (no_profile) ImGui::EndDisabled();
			
			if (this->new_profile){
				if(ImGui::InputTextWithHint("##input_profile","Nouveau profil", this->new_profile_name, IM_ARRAYSIZE(this->new_profile_name),ImGuiInputTextFlags_CallbackCharFilter,this->FilterReservedLetters)){
					this->disallow_new_profile=this->profileFileExist(this->new_profile_name)||(strlen(this->new_profile_name)<4);
				}
				ImGui::SameLine();
				
				if (this->disallow_new_profile) ImGui::BeginDisabled();
				if(ImGui::Button("Créer")){
					addProfile(this->new_profile_name);
					
					this->new_profile_name[0] = 0;
					this->new_profile=false;
				}
				if (this->disallow_new_profile) ImGui::EndDisabled();
			}
		}
		
		void setCallback(std::function<void(char*)> func){
			this->selectionCallback=func;
		}
		
	private:
		char* searchDir=NULL;
		
		const char* ext[4]={".raw",".bin",".ram",".eram"};
		const size_t file_size=0x10000;
		
		char* RAMSelected=NULL;
		std::vector<char*> RAMCandidate;
		
		bool ram_combo_state_previous=false;
		bool new_profile=false;
		char new_profile_name[32]="";
		bool disallow_new_profile=true;
		
		std::function<void(char*)> selectionCallback=[](char* f){};
		
		bool isCandidate(const char* f){
			if (!std::filesystem::is_regular_file(f)) return false;
			
			if (std::filesystem::file_size(f)!=this->file_size) return false;
			
			for (long long unsigned int i=0;i<sizeof(this->ext)/sizeof(this->ext[0]);i++){
				if (strcmp(this->ext[i],std::filesystem::path(f).extension().string().c_str())==0) return true;
			}
			return false;
		}
		void updateFileList(){
			std::for_each(this->RAMCandidate.begin(), this->RAMCandidate.end(), [](char* f) { free(f); });
			this->RAMCandidate.clear();
			
			if (!std::filesystem::is_directory(this->searchDir)) std::filesystem::create_directories(this->searchDir);
			for (const auto& entry : std::filesystem::directory_iterator(this->searchDir)) {
				if ((this->RAMSelected==NULL||!std::filesystem::equivalent(entry,this->RAMSelected))&&this->isCandidate(entry.path().string().c_str())){
					
					size_t s=strlen(entry.path().string().c_str());
					char* f=(char*)malloc(s+1);
					strcpy(f,entry.path().string().c_str());
					f[s]=0;
					
					this->RAMCandidate.push_back(f);
				}
			}
		}
		void deleteSelectedFile(){
			char* f=this->RAMSelected;
			this->RAMSelected=NULL;
			//callback
			this->selectionCallback(this->RAMSelected);
			std::filesystem::remove(f);
			free(f);
		}
		void addProfile(const char* profile){
			/*size_t fpath_size=strlen(profile)+strlen(this->searchDir)+strlen(this->ext[0])+1;
			char* fpath=(char*)malloc(fpath_size);
			std::fill_n(fpath,fpath_size,0);
			strcpy(fpath,this->searchDir);
			strcat(fpath,profile);
			strcat(fpath,this->ext[0]);*/
			std::filesystem::path p=this->searchDir;
			p/=profile;
			p+=this->ext[0];
			char* fpath=(char*)malloc(strlen(p.string().c_str())+1);
			strcpy(fpath,p.string().c_str());
			fpath[strlen(p.string().c_str())]=0;
			
			std::ofstream ofs(fpath,std::ios::binary|std::ios::out);
			ofs.seekp(ERAM_SIZE-1);
			ofs.write("",1);
			ofs.close();
			
			if (this->RAMSelected!=NULL) free(this->RAMSelected);
			this->RAMSelected=fpath;
			//callback
			this->selectionCallback(this->RAMSelected);
		}
		bool profileFileExist(const char* profile){
			size_t fpath_size=strlen(profile)+strlen(this->searchDir)+strlen(this->ext[0])+1;
			char* fpath=(char*)malloc(fpath_size);
			std::fill_n(fpath,fpath_size,0);
			strcpy(fpath,this->searchDir);
			strcat(fpath,profile);
			strcat(fpath,this->ext[0]);
			bool e=std::filesystem::exists(std::filesystem::path(fpath));
			free((void*)fpath);
			return e;
		}
		static int FilterReservedLetters(ImGuiInputTextCallbackData* data){
			if (data->EventChar < 256 && data->EventChar > 31 && !strchr("/\\\"<>:|?*.", (char)data->EventChar)) return 0;
			return 1;
		}
};
#endif