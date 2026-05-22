#ifndef ENCODING_H
#define ENCODING_H
#include <vector>
#include <cctype>
#include <cstring>

char* url_encode(const char* cstr){
	std::vector<char> url;
	char* pstr=(char*)cstr;
	while (*pstr){
		if (isalnum(*pstr)||*pstr=='-'||*pstr=='_'||*pstr=='.'||*pstr=='~'){
			url.push_back(*pstr);
		}
		else{
			url.push_back('%');
			constexpr char T[]="0123456789ABCDEF";
			url.push_back(T[(*pstr)>>4]);
			url.push_back(T[(*pstr)&0x0F]);
		}
		pstr++;
	}
	url.push_back(0);
	char* urlstr=(char*)malloc(url.size()*sizeof(char));
	strcpy(urlstr,url.data());
	return urlstr;
}

char* videotex_to_utf8(std::vector<unsigned char>* vdt, bool G1=false){//SEP/REP/US/ESC/DLE sequences not supported
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

constexpr std::vector<unsigned char>* utf8_to_videotex_ts9347(const char* cstr, bool G1=false, bool plaintext=false){
	if (cstr==NULL) return NULL;
	unsigned char* pstr=(unsigned char*)cstr;
	std::vector<unsigned char>* vdt=new std::vector<unsigned char>;
	bool graphics=G1;
	while ((bool)pstr[0]){
		switch(pstr[0]){
			case 0x00 ... 0x7F:
				if (!plaintext||pstr[0]>=0x20){
					if (pstr[0]>0x20&&pstr[0]<0x7F&&graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(pstr[0]);
					pstr++;
				}
				break;
			case 0xC2 ... 0xDF:
				
				if (pstr[0]==0xC2&&pstr[1]==0xA3){//pound sign
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x23);
				}
				if (pstr[0]==0xC2&&pstr[1]==0xA7){//section sign
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x27);
				}
				if (pstr[0]==0xC2&&pstr[1]==0xB0){//degree sign
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x30);
				}
				if (pstr[0]==0xC2&&pstr[1]==0xB1){//plus/minus
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x31);
				}
				if (pstr[0]==0xC3&&pstr[1]==0xB7){//division sign
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x35);
				}
				if (pstr[0]==0xC3&&pstr[1]>=0xBC&&pstr[1]<=0xBE){//vulgar fraction
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(pstr[1]&0x7F);
				}
				if (pstr[0]==0xC5&&pstr[1]==0x92){//latin capital ligature oe
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x6A);
				}
				if (pstr[0]==0xC5&&pstr[1]==0x93){//latin small ligature oe
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x7A);
				}
				if (pstr[0]==0xCE&&pstr[1]==0xB2){//greek small letter beta
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x7B);
				}
				if (pstr[0]==0xC3&&pstr[1]==0xA7){//cedilla
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x4B);
					vdt->push_back(0x63);
				}
				if (pstr[0]==0xC3&&(pstr[1]==0xA0||pstr[1]==0xA2||pstr[1]==0xA4)){//a
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					switch (pstr[1]){
						case 0xA0:vdt->push_back(0x41);break;
						case 0xA2:vdt->push_back(0x43);break;
						case 0xA4:vdt->push_back(0x48);break;
					}
					vdt->push_back(0x61);
				}
				if (pstr[0]==0xC3&&(pstr[1]>=0xA8&&pstr[1]<=0xAB)){//e
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					switch (pstr[1]){
						case 0xA8:vdt->push_back(0x41);break;
						case 0xA9:vdt->push_back(0x42);break;
						case 0xAA:vdt->push_back(0x43);break;
						case 0xAB:vdt->push_back(0x48);break;
					}
					vdt->push_back(0x65);
				}
				if (pstr[0]==0xC3&&(pstr[1]==0xAE||pstr[1]==0xAF)){//i
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					switch (pstr[1]){
						case 0xAE:vdt->push_back(0x43);break;
						case 0xAF:vdt->push_back(0x48);break;
					}
					vdt->push_back(0x69);
				}
				if (pstr[0]==0xC3&&(pstr[1]==0xB4||pstr[1]==0xB6)){//o
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					switch (pstr[1]){
						case 0xB4:vdt->push_back(0x43);break;
						case 0xB6:vdt->push_back(0x48);break;
					}
					vdt->push_back(0x6F);
				}
				if (pstr[0]==0xC3&&(pstr[1]==0xB9||pstr[1]==0xBB||pstr[1]==0xBC)){//u
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					switch (pstr[1]){
						case 0xB9:vdt->push_back(0x41);break;
						case 0xBB:vdt->push_back(0x43);break;
						case 0xBC:vdt->push_back(0x48);break;
					}
					vdt->push_back(0x75);
				}
				pstr+=2;
				break;
			case 0xE0 ... 0xEF:
				if (pstr[0]==0xE2&&pstr[1]==0x96&&pstr[2]==0x88){//full block
					if (!graphics){
						vdt->push_back(0x0E);//SO
						graphics=true;
					}
					vdt->push_back(0x5F);
				}
				if (pstr[0]==0xE2&&pstr[1]==0x96&&pstr[2]==0x8C){//left half block
					if (!graphics){
						vdt->push_back(0x0E);//SO
						graphics=true;
					}
					vdt->push_back(0x35);
				}
				if (pstr[0]==0xE2&&pstr[1]==0x96&&pstr[2]==0x90){//right half block
					if (!graphics){
						vdt->push_back(0x0E);//SO
						graphics=true;
					}
					vdt->push_back(0x6A);
				}
				if (pstr[0]==0xE2&&pstr[1]==0x80&&pstr[2]==0x95){//horizontal bar
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x60);
				}
				if (pstr[0]==0xEF&&pstr[1]==0xBC&&pstr[2]==0x83){//number sign
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x26);
				}
				if (pstr[0]==0xE2&&pstr[1]==0x86&&pstr[2]>=0x90&&pstr[2]<=0x93){//arrow
					if (graphics){
						vdt->push_back(0x0F);//SI
						graphics=false;
					}
					vdt->push_back(0x19);//SS2
					vdt->push_back(0x2C+(pstr[2]&3));
				}
				pstr+=3;
				break;
			case 0xF0 ... 0xF4:
				if (pstr[0]==0xF0&&pstr[1]==0x9F&&pstr[2]==0xAC&&pstr[3]>=0x80&&pstr[3]<=0xBB){//sextant
					if (!graphics){
						vdt->push_back(0x0E);//SO
						graphics=true;
					}
					{
						unsigned char c=pstr[3]&0x7F;
						if (c<0x14) c+=0x21;
						else if (c<0x1E) c+=0x22;
						else if (c<0x28) c+=0x42;
						else c+=0x43;
						vdt->push_back(c);
					}
					
				}
				pstr+=4;
				break;
			default:
				delete vdt;
				return NULL;
		}
	}
	if (graphics&&!G1) vdt->push_back(0x0F);//SI
	if (G1&&!graphics) vdt->push_back(0x0E);//SI
	return vdt;
}

