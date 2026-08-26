#ifndef NOTIFICATIONSERVER_H
#define NOTIFICATIONSERVER_H
#include "imgui.h"

struct imgui_notification{
	char* message;
	ImVec4 color;
	double timestamp;
	imgui_notification* older;
} ;
class NotificationServer{
	public:
		~NotificationServer(){
			imgui_notification* tmp=this->notification_list;
			imgui_notification* tmp2=NULL;
			while (tmp!=NULL){
				tmp2=tmp;
				tmp=tmp->older;
				free((void*)tmp2->message);
				delete tmp2;
			}
		}
		void notify(const char* message,ImVec4 color=ImVec4(1.,1.,1.,1.)){
			imgui_notification* notif=new imgui_notification;
			notif->message=(char*)malloc(strlen(message)+1);
			strcpy(notif->message,message);
			notif->color=color;
			notif->timestamp=glfwGetTime();
			notif->older=this->notification_list;
			this->notification_list=notif;
		}
		void notification_window(double duration=5,double duration_fading=1,int max_notification=-1){
			if (delete_old_notifications(duration,max_notification)){
				ImGui::SetNextWindowPos(ImVec2(10,10));
				ImGui::Begin("Notifications",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoBackground);
				imgui_notification* tmp=this->notification_list;
				double t=glfwGetTime()-duration;
				while (tmp!=NULL){
					double t_f=(tmp->timestamp-t)/duration_fading;
					if (t_f>0&&t_f<1){
						ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(tmp->color.x,tmp->color.y,tmp->color.z,tmp->color.w*t_f));
					}
					else{
						ImGui::PushStyleColor(ImGuiCol_Text,tmp->color);
					}
					ImGui::Text("%s",tmp->message);
					ImGui::PopStyleColor();
					tmp=tmp->older;
				}
				ImGui::End();
			}	
		}
	private:
		imgui_notification* notification_list=NULL;
		
		bool delete_old_notifications(double duration=5,int max_notification=5){
			double t=glfwGetTime()-duration;
			imgui_notification* tmp=this->notification_list;
			bool show=false;
			int n=1;
			imgui_notification* tmp2=NULL;
			while (tmp!=NULL&&tmp->timestamp>=t&&(n<max_notification||max_notification<0)){
				show=true;
				tmp2=tmp;
				tmp=tmp->older;
				n++;
			}
			if (tmp!=NULL){
				if (tmp2==NULL) this->notification_list=NULL;
				else tmp2->older=NULL;
				while (tmp!=NULL){
					tmp2=tmp;
					tmp=tmp->older;
					free((void*)tmp2->message);
					delete tmp2;
				}
			}
			return show;
		}
};
#endif