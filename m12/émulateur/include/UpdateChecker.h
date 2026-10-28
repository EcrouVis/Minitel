#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H
#include <thread>
#include <atomic>

#include "imgui.h"

#include "desktop/check_update.h"
#include "Parameters.h"

class UpdateChecker{
	public:
		UpdateChecker(Parameters* p){
			this->pPARAMETERS=p;
			if(this->pPARAMETERS->info.check_update_startup) this->check();
		}
		~UpdateChecker(){
			if (this->thrd.joinable()) this->thrd.join();
		}
		void widget(){
			ImGui::Text("%s (ver.:%s)",this->pPARAMETERS->info.title,this->pPARAMETERS->info.version);
			//TODO: add parameter to select prerelease
			bool w=this->working.load(std::memory_order_acquire);
			if (w) ImGui::BeginDisabled();
				if (ImGui::Checkbox("Inclure les préversions",&(this->pPARAMETERS->info.check_update_prerelease))){
					this->checked.store(false,std::memory_order_relaxed);
				}
				if(ImGui::Button("Vérifier les mises à jour")){
					this->check();
				}
			if (w) ImGui::EndDisabled();
			if (this->checked.load(std::memory_order_acquire)){
				if (this->update.load(std::memory_order_acquire)){
					ImGui::SameLine();
					ImGui::Text("Une nouvelle version (%i.%i.%i.%i) est disponible!",
						this->major.load(std::memory_order_relaxed),
						this->minor.load(std::memory_order_relaxed),
						this->patch.load(std::memory_order_relaxed),
						this->tweak.load(std::memory_order_relaxed)
					);
				}
				else{
					ImGui::SameLine();
					ImGui::Text("L'émulateur est à jour!");
				}
			}
			ImGui::TextLinkOpenURL("Ouvrir la page des mises à jour",this->pPARAMETERS->info.releases);
		}
	private:
		Parameters* pPARAMETERS;
		std::thread thrd;
		std::atomic_int major=0;
		std::atomic_int minor=0;
		std::atomic_int patch=0;
		std::atomic_int tweak=0;
		std::atomic_bool update=false;
		std::atomic_bool checked=false;
		std::atomic_bool working=false;
		
		void check(){
			if (this->thrd.joinable()) this->thrd.join();
			this->working.store(true,std::memory_order_relaxed);
			this->thrd=std::thread(UpdateChecker::checkThread,this,this->pPARAMETERS->info.check_update_prerelease);
		}
		
		static void checkThread(UpdateChecker* puc, bool prerelease){
			int mj=0;
			int mn=0;
			int p=0;
			int t=0;
			getLatestVersionNumber(prerelease,&mj,&mn,&p,&t);
			bool u=true;
			if (mj==M12_VERSION_MAJOR){
				if (mn==M12_VERSION_MINOR){
					if (p==M12_VERSION_PATCH){
						if (t<=M12_VERSION_TWEAK) u=false;
					}
					else if (p<M12_VERSION_PATCH) u=false;
				}
				else if (mn<M12_VERSION_MINOR) u=false;
			}
			else if (mj<M12_VERSION_MAJOR) u=false;
			puc->major.store(mj,std::memory_order_relaxed);
			puc->minor.store(mn,std::memory_order_relaxed);
			puc->patch.store(p,std::memory_order_relaxed);
			puc->tweak.store(t,std::memory_order_relaxed);
			puc->update.store(u,std::memory_order_release);
			puc->checked.store(true,std::memory_order_release);
			puc->working.store(false,std::memory_order_release);
		}
};
#endif