class cmdSplitterVideotex{
	public:
		cmdSplitterVideotex(){
			this->cmdnestq.push_back(new std::vector<unsigned char>);
		}
		void updateCmd(unsigned char d){
			resync:
			
			if (d==0x1B||d==0x00) this->cmdnestq.push_back(new std::vector<unsigned char>);
			std::vector<unsigned char>* pcmd=cmdnestq.back();//avoid issues with sequences nesting
			pcmd->push_back(d);
			switch (pcmd->front()){
				case 0x00://NUL
				{
					this->callback(pcmd);
					this->cmdnestq.pop_back();
					delete pcmd;
					break;
				}
				case 0x10://DLE
					if (pcmd->size()==2){
						this->callback(pcmd);
						pcmd->clear();
					}
					break;
				case 0x12://REP
					if (pcmd->size()==2){
						if (pcmd->back()<0x20){
							pcmd->clear();
							goto resync;
						}
						else{
							if (pcmd->back()>=0x40) this->callback(pcmd);
							pcmd->clear();
						}
					}
					break;
				case 0x13://SEP
					if (pcmd->size()==2){
						this->callback(pcmd);
						pcmd->clear();
					}
					break;
				case 0x19://SS2
					if (pcmd->size()>=2&&pcmd->back()<0x20){
						pcmd->clear();
						goto resync;
					}
					else{
						if ((*pcmd)[1]>=0x40) this->callback(pcmd);
						pcmd->clear();
					}
					break;
				case 0x1D://SS3
					if (pcmd->size()==2){
						if ((*pcmd)[1]<0x20){
							pcmd->clear();
							goto resync;
						}
						else{
							this->callback(pcmd);
							pcmd->clear();
						}
					}
					break;
				case 0x1F://US
					//STUTEL (ETS 300 075) -> 0x15 0x3E ...
					//STUCAM -> 0x15 0x3C ...
					if (pcmd->size()==2){
						if (pcmd->back()<0x20){
							pcmd->clear();
							goto resync;
						}
						else{
							this->callback(pcmd);
							pcmd->clear();
						}
					}
					break;
				//case 0x1A://SUB
				case 0x1B://ESC
					if (pcmd->size()>=2){
						if (pcmd->back()<0x20||pcmd->back()==0x7F){
							this->cmdnestq.pop_back();
							delete pcmd;
							goto resync;
						}
						else{
							if((*pcmd)[1]<0x30){//nF escape sequence
								if (pcmd->back()>=0x30){
									this->callback(pcmd);
									this->cmdnestq.pop_back();
									delete pcmd;
								}
							}
							else{//Fp/Fe/Fs escape sequence
								this->callback(pcmd);
								this->cmdnestq.pop_back();
								delete pcmd;
							}
						}
					}
					break;
				default:
					this->callback(pcmd);
					pcmd->clear();
					break;
			}
		}
		void setCmdCallback(std::function<void(std::vector<unsigned char>*)> f){
			this->callback=f;
		}
		
	private:
		std::function<void(std::vector<unsigned char>*)> callback;
		std::vector<std::vector<unsigned char>*> cmdnestq;
};

#endif