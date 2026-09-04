#ifndef DBG_TS9347_H
#define DBG_TS9347_H
#include "circuit/TS9347.h"
class TS9347Logger{
	public:
		TS9347Logger(TS9347wVRAM* ic){
			this->ic=ic;
		}
		void update(unsigned char address, unsigned char data, bool RnW){
			if ((address&0xF0)!=0x20) return;
			if (this->isBusy){
				constexpr const char* reg_r[8]={"STATUS","R1","R2","R3","R4","R5","R6","R7"};
				constexpr const char* reg_w[8]={"CMD","R1","R2","R3","R4","R5","R6","R7"};
				if ((bool)(address&0x08)){
					printf("TS9347: Previous command aborted\n");
					if ((bool)(address&0x07)){
						if (RnW){
							printf("TS9347: Warning - Read probably garbage value for %s: %02X\n",reg_r[address&0x07],data);
						}
						else{
							printf("TS9347: Warning - Write probably ineffective for %s: %02X\n",reg_w[address&0x07],data);
						}
					}
					this->last_cmd.address=0;//force update
					this->_update(address,data,RnW);
				}
				else{
					if (RnW){
						if ((bool)(address&0x07)){
							printf("TS9347: Warning - Read probably garbage value for %s: %02X\n",reg_r[address&0x07],data);
						}
						else{
							this->_update(address,data,RnW);
						}
					}
					else{
						printf("TS9347: Warning - Write probably ineffective for %s: %02X\n",reg_w[address&0x07],data);
					}
				}
			}
			else{
				this->_update(address,data,RnW);
			}
		}
	private:
		struct {
			unsigned char address=0;
			unsigned char data;
			bool RnW;
		} last_cmd;
		
		bool isBusy=true;//do not rely on STATUS because check could be performed after BUSY is changed
		TS9347wVRAM* ic;
		static constexpr char R1[]="R1";
		static constexpr char R2[]="R2";
		static constexpr char R3[]="R3";
		static constexpr char A[]="A";
		static constexpr char B[]="B";
		static constexpr char C[]="C";
		static constexpr char D[]="D";
		static constexpr char W[]="W";
		static constexpr char As[]="A*";
		static constexpr char Bs[]="B*";
		const char* R123_value[3]={R1,R2,R3};
		
		unsigned char wX=0;
		unsigned char wXp=0;
		
		void _update(unsigned char address, unsigned char data, bool RnW){
			if (address!=this->last_cmd.address||data!=this->last_cmd.data||RnW!=this->last_cmd.RnW){
				this->last_cmd.address=address;
				this->last_cmd.data=data;
				this->last_cmd.RnW=RnW;
				
				if (RnW){
					if (!(bool)(address&0x07)) this->isBusy=(bool)(data&TS9347wVRAM::BUSY_MASK);
					this->updateRead(address,data);
				}
				else{
					if ((address&0x07)==0x05){
						this->wXp=data&0x3F;
					}
					if ((address&0x07)==0x07){
						this->wX=data&0x3F;
					}
					if ((address&0x07)==1) this->R123_value[0]=this->R1;
					if ((address&0x07)==2) this->R123_value[1]=this->R2;
					if ((address&0x07)==3) this->R123_value[2]=this->R3;
				}
				
				if ((bool)(address&0x08)){
					this->updateCmd();
				}
			}
		}
		
		void updateRead(unsigned char address, unsigned char data){
			constexpr unsigned char Bt[4]={0,2,1,3};
			switch (address&0x07){
				case 0:
					printf("TS9347: Read STATUS: Busy=%i Alarm=%i LXm=%i LXa=%i VSync=%i\n",(bool)(data&TS9347wVRAM::BUSY_MASK),(bool)(data&TS9347wVRAM::Al_MASK),(bool)(data&TS9347wVRAM::LXm_MASK),(bool)(data&TS9347wVRAM::LXa_MASK),(bool)(data&TS9347wVRAM::VSYNC_MASK));
					break;
				case 1:
				case 2:
				case 3:
					printf("TS9347: Read %s: 0x%02X\n",R123_value[(address&0x07)-1],data);
					break;
				case 4:
					printf("TS9347: Read AP: D'=%u Y'=%u\n",data>>5,data&0x1F);
					break;
				case 5:
					printf("TS9347: Read AP: B'=%u X'=%u\n",Bt[data>>6],data&0x3F);
					break;
				case 6:
					printf("TS9347: Read MP: D=%u Y=%u\n",data>>5,data&0x1F);
					break;
				case 7:
					printf("TS9347: Read MP: B=%u X=%u\n",Bt[data>>6],data&0x3F);
					break;
			}
		}
		
