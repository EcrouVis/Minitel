#ifndef ENCODING_H
#define ENCODING_H
#include <vector>
#include <cctype>
#include <cstring>
#include <functional>

char* videotex_to_utf8(std::vector<unsigned char>* vdt, bool G1=false);

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
class VideotexSplitter{//TODO: does not support minitel network sequence nesting
	public:
		bool sequenceEnded=false;
		std::vector<unsigned char> sequence;
		
		bool updateSequence(unsigned char d){
			if (this->sequenceEnded){
				resync:
				this->sequenceEnded=false;
				this->sequence.clear();
			}
			
			this->sequence.push_back(d);
			
			switch (this->sequence.front()){
				default:
					this->sequenceEnded=true;
					break;
					
				case 0x00://NUL
					this->sequenceEnded=false;
					break;
					
				case 0x12://REP
					if (this->sequence.size()==2){
						if (this->sequence.back()<0x40){
							goto resync;
						}
						else{
							this->sequenceEnded=true;
						}
					}
					break;
					
				case 0x13://SEP
					if (this->sequence.size()==2){
						this->sequenceEnded=true;
					}
					break;
					
				case 0x16://SYN
				case 0x19://SS2
					if (this->sequence.size()>=2&&this->sequence.back()<0x20){
						goto resync;
					}
					else{
						if ((this->sequence[1]&0x70)!=0x40||this->sequence.size()==3) this->sequenceEnded=true;
					}
					break;
					
				case 0x1D://SS3
					if (this->sequence.size()==2){
						if (this->sequence.back()<0x20){
							goto resync;
						}
						else{
							this->sequenceEnded=true;
						}
					}
					break;
					
				case 0x1F://US / TODO: support sequences other than US XX+64 YY+64
					if (this->sequence.size()>=2){
						if (this->sequence.back()<0x40||this->sequence.back()>0x58){
							goto resync;
						}
						else{
							if (this->sequence.size()==3){
								this->sequenceEnded=true;
							}
						}
					}
					break;
					
				case 0x1B://ESC
					if (this->sequence.size()>=2){
						if (this->sequence.back()<0x20||this->sequence.back()==0x7F){
							goto resync;
						}
						else if(this->sequence[1]<0x30){//nF escape sequence
							if (this->sequence.back()>=0x30){
								this->sequenceEnded=true;
							}
						}
						else if (this->sequence[1]<0x40){//Fp escape sequence
							switch(this->sequence[1]){
								case 0x35 ... 0x37:
									if (this->sequence.size()==3){
										if ((this->sequence[2]&0x70)==0x40){
											this->sequenceEnded=true;
										}
										else{
											goto resync;
										}
									}
									break;
								case 0x39:
									if (this->sequence.size()==3){
										this->sequenceEnded=true;
									}
									break;
								case 0x3A:
									if (this->sequence.size()==4){
										this->sequenceEnded=true;
									}
									break;
								case 0x3B:
									if (this->sequence.size()==5){
										this->sequenceEnded=true;
									}
									break;
								default:
									goto resync;
							}
						}
						else if (this->sequence[1]<0x60){//Fe escape sequence
							if (this->sequence[1]==0x5B){//CSI
								if(this->sequence.size()>=3){
									switch(this->sequence.back()){
										case 0x30 ... 0x3F:
											if((this->sequence[this->sequence.size()-2]<0x30||this->sequence[this->sequence.size()-2]>0x3F)&&this->sequence.size()!=3){
												goto resync;
											}
											break;
										case 0x20 ... 0x2F:
											if((this->sequence[this->sequence.size()-2]<0x20||this->sequence[this->sequence.size()-2]>0x3F)&&this->sequence.size()!=3){
												goto resync;
											}
											break;
										case 0x40 ... 0x7E:
											this->sequenceEnded=true;
											break;
										default:
											goto resync;
									}
								}
							}
							else{//C1
								this->sequenceEnded=true;
							}
						}
						else{//Fs escape sequence
							//if (this->sequence.size()==2)
							this->sequenceEnded=true;
						}
					}
					break;
			}
			
			return this->sequenceEnded;
		}
};

std::vector<unsigned char>* DProtocolTranslationMode4Encode(const std::vector<unsigned char>* data,bool C0=false,bool space=false);

std::vector<unsigned char>* DProtocolTranslationMode4Decode(const std::vector<unsigned char>* edata);

char* Base64Encode(const unsigned char* data, size_t s);

unsigned char* Base64Decode(const char* edata, size_t* ps);

#endif