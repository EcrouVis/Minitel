#ifndef MEMORYWINDOW_H
#define MEMORYWINDOW_H

#include "Parameters.h"

char printable_ascii(char x){
	return (x<0x20)?0x2e:x;
}
const char* nibble_table="0123456789ABCDEF";

void memoryWindow(const char* w_title,imguiMemoryView* memView){
	std::atomic_uchar* p_mem=memView->mem;
	int m_len=memView->mem_size;
	std::atomic_uint* mem_op=memView->op;
	
	
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1,ImGui::GetTextLineHeightWithSpacing()*19),ImVec2(-1,FLT_MAX));
	ImGui::Begin(w_title,&(memView->show),0);
	
	int n_nibble=0;
	int a=m_len-1;
	while (a!=0){
		a=a>>4;
		n_nibble++;
	}
	
	char la[n_nibble+1];
	std::fill(la,la+n_nibble,0x20);
	la[n_nibble-1]=0x78;
	la[n_nibble]=0;
	
	char ld[19];
	ld[16]=0x20;//hack
	ld[17]=0x20;
	ld[18]=0;
	
	if (mem_op==NULL) memView->follow_address=false;
	else {
		ImGui::Checkbox("Suivre les opérations sur la mémoire:",&(memView->follow_address));
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0,1,0,1), "Lecture");
		ImGui::SameLine();
		ImGui::Text("/");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1,0,0,1), "Ecriture");
	}
		
	unsigned int addr=0;
	bool RW=false;
	if (mem_op!=NULL&&m_len!=0){
		addr=(*mem_op).load(std::memory_order_relaxed);
		RW=addr/m_len;
		addr%=m_len;
	}
	
	if (ImGui::BeginTable("memory",18,ImGuiTableFlags_ScrollY|ImGuiTableFlags_NoBordersInBody|ImGuiTableFlags_SizingFixedFit,ImVec2(0,ImGui::GetContentRegionAvail().y))){
        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn("A", ImGuiTableColumnFlags_NoHeaderLabel);
        ImGui::TableSetupColumn("x0", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x1", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x2", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x3", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x4", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x5", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x6", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x7", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x8", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("x9", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xA", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xB", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xC", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xD", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xE", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("xF", ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn("str", ImGuiTableColumnFlags_NoHeaderLabel);
		ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0,1,1,1));
        ImGui::TableHeadersRow();
		ImGui::PopStyleColor();
		
		if (memView->follow_address) ImGui::SetScrollFromPosY(ImGui::GetCursorStartPos().y+ImGui::GetTextLineHeightWithSpacing()*(addr>>4),0.5);
		
		ImGuiListClipper clipper;
		clipper.Begin(m_len/16+((m_len%16)!=0));
		while (clipper.Step()){
			for (int i=clipper.DisplayStart;i<clipper.DisplayEnd;i++){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				for (int k=0;k<=n_nibble-2;k++){
					la[n_nibble-2-k]=nibble_table[(i>>(4*k))&0x0F];
				}
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0,1,1,1));
				ImGui::Selectable(la,false,ImGuiSelectableFlags_None,ImVec2(0, 0));
				ImGui::PopStyleColor();
				//ImGui::TextColored(ImVec4(0,1,1,1),la);
				for (int j=0;j<16;j++){
					if (i*16+j<m_len){
						ImGui::TableSetColumnIndex(j+1);
						unsigned char c=p_mem[i*16+j].load(std::memory_order_relaxed);
						const char hx[3]={nibble_table[c>>4],nibble_table[c&0x0F],0};
						if (((unsigned int)i*16+j)==addr&&mem_op!=NULL)ImGui::TextColored(RW?ImVec4(1,0,0,1):ImVec4(0,1,0,1),hx);
						else ImGui::Text(hx);
						ld[j]=printable_ascii(c);//ascii_table[c];
					}
					else{
						ld[j]=0x20;
					}
				}
				ImGui::TableSetColumnIndex(17);
				ImGui::Text(ld);
			}
		}
		
		ImGui::EndTable();
	}
	ImGui::End();
}
void sfr80C32Window(imgui80C32SFRView* sfrView){
	
	ImGui::SetNextWindowSizeConstraints(ImVec2(-1,ImGui::GetTextLineHeightWithSpacing()*19),ImVec2(-1,FLT_MAX));
	ImGui::Begin("Registres spéciaux du 80C32",&(sfrView->show),0);
	
	char b[]=": 0x\0\0\0";
	unsigned char v_sfr;
	
	const char* n_sfr[]={"ACC","B","DPH","DPL","IE","IP","P0","P1","P2","P3","PCON","PSW","RCAP2H","RCAP2L","SBUF","SCON","SP","TCON","T2CON","TH0","TH1","TH2","TL0","TL1","TL2","TMOD"};
	std::atomic_uchar* p_sfr[]={sfrView->ACC,sfrView->B,sfrView->DPH,sfrView->DPL,sfrView->IE,sfrView->IP,sfrView->P0,sfrView->P1,sfrView->P2,sfrView->P3,sfrView->PCON,
								sfrView->PSW,sfrView->RCAP2H,sfrView->RCAP2L,sfrView->SBUF,sfrView->SCON,sfrView->SP,sfrView->TCON,sfrView->T2CON,sfrView->TH0,sfrView->TH1,
								sfrView->TH2,sfrView->TL0,sfrView->TL1,sfrView->TL2,sfrView->TMOD};
	for (unsigned int i=0;i<sizeof(n_sfr)/sizeof(*n_sfr);i++){
		ImGui::Text(n_sfr[i]);
		if (p_sfr[i]!=NULL){
			ImGui::SameLine();
			v_sfr=p_sfr[i]->load(std::memory_order_relaxed);
			b[4]=nibble_table[v_sfr>>4];
			b[5]=nibble_table[v_sfr&0x0F];
			ImGui::Text(b);
		}
	}
	ImGui::End();
}
#endif