		void updateCmd(){
			constexpr unsigned char Bt[4]={0,2,1,3};
			constexpr const char* reg[8]={"ROM","TGS","MAT","PAT","DOR","-","-","ROR"};
			bool RnW=(bool)(this->ic->Rx[0].load(std::memory_order_relaxed)&0x08);
			char RnWc=RnW?'R':'W';
			bool inc=(bool)(this->ic->Rx[0].load(std::memory_order_relaxed)&0x01);
			bool MP=false;
			bool AP=false;
			switch (this->ic->Rx[0].load(std::memory_order_relaxed)&0xF0){
				
				case 0x00://TLM/TSM/CLL/CLS
					////////////////////////////
					switch (this->ic->Rx[0].load(std::memory_order_relaxed)&0x0F){
						case 0x00:
						case 0x01:
						case 0x08:
						case 0x09:
						case 0x0A:
						case 0x0B:
							printf("TS9347: TLM: %c increment=%i\n",RnWc,inc);
							MP=true;
							this->R123_value[0]=this->C;
							this->R123_value[1]=this->B;
							this->R123_value[2]=this->A;
							break;
						case 0x02:
						case 0x03:
							printf("TS9347: TSM:%c increment=%i\n",RnWc,inc);
							MP=true;
							this->R123_value[0]=this->As;
							this->R123_value[1]=this->Bs;
							this->R123_value[2]=this->R3;
							break;
						case 0x05:
							printf("TS9347: CLL\n");
							MP=true;
							this->R123_value[0]=this->C;
							this->R123_value[1]=this->B;
							this->R123_value[2]=this->A;
							break;
						case 0x07:
							printf("TS9347: CLS\n");
							MP=true;
							this->R123_value[0]=this->As;
							this->R123_value[1]=this->Bs;
							this->R123_value[2]=this->R3;
							break;
						default:
							printf("TS9347: Warning - Unknown command %02X\n",this->ic->Rx[0].load(std::memory_order_relaxed));
							this->R123_value[0]=this->R1;
							this->R123_value[1]=this->R2;
							this->R123_value[2]=this->R3;
							break;
					}
					break;
				case 0x20://TLA
					printf("TS9347: TLA: %c increment=%i\n",RnWc,inc);
					AP=true;
					this->R123_value[0]=this->C;
					this->R123_value[1]=this->B;
					this->R123_value[2]=this->A;
					break;
				case 0x30://TBM/TBA
					if (this->ic->Rx[0].load(std::memory_order_relaxed)&0x04){
						printf("TS9347: TBA: %c increment=%i\n",RnWc,inc);
						AP=true;
					}
					else{
						printf("TS9347: TBM: %c increment=%i\n",RnWc,inc);
						MP=true;
					}
					this->R123_value[0]=this->D;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0x40://KRS
					printf("TS9347: KRS: %c increment=%i\n",RnWc,inc);
					MP=true;
					this->R123_value[0]=this->C;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0x50://KRL
					printf("TS9347: KRL: %c increment=%i\n",RnWc,inc);
					MP=true;
					this->R123_value[0]=this->C;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->A;
					break;
				case 0x60://TSM/CLS
					if ((this->ic->Rx[0].load(std::memory_order_relaxed)&0x04)==0){
						printf("TS9347: TSM: %c increment=%i\n",RnWc,inc);
						MP=true;
						this->R123_value[0]=this->As;
						this->R123_value[1]=this->Bs;
						this->R123_value[2]=this->R3;
					}
					else if ((this->ic->Rx[0].load(std::memory_order_relaxed)&0x0D)==0x05){
						printf("TS9347: CLS\n");
						MP=true;
						this->R123_value[0]=this->As;
						this->R123_value[1]=this->Bs;
						this->R123_value[2]=this->R3;
					}
					else{
						printf("TS9347: Warning - Unknown command %02X\n",this->ic->Rx[0].load(std::memory_order_relaxed));
						this->R123_value[0]=this->R1;
						this->R123_value[1]=this->R2;
						this->R123_value[2]=this->R3;
					}
					break;
				case 0x70://TSA
					printf("TS9347: TSA: %c increment=%i\n",RnWc,inc);
					AP=true;
					this->R123_value[0]=this->As;
					this->R123_value[1]=this->Bs;
					this->R123_value[2]=this->R3;
					break;
				case 0x80://IND
					printf("TS9347: IND: %c reg=%s\n",RnWc,reg[this->ic->Rx[0].load(std::memory_order_relaxed)&0x07]);
					if ((this->ic->Rx[0].load(std::memory_order_relaxed)&0x07)==0&&!RnW){
						printf("TS9347:    : Warning - ROM is read only\n");
					}
					else if((this->ic->Rx[0].load(std::memory_order_relaxed)&0x07)==5||(this->ic->Rx[0].load(std::memory_order_relaxed)&0x07)==6){
						printf("TS9347:    : Warning - Unknown internal register: %u\n",this->ic->Rx[0].load(std::memory_order_relaxed)&0x07);
					}
					this->R123_value[0]=this->D;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0x90://NOP/VRM/VSM
					switch (this->ic->Rx[0].load(std::memory_order_relaxed)&0x0F){
						case 0x01:
							printf("TS9347: NOP\n");
							this->R123_value[0]=this->R1;
							this->R123_value[1]=this->R2;
							this->R123_value[2]=this->R3;
							break;
						case 0x05:
							printf("TS9347: VRM\n");
							this->R123_value[0]=this->R1;
							this->R123_value[1]=this->R2;
							this->R123_value[2]=this->R3;
							break;
						case 0x09:
							printf("TS9347: VSM\n");
							this->R123_value[0]=this->R1;
							this->R123_value[1]=this->R2;
							this->R123_value[2]=this->R3;
							break;
						default:
							printf("TS9347: Warning - Unknown command %02X\n",this->ic->Rx[0].load(std::memory_order_relaxed));
							this->R123_value[0]=this->R1;
							this->R123_value[1]=this->R2;
							this->R123_value[2]=this->R3;
							break;
					}
					break;
				case 0xB0://INY
					printf("TS9347: INY: Y=0x%02X\n",this->ic->Rx[6].load(std::memory_order_relaxed)&0x1F);
					this->R123_value[0]=this->R1;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0xD0://MVB
					printf("TS9347: MVB: %s stop=%i\n",RnW?"AP->MP":"MP->AP",inc);
					MP=true;
					AP=true;
					this->R123_value[0]=this->W;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0xE0://MVD
					printf("TS9347: MVD: %s stop=%i\n",RnW?"AP->MP":"MP->AP",inc);
					MP=true;
					AP=true;
					this->R123_value[0]=this->W;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				case 0xF0://MVT
					printf("TS9347: MVT: %s stop=%i\n",RnW?"AP->MP":"MP->AP",inc);
					MP=true;
					AP=true;
					this->R123_value[0]=this->W;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
				default:
					printf("TS9347: Warning - Unknown command %02X\n",this->ic->Rx[0].load(std::memory_order_relaxed));
					this->R123_value[0]=this->R1;
					this->R123_value[1]=this->R2;
					this->R123_value[2]=this->R3;
					break;
			}
			if (MP){
				if (this->wX>=40){
					printf("TS9347:    : Warning - X input was not in the acceptable range [0,39]: %u\n",this->wX);
				}
				printf("TS9347:    : MP:  D=%u  B=%u  Y=%u  X=%u\n",this->ic->Rx[6].load(std::memory_order_relaxed)>>5,Bt[this->ic->Rx[7].load(std::memory_order_relaxed)>>6],this->ic->Rx[6].load(std::memory_order_relaxed)&0x1F,this->ic->Rx[7].load(std::memory_order_relaxed)&0x3F);
			}
			if (AP){
				if (this->wXp>=40){
					printf("TS9347:    : Warning - X' input was not in the acceptable range [0,39]: %u\n",this->wXp);
				}
				printf("TS9347:    : AP: D'=%u B'=%u Y'=%u X'=%u\n",this->ic->Rx[4].load(std::memory_order_relaxed)>>5,Bt[this->ic->Rx[5].load(std::memory_order_relaxed)>>6],this->ic->Rx[4].load(std::memory_order_relaxed)&0x1F,this->ic->Rx[5].load(std::memory_order_relaxed)&0x3F);
			}
			if ((!RnW)&&((this->R123_value[0]!=this->R1&&this->R123_value[0]!=this->W)||this->R123_value[1]!=this->R2||this->R123_value[2]!=this->R3)){
				printf("TS9347:    :");
				if (this->R123_value[0]!=this->R1&&this->R123_value[0]!=this->W){
					printf(" %s=0x%02X",this->R123_value[0],this->ic->Rx[1].load(std::memory_order_relaxed));
				}
				if (this->R123_value[1]!=this->R2){
					printf(" %s=0x%02X",this->R123_value[1],this->ic->Rx[2].load(std::memory_order_relaxed));
				}
				if (this->R123_value[2]!=this->R3){
					printf(" %s=0x%02X",this->R123_value[2],this->ic->Rx[3].load(std::memory_order_relaxed));
				}
				printf("\n");
			}
		}
};
#endif