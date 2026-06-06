#include "encoding.h"
#include <cstdlib>

char* videotex_to_utf8(std::vector<unsigned char>* vdt, bool G1){//SEP/REP/US/ESC/DLE sequences not supported
	if (vdt==NULL) return NULL;
	bool graphics=G1;
	std::vector<char> str;
	size_t i=0;
	while (i<vdt->size()){
		if ((*vdt)[i]<0x20){
			switch ((*vdt)[i]){
				case 0x0E:
					graphics=true;
					break;
				case 0x0F:
					graphics=false;
					break;
				case 0x19:
					break;
				case 0x10:
				case 0x12:
				case 0x13:
				case 0x1B:
				case 0x1F:
					return NULL;
				default:
					str.push_back((*vdt)[i]);
					break;
			}
		}
		else{
			if (graphics){
				if ((*vdt)[i]==0x20||(*vdt)[i]==0x7F){
					str.push_back((*vdt)[i]);
				}
				else{
					switch ((*vdt)[i]){
						case 0x35: str.push_back(0xE2); str.push_back(0x96); str.push_back(0x8C); break;
						case 0x4A:
						case 0x6A: str.push_back(0xE2); str.push_back(0x96); str.push_back(0x90); break;
						case 0x5F: str.push_back(0xE2); str.push_back(0x96); str.push_back(0x88); break;
						case 0x21 ... 0x34: str.push_back(0xF0); str.push_back(0x9F); str.push_back(0xAC); str.push_back(0x80-0x21+(*vdt)[i]); break;
						case 0x36 ... 0x49: str.push_back(0xF0); str.push_back(0x9F); str.push_back(0xAC); str.push_back(0x80-0x22+(*vdt)[i]); break;
						case 0x4B ... 0x5E: str.push_back(0xF0); str.push_back(0x9F); str.push_back(0xAC); str.push_back(0x80-0x23+(*vdt)[i]); break;
						case 0x60 ... 0x69: str.push_back(0xF0); str.push_back(0x9F); str.push_back(0xAC); str.push_back(0x80-0x42+(*vdt)[i]); break;
						case 0x6B ... 0x6E: str.push_back(0xF0); str.push_back(0x9F); str.push_back(0xAC); str.push_back(0x80-0x43+(*vdt)[i]); break;
					}
				}
			}
			else{
				if (i>=1&&(*vdt)[i-1]==0x19){
					if (((*vdt)[i]&0xF0)!=0x40){
						switch ((*vdt)[i]){
							case 0x23: str.push_back(0xC2); str.push_back(0xA3); break;
							case 0x24: str.push_back(0x24); break;
							case 0x26: str.push_back(0xEF); str.push_back(0xBC); str.push_back(0x83); break;
							case 0x27: str.push_back(0xC2); str.push_back(0xA7); break;
							case 0x2C:
							case 0x2D:
							case 0x2E:
							case 0x2F: str.push_back(0xE2); str.push_back(0x86); str.push_back(0x90+((*vdt)[i]&3)); break;
							case 0x30: str.push_back(0xC2); str.push_back(0xB0); break;
							case 0x31: str.push_back(0xC2); str.push_back(0xB1); break;
							case 0x35: str.push_back(0xC3); str.push_back(0xB7); break;
							case 0x3C:
							case 0x3D:
							case 0x3E: str.push_back(0xC2); str.push_back((*vdt)[i]+0x80); break;
							case 0x6A: str.push_back(0xC5); str.push_back(0x92); break;
							case 0x7A: str.push_back(0xC5); str.push_back(0x93); break;
							case 0x7B: str.push_back(0xCE); str.push_back(0xB2); break;
							default: str.push_back(0x5F); break;
						}
					}
				}
				else if (i>=2&&(*vdt)[i-2]==0x19&&((*vdt)[i-1]&0xF0)==0x40){
					switch ((*vdt)[i-1]){
						case 0x41://àèù
							switch ((*vdt)[i]){
								case 0x61: str.push_back(0xC3); str.push_back(0xA0); break;
								case 0x65: str.push_back(0xC3); str.push_back(0xA8); break;
								case 0x75: str.push_back(0xC3); str.push_back(0xB9); break;
								default: str.push_back((*vdt)[i]); break;
							}
							break;
						case 0x42://é
							if ((*vdt)[i]==0x65){
								str.push_back(0xC3);
								str.push_back(0xA9);
							}
							else str.push_back((*vdt)[i]);
							break;
						case 0x43://âêîôû
							switch ((*vdt)[i]){
								case 0x61: str.push_back(0xC3); str.push_back(0xA2); break;
								case 0x65: str.push_back(0xC3); str.push_back(0xAA); break;
								case 0x69: str.push_back(0xC3); str.push_back(0xAE); break;
								case 0x6F: str.push_back(0xC3); str.push_back(0xB4); break;
								case 0x75: str.push_back(0xC3); str.push_back(0xBB); break;
								default: str.push_back((*vdt)[i]); break;
							}
							break;
						case 0x48://äëïöü
							switch ((*vdt)[i]){
								case 0x61: str.push_back(0xC3); str.push_back(0xA4); break;
								case 0x65: str.push_back(0xC3); str.push_back(0xAB); break;
								case 0x69: str.push_back(0xC3); str.push_back(0xAF); break;
								case 0x6F: str.push_back(0xC3); str.push_back(0xB6); break;
								case 0x75: str.push_back(0xC3); str.push_back(0xBC); break;
								default: str.push_back((*vdt)[i]); break;
							}
							break;
						case 0x4B://ç
							if ((*vdt)[i]==0x63){
								str.push_back(0xC3);
								str.push_back(0xA7);
							}
							else str.push_back((*vdt)[i]);
							break;
						default: str.push_back((*vdt)[i]); break;
					}
				}
				else{
					str.push_back((*vdt)[i]);
				}
			}
		}
		i++;
	}
	char* cstr=(char*)malloc(str.size()*sizeof(char)+1);
	memcpy(cstr,str.data(),str.size()*sizeof(char));
	cstr[str.size()]=0;
	return cstr;
}