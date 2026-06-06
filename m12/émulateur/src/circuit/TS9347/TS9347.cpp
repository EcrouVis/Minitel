#include "circuit/TS9347.h"

//IO

void TS9347wVRAM::DChangeIn(unsigned char d){this->D=d;}

void TS9347wVRAM::nCSChangeIn(bool b){this->nCS=b;}

void TS9347wVRAM::ASChangeIn(bool b){
	if (this->AS&&!b){
		this->ADDR_BUF.store(this->D|(this->nCS?0x100:0),std::memory_order_relaxed);
	}
	this->AS=b;
}

void TS9347wVRAM::DSChangeIn(bool b){
			if (this->DS&&!b){
				this->DS=b;
				//read reg
				if (this->isICSelected()){
					unsigned int addr=this->ADDR_BUF.load(std::memory_order_relaxed);
					if ((addr&0x07)==0) this->sendD(this->STATUS.load(std::memory_order_relaxed));
					else if(!this->isBusy()) this->sendD(this->Rx[addr&0x07].load(std::memory_order_relaxed));
					else printf("TS9347 read garbage!!!\n");
				}
			}
			else{
				this->DS=b;
			}
		}

void TS9347wVRAM::RnWChangeIn(bool b){
	if ((!this->RnW)&&b){
		this->RnW=b;
		//write reg
		unsigned int addr=this->ADDR_BUF.load(std::memory_order_relaxed);
		if (this->isICSelected()){
			if ((!this->isBusy())||(this->requestExecution()&&(addr&0x07)==0)){
				this->Rx[addr&0x07].store(this->D,std::memory_order_relaxed);
			}
			else printf("TS9347 write inefective!!!\n");
			if (this->requestExecution()){
				this->STATUS.fetch_or(this->BUSY_MASK,std::memory_order_relaxed);
				///execute command
				this->cmd_step=0;
			}
		}
	}
	else{
		this->RnW=b;
	}
}

void TS9347wVRAM::subscribeD(std::function<void(unsigned char)> f){this->sendD=f;}

void TS9347wVRAM::subscribeVideo(std::function<void(unsigned char*)> f){this->sendVideo=f;}

//Charset ROM loading

void TS9347wVRAM::setROMCharset(const unsigned char* cs){//the slice should be inverted
	for (int i=0;i<ROM_CHARSET_SIZE;i++){
		this->ROM_CHARSET[i].store(cs[i],std::memory_order_release);
	}
}

//Display

unsigned char TS9347wVRAM::getMarginABGR(){
	return (this->MAT.load(std::memory_order_relaxed)&0x0F)|((this->PAT.load(std::memory_order_relaxed)&0x20)<<2);
}

int TS9347wVRAM::getVideoIndex(const unsigned int line, const unsigned char column, bool mode40){
	return (line+1)*(40*8*3+2)+1+column*((mode40)?8*3:6*2);
}

void TS9347wVRAM::setVideoOtputABGR(int index,unsigned char abgr){
	if ((bool)(index&1)){
		this->VIDEO_OUTPUT[index>>1]=(this->VIDEO_OUTPUT[index>>1]&0xF0)|(abgr&0x0F);
	}
	else{
		this->VIDEO_OUTPUT[index>>1]=(this->VIDEO_OUTPUT[index>>1]&0x0F)|(abgr<<4);
	}
}

int TS9347wVRAM::getSliceRAMIndex(unsigned char slice, unsigned char C, unsigned char Z){
	int A=(Z&0x1E)<<10;
	
	if ((bool)(C&0x60)){
		A|=(Z&1)<<10;
		A|=(C&0x1C)<<3;
		if ((bool)(slice&0x08)){
			A|=(C&0x60)>>2;
		}
		else{
			A|=(C&0x60)<<3;
			A|=(slice&0x06)<<2;
		}
	}
	else{
		if ((bool)(C&0x04)){
			if ((bool)(Z&1)){
				A|=0x0400;
			}
			else{
				A|=(slice&0x02)<<9;
			}
			A|=0x0080;
			A|=0x0060^((slice&0x0C)<<3);
		}
		else{
			A|=(Z&1)<<10;
			A|=(slice&0x0E)<<4;
		}
	}
	
	A|=C&0x03;
	A|=(slice&1)<<2;
	
	return A;
}

unsigned char TS9347wVRAM::getYFromRow(unsigned char row){
	row+=(this->TGS.load(std::memory_order_relaxed)&1)-1;//service row -> 24 or 255 (overflow)
	if (row>=24) row=0;
	else{
		if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x80)) row=row>>1;//double height
		row+=this->ROR.load(std::memory_order_relaxed)&0x1F;//TODO: search behavior of YOR
		row=(row>31)?row-24:row;
	}
	return row;
}

unsigned char TS9347wVRAM::getDisplayDistrict(){
	return ((this->DOR.load(std::memory_order_relaxed)&0x80)>>5)|
		   ((this->ROR.load(std::memory_order_relaxed)&0x80)>>6)|
		   ((this->ROR.load(std::memory_order_relaxed)&0x20)>>5);
}

void TS9347wVRAM::loadRowBuffer(){
	
	//calculate coords
	unsigned char column=(this->clk_frame>>1)&0x3F;
	if (((this->clk_frame>>7)%10)==1){
		if (column>=40)column+=column-40+(this->clk_frame&1);
	}
	else column+=88;
	unsigned char block=column%3;
	column=column/3;
	
	if (column<40){
		//unsigned char row=((this->clk_frame+64*2-(62<<7))/(640*2))%25;
		unsigned char row=((this->clk_frame>>7)-61)/10;
		/*row+=(this->TGS.load(std::memory_order_relaxed)&1);//service row position
		row=(row)%25;
		if (row!=0){
			row=(row+(this->ROR.load(std::memory_order_relaxed)&0x1F)-8)%25;
			row+=7;
			if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x80)) row=(row+1)>>1;//double height
		}*/
		/*row+=(this->TGS.load(std::memory_order_relaxed)&1)-1;//service row -> 24 or 255 (overflow)
		if (row>=24) row=0;
		else{
			if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x80)) row=row>>1;//double height
			row+=this->ROR.load(std::memory_order_relaxed)&0x1F;//TODO: search behavior of YOR
			row=(row>31)?row-24:row;
		}*/
		row=this->getYFromRow(row);
		//calculate ram address
		int A=(this->DOR.load(std::memory_order_relaxed)&0x80)<<7;//Z
		A|=(this->ROR.load(std::memory_order_relaxed)&0x80)<<6;
		A|=(this->ROR.load(std::memory_order_relaxed)&0x20)<<7;
		A|=((block+((this->ROR.load(std::memory_order_relaxed)&0x40)>>5))&0x03)<<10;
		if (row==0){
			A|=((column&0x38)<<2)|(column&0x07);
		}
		else if ((bool)(column&0x20)){
			A|=((row&0x07)<<5)|(row&0x18)|(column&0x07);
		}
		else{
			A|=((row&0x1F)<<5)|(column&0x1F);
		}
		
		//store all data as long code
		switch (this->TGS.load(std::memory_order_relaxed)>>6){
			case 0://40 char long
				switch (block){
					case 0:
						this->ROW_BUFFER[column*3].store((this->VRAM[A].load(std::memory_order_relaxed)&0x7F)|(this->ROW_BUFFER[column*3].load(std::memory_order_relaxed)&0x80),std::memory_order_relaxed);
						break;
					case 1:
						this->ROW_BUFFER[column*3+1].store(this->VRAM[A].load(std::memory_order_relaxed),std::memory_order_relaxed);
						if ((bool)(this->VRAM[A].load(std::memory_order_relaxed)&0x02))this->ROW_BUFFER[column*3].fetch_xor(0x80,std::memory_order_relaxed);//C7=0 when double height first half / C7=1 when double height second half
						else this->ROW_BUFFER[column*3].fetch_or(0x80,std::memory_order_relaxed);//C7=1 when no 
						break;
					case 2:
						this->ROW_BUFFER[column*3+2].store(this->VRAM[A].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
				}
				break;
			case 1://40 char short
				switch (block){
					case 0:
					{
						unsigned char as=this->VRAM[A].load(std::memory_order_relaxed);
						
						unsigned char a=(as&0x08)|((as&0x07)<<4);
						unsigned char b=0;
						if (column!=0){
							a|=this->ROW_BUFFER[column*3-1].load(std::memory_order_relaxed)&0x07;
							b=this->ROW_BUFFER[column*3-2].load(std::memory_order_relaxed)&0b01010101;
						}
						
						if ((bool)(as&0x80)){
							a&=0xF8;
							a|=(as&0x70)>>4;
							
							b|=0x20;
							b&=0xEF;//U=0
							
							this->ROW_BUFFER[column*3].fetch_or(0x80,std::memory_order_relaxed);//no double height
						}
						else{
							a|=(as&0x40)<<1;//N
							b|=(as&0x20)>>2;//L
							b|=(as&0x10)>>3;//H
							
							if ((bool)(as&0x10))this->ROW_BUFFER[column*3].fetch_xor(0x80,std::memory_order_relaxed);//C7=0 when double height first half / C7=1 when double height second half
							else this->ROW_BUFFER[column*3].fetch_or(0x80,std::memory_order_relaxed);//C7=1 when no 
						}
						this->ROW_BUFFER[column*3+1].store(b,std::memory_order_relaxed);
						this->ROW_BUFFER[column*3+2].store(a,std::memory_order_relaxed);
						break;
					}
					case 1:
					{
						unsigned char bs=this->VRAM[A].load(std::memory_order_relaxed);
						
						unsigned char a=this->ROW_BUFFER[column*3+2].load(std::memory_order_relaxed);
						unsigned char b=this->ROW_BUFFER[column*3+1].load(std::memory_order_relaxed);
						unsigned char c=this->ROW_BUFFER[column*3].load(std::memory_order_relaxed)&0x80;//double height position
						
						switch (bs>>5){
							case 0x00:
							case 0x01:
							case 0x02:
							case 0x03:
								c|=bs&0x7F;
								break;
							case 0x04:
								a|=0x80;
								a&=0xF7;
								
								b&=0b10101010;
								b|=(bs&0x08)<<3;//i2
								b|=(bs&0x04)<<2;//U
								b|=(bs&0x02)>>1;//i1
								b|=(bs&0x01)<<2;//m
								break;
							case 0x05:
							case 0x06:
							case 0x07:
								c|=bs&0x7F;
								b|=0x80;
								break;
						}
						
						this->ROW_BUFFER[column*3].store(c,std::memory_order_relaxed);
						this->ROW_BUFFER[column*3+1].store(b,std::memory_order_relaxed);
						this->ROW_BUFFER[column*3+2].store(a,std::memory_order_relaxed);
						break;
					}
				}
				break;
			case 2://80 char short
				if (block==2) this->ROW_BUFFER[column*3+2].store(0,std::memory_order_relaxed);
				else this->ROW_BUFFER[column*3+block].store(this->VRAM[A].load(std::memory_order_relaxed),std::memory_order_relaxed);
				break;
			case 3://80 char long
				this->ROW_BUFFER[column*3+block].store(this->VRAM[A].load(std::memory_order_relaxed),std::memory_order_relaxed);
				break;
		}
		
	}
	
	
}

void TS9347wVRAM::loadUDS(){//load UDS + video output
	unsigned char column=(this->clk_frame>>1)&0x3F;
	unsigned int line=(this->clk_frame>>7)-62;
	unsigned char slice=line;
	/*unsigned char slice=line%10;
	if (((line/10)+(this->TGS.load(std::memory_order_relaxed)&1))%25!=0){
		if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x80)) slice=((line>>1)+5-(this->TGS.load(std::memory_order_relaxed)&1)*5)%10;//double height
	}*/
	if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x80)){//double height
		slice=line>>1;
		slice+=((bool)(this->TGS.load(std::memory_order_relaxed)&1))?0:5;//service row low
	}
	slice=slice%10;
	
	
	//margin
	if (column==0){
		//unsigned char c=this->getMarginColor();
		unsigned char c=this->getMarginABGR();
		//this->VIDEO_OUTPUT[(line+1)*(40*8*3+2)].store(c,std::memory_order_relaxed);
		this->setVideoOtputABGR((line+1)*(40*8*3+2),c);
		if (line==0){
			for (int i=0;i<(40*8*3+2);i++){
				//this->VIDEO_OUTPUT[i].store(c,std::memory_order_relaxed);
				this->setVideoOtputABGR(i,c);
			}
		}
	}
	else if (column==39){
		//unsigned char c=this->getMarginColor();
		unsigned char c=this->getMarginABGR();
		//this->VIDEO_OUTPUT[(line+2)*(40*8*3+2)-1].store(c,std::memory_order_relaxed);
		this->setVideoOtputABGR((line+2)*(40*8*3+2)-1,c);
		if (line==249){
			for (int i=0;i<(40*8*3+2);i++){
				//this->VIDEO_OUTPUT[251*(40*8*3+2)+i].store(c,std::memory_order_relaxed);
				this->setVideoOtputABGR(251*(40*8*3+2)+i,c);
			}
		}
	}
	//margin extension
	bool margin_ext;
	if (((bool)(this->TGS.load(std::memory_order_relaxed)&1))?(line>=240):(line<10)) margin_ext=!(bool)(this->PAT.load(std::memory_order_relaxed)&1);
	else margin_ext=!(bool)(this->PAT.load(std::memory_order_relaxed)&2);
	if (margin_ext){
		unsigned char c=this->getMarginABGR();
		int j=getVideoIndex(line,column);
		for (int i=0;i<24;i++){
			this->setVideoOtputABGR(j+i,c);
		}
		return;
	}
	//output slice
	if ((bool)(this->TGS.load(std::memory_order_relaxed)&0x80)){//80 char long/short
		//slice->6 first bits of 40 char mode
		for (int i=0;i<2;i++){
			unsigned char c=this->ROW_BUFFER[3*column+i].load(std::memory_order_relaxed);
			switch (c>>5){
				case 0:
				case 1:
				case 2:
				case 3:
					c=this->ROM_CHARSET[c*10+slice].load(std::memory_order_relaxed);
					break;
				case 4:
					c=this->ROM_CHARSET[c*10+1280+slice].load(std::memory_order_relaxed);
					break;
				default:
					c=0;
					break;
			}
			unsigned char a=this->ROW_BUFFER[3*column+2].load(std::memory_order_relaxed)<<(i<<2);//NFUDXXXX
			//load insert+foreground color
			bool insert;
			unsigned char mc;
			if ((bool)(a&0x10)){
				insert=(bool)(this->DOR.load(std::memory_order_relaxed)&0x80);
				//mc=(this->DOR.load(std::memory_order_relaxed)&0x70)<<1;
				mc=(this->DOR.load(std::memory_order_relaxed)&0x70)>>4;
			}
			else{
				insert=(bool)(this->DOR.load(std::memory_order_relaxed)&0x08);
				//mc=this->DOR.load(std::memory_order_relaxed)<<5;
				mc=this->DOR.load(std::memory_order_relaxed)&0x07;
			}
			//load cursor attributes
			bool comp=false;
			if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x40)){//cursor enabled
				if (this->getX(true)==column&&((this->getB(true)&1)==i)&&this->getY(true)==this->getYFromRow(line/10)&&this->getD(true)==this->getDisplayDistrict()){//on the cursor
					if (this->n_frame<25||(!(bool)(this->MAT.load(std::memory_order_relaxed)&0x20))){
						a^=(this->MAT.load(std::memory_order_relaxed)&0x10)<<1;//U
						comp=!(bool)(this->MAT.load(std::memory_order_relaxed)&0x10);
					}
				}
			}
			//U
			if (((bool)(a&0x20))&&slice==9) c=0xFF;
			//F
			if (((bool)(a&0x40))&&((bool)(this->PAT.load(std::memory_order_relaxed)&0x40))){
				if (((bool)(a&0x80))&&this->n_frame<25) c=0;
				else if ((!(bool)(a&0x80))&&this->n_frame>=25) c=0;
			}
			//color selection+N
			//unsigned char sc=this->MAT.load(std::memory_order_relaxed)<<5;
			unsigned char sc=this->MAT.load(std::memory_order_relaxed)&0x07;
			if ((bool)(a&0x80)) c=~c;
			//color complementation
			if (comp){
				//mc=mc^0xE0;
				mc=mc^0x07;
				//sc=sc^0xE0;
				sc=sc^0x07;
			}
			//insert
			if (insert||((bool)(this->PAT.load(std::memory_order_relaxed)&0x20))){
				mc|=0x08;
				sc|=0x08;
			}
			int k=this->getVideoIndex(line,2*column+i,false);
			for (int j=0;j<6;j++){
				bool fg=(c>>j)&1;
				//c=c>>1;
				this->setVideoOtputABGR(k+2*j,fg?mc:sc);
				this->setVideoOtputABGR(k+2*j+1,fg?mc:sc);
			}
		}
	}
	else{//40 char long/short
		unsigned char a=this->ROW_BUFFER[3*column+2].load(std::memory_order_relaxed);
		unsigned char b=this->ROW_BUFFER[3*column+1].load(std::memory_order_relaxed);
		unsigned char c=this->ROW_BUFFER[3*column].load(std::memory_order_relaxed);
		bool i2=false;
		bool U=false;
		bool L=false;
		bool m=(bool)(b&0x04);
		bool i1=(bool)(b&0x01);
		bool N=(bool)(a&0x80);
		bool F=(bool)(a&0x08);
		//unsigned char mc=(a<<1)&0xE0;
		//unsigned char sc=(a<<5)&0xE0;
		unsigned char mc=(a>>4)&0x0F;
		unsigned char sc=a&0x0F;
		bool comp=false;
		//slice
		unsigned char mask=0x0F;
		if ((bool)(this->TGS.load(std::memory_order_relaxed)&0x40)) mask=0x0A;//40 char short Q0-Q7 and GOE not reachable
		switch ((b>>4)&mask){
			case 0:
			case 1://G0
			case 4:
			case 5:
				//double height
				if ((bool)(b&0x02)){
					if (((bool)(c&0x80))||slice!=0){
						if ((bool)(c&0x80)) slice+=10;
						slice=(slice-1)/2;
					}
				}
				U=(bool)(b&0x10);
				i2=(bool)(b&0x40);
				L=(bool)(b&0x08);
				c=this->ROM_CHARSET[(c&0x7F)*10+slice].load(std::memory_order_relaxed);
				break;
			case 2://G10
			case 6:
				//double height
				if ((bool)(b&0x02)){
					if ((bool)(c&0x80)) slice+=10;
					slice=slice/2;
				}
				i2=(bool)(b&0x40);
				L=(bool)(b&0x08);
				c=this->ROM_CHARSET[(c&0x7F)*10+1280+slice].load(std::memory_order_relaxed);
				break;
			case 3://GOE
			case 7:
				//double height
				if ((bool)(b&0x02)){
					if ((bool)(c&0x80)) slice+=10;
					slice=slice/2;
				}
				i2=(bool)(b&0x40);
				L=(bool)(b&0x08);
				c=this->ROM_CHARSET[(c&0x7F)*10+2560+slice].load(std::memory_order_relaxed);
				break;
			case 8:
			case 9://G'0
			{
				//double height
				if ((bool)(b&0x02)){
					if (((bool)(c&0x80))||slice!=0){
						if ((bool)(c&0x80)) slice+=10;
						slice=(slice-1)/2;
					}
				}
				U=(bool)(b&0x10);
				L=(bool)(b&0x08);
				if ((bool)(this->TGS.load(std::memory_order_relaxed)&0x40)) i2=(bool)(b&0x40);
				unsigned char Z=this->DOR.load(std::memory_order_relaxed);
				Z=(Z&0x0F)|((Z&0x80)>>3);
				c=this->VRAM[this->getSliceRAMIndex(slice,c,Z)];
				break;
			}
			case 10://G'10
			{
				//double height
				if ((bool)(b&0x02)){
					if ((bool)(c&0x80)) slice+=10;
					slice=slice/2;
				}
				L=(bool)(b&0x08);
				if ((bool)(this->TGS.load(std::memory_order_relaxed)&0x40)) i2=(bool)(b&0x40);
				unsigned char Z=this->DOR.load(std::memory_order_relaxed);
				Z=(Z&0xF0)>>3;
				c=this->VRAM[this->getSliceRAMIndex(slice,c,Z)];
				break;
			}
			case 11://G'11
			{
				//double height
				if ((bool)(b&0x02)){
					if ((bool)(c&0x80)) slice+=10;
					slice=slice/2;
				}
				L=(bool)(b&0x08);
				unsigned char Z=this->DOR.load(std::memory_order_relaxed);
				Z=((Z&0xF0)>>3)|1;
				c=this->VRAM[this->getSliceRAMIndex(slice,c,Z)];
				break;
			}
			case 12:
			case 13:
			case 14:
			case 15://Q0-Q7
			{
				//double height
				if ((bool)(b&0x02)){
					if ((bool)(c&0x80)) slice+=10;
					slice=slice/2;
				}
				unsigned char Z=this->DOR.load(std::memory_order_relaxed);
				constexpr unsigned char l[]={0x08,0x0A,0x09,0x0B,0x0C,0x0E,0x0D,0x0F};
				Z=l[(b&0x38)>>3]|((Z&0x80)>>3);
				c=this->VRAM[this->getSliceRAMIndex(slice,c,Z)];
				break;
			}
		}
		//double length
		if (this->shift_slice) c=c>>4;
		if (L){
			c=c&0x0F;//double bit pattern
			c=(c|(c<<2))&0x33;
			c=(c|(c<<1))&0x55;
			c|=c<<1;
			this->shift_slice=!this->shift_slice;
		}
		else this->shift_slice=false;
		//cursor
		if ((bool)(this->MAT.load(std::memory_order_relaxed)&0x40)){//cursor enabled
			if (this->getX(true)==column&&this->getY(true)==this->getYFromRow(line/10)&&this->getD(true)==this->getDisplayDistrict()){//on the cursor
				if (this->n_frame<25||(!(bool)(this->MAT.load(std::memory_order_relaxed)&0x20))){
					U^=(this->MAT.load(std::memory_order_relaxed)&0x10)<<1;//U
					comp=!(bool)(this->MAT.load(std::memory_order_relaxed)&0x10);
				}
			}
		}
		//U
		if (U&&slice==9) c=0xFF;
		//F
		if (F&&((bool)(this->PAT.load(std::memory_order_relaxed)&0x40))){
			if (N&&this->n_frame<25) c=0;
			else if ((!N)&&this->n_frame>=25) c=0;
		}
		//conceal
		if (m&&((bool)(this->PAT.load(std::memory_order_relaxed)&0x08))) c=0;
		//N
		if ((bool)(a&0x80)) c=~c;
		//color complementation
		if (comp){
			//mc=mc^0xE0;
			//sc=sc^0xE0;
			mc=mc^0x07;
			sc=sc^0x07;
		}
		//insert
		/*switch (this->PAT.load(std::memory_order_relaxed)&0x30){
			case 0x00:
				sc=0;
				if (!i1) mc=0;
				break;
			case 0x10:
				if (i1){
					if (i2) sc=0;
				}
				else{
					sc=0;
					mc=0;
				}
				break;
		}*/
		switch (this->PAT.load(std::memory_order_relaxed)&0x30){
			case 0x00:
				if (i1) mc|=0x08;
				break;
			case 0x10:
				if (i1){
					if (!i2) sc|=0x08;
					mc|=0x08;
				}
				break;
			case 0x20:
			case 0x30:
				sc|=0x08;
				mc|=0x08;
		}
		//video output
		int j=this->getVideoIndex(line,column,true);
		for (int i=0;i<8;i++){
			unsigned char clr=(((c>>i)&1)?mc:sc);
			this->setVideoOtputABGR(j+3*i,clr);
			this->setVideoOtputABGR(j+3*i+1,clr);
			this->setVideoOtputABGR(j+3*i+2,clr);
		}
	}
	
}

//Commands

bool TS9347wVRAM::isICSelected(){return (this->ADDR_BUF.load(std::memory_order_relaxed)&0x1F0)==0x020;}

bool TS9347wVRAM::isBusy(){return (bool)(this->STATUS.load(std::memory_order_relaxed)&this->BUSY_MASK);}

bool TS9347wVRAM::requestExecution(){return (this->ADDR_BUF.load(std::memory_order_relaxed)&0x008)==0x008;}

bool TS9347wVRAM::incrementX(bool MP){
	unsigned char r=this->Rx[MP?7:5].load(std::memory_order_relaxed);
	if ((r&0x3F)>=39) r&=0xC0;
	else r++;
	this->Rx[MP?7:5].store(r,std::memory_order_relaxed);
	return (r&0x3F)==0;
}

unsigned char TS9347wVRAM::getX(bool MP){
	return this->Rx[MP?7:5].load(std::memory_order_relaxed)&0x3F;
}

void TS9347wVRAM::incrementY(bool MP){
	unsigned char r=this->Rx[MP?6:4].load(std::memory_order_relaxed);
	if ((r&0x1F)==0x1F) r=(r&0xE0)|0x08;
	else r++;
	this->Rx[MP?6:4].store(r,std::memory_order_relaxed);
}

unsigned char TS9347wVRAM::getY(bool MP){
	return this->Rx[MP?6:4].load(std::memory_order_relaxed)&0x1F;
}

void TS9347wVRAM::incrementB(bool MP){
	unsigned char r=this->Rx[MP?7:5].load(std::memory_order_relaxed);
	//r+=0x40;
	constexpr unsigned char l[]={0x80,0xC0,0x40,0x00};
	r=(l[r>>6])|(r&0x3F);
	this->Rx[MP?7:5].store(r,std::memory_order_relaxed);
}

void TS9347wVRAM::addB(bool MP,unsigned char d){
	unsigned char r=this->Rx[MP?7:5].load(std::memory_order_relaxed);
	//r+=d<<6;
	constexpr unsigned char l1[]={0x00,0x02,0x01,0x03};
	constexpr unsigned char l2[]={0x00,0x80,0x40,0xC0};
	r=(l2[((l1[r>>6])+d)&0x03])|(r&0x3F);
	this->Rx[MP?7:5].store(r,std::memory_order_relaxed);
}

unsigned char TS9347wVRAM::getB(bool MP){
	constexpr unsigned char l[]={0x00,0x02,0x01,0x03};
	return l[this->Rx[MP?7:5].load(std::memory_order_relaxed)>>6];
}

unsigned char TS9347wVRAM::getD(bool MP){
	return this->Rx[MP?6:4].load(std::memory_order_relaxed)>>5;
}

int TS9347wVRAM::pointer2RAMAddress(bool MP){
	unsigned char r1=this->Rx[MP?6:4].load(std::memory_order_relaxed);
	unsigned char r2=this->Rx[MP?7:5].load(std::memory_order_relaxed);
	if ((r2&0x3F)>=40){//protect against X>=40 - ex: mem fnct+T F mem mem -> service row corrupted at X=0 (position 0 and 1 because 80 collumn display)
		r2&=0xC0;
		this->Rx[MP?7:5].store(r2,std::memory_order_relaxed);
	}
	
	int A=(r1&0xE0)<<7;
	A|=(r2&0x40)<<5;
	
	if ((bool)(r1&0x18)){
		A|=(r2&0x80)<<3;
		A|=(r1&0x07)<<5;
		if ((bool)(r2&0x20)){
			A|=r1&0x18;
		}
		else{
			A|=(r1&0x18)<<5;
			A|=r2&0x18;
		}
	}
	else{
		if ((bool)(r1&0x01)){
			if ((bool)(r2&0x80)){
				A|=0x0400;
			}
			else{
				A|=(r2&0x08)<<7;
			}
			A|=0x0080;
			A|=0x0060^((r2&0x30)<<1);
		}
		else{
			A|=(r2&0x80)<<3;
			A|=(r2&0x38)<<2;
		}
	}
	
	A|=r2&0x07;
	return A;
}

void TS9347wVRAM::executeCommand(){
	if (this->isBusy()){
		switch (this->Rx[0].load(std::memory_order_relaxed)&0xF0){
			case 0x00://TLM/TSM/CLL/CLS
				////////////////////////////
				switch (this->Rx[0].load(std::memory_order_relaxed)&0x0F){
					case 0x00:
					case 0x01:
					case 0x08:
					case 0x09:
					case 0x0A:
					case 0x0B:
						this->TLM();
						break;
					case 0x02:
					case 0x03:
						this->TSM();
						break;
					case 0x05:
						this->CLL();
						break;
					case 0x07:
						this->CLS();
						break;
					default:
						this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
						printf("error %02X\n",this->Rx[0].load(std::memory_order_relaxed));
						break;
				}
				break;
			case 0x20://TLA
				this->TLA();
				break;
			case 0x30://TBM/TBA
				if (this->Rx[0].load(std::memory_order_relaxed)&0x04) this->TBA();
				else this->TBM();
				break;
			case 0x40://KRS
				this->KRS();
				break;
			case 0x50://KRL
				this->KRL();
				break;
			case 0x60://TSM/CLS
				if ((this->Rx[0].load(std::memory_order_relaxed)&0x04)==0) this->TSM();
				else if ((this->Rx[0].load(std::memory_order_relaxed)&0x0D)==0x05) this->CLS();
				else{
					printf("error %02X\n",this->Rx[0].load(std::memory_order_relaxed));
					this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
				}
				break;
			case 0x70://TSA
				this->TSA();
				break;
			case 0x80://IND
				this->IND();
				break;
			case 0x90://NOP/VRM/VSM
				switch (this->Rx[0].load(std::memory_order_relaxed)&0x0F){
					case 0x01:
						this->NOP();
						break;
					case 0x05:
						this->VRM();
						break;
					case 0x09:
						this->VSM();
						break;
					default:
						this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
						printf("error %02X\n",this->Rx[0].load(std::memory_order_relaxed));
						break;
				}
				break;
			case 0xB0://INY
				this->INY();
				break;
			case 0xD0://MVB
				this->MVB();
				break;
			case 0xE0://MVD
				this->MVD();
				break;
			case 0xF0://MVT
				this->MVT();
				break;
			default:
				this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
				printf("error %02X\n",this->Rx[0].load(std::memory_order_relaxed));
				break;
		}
	}
}

void TS9347wVRAM::NOP(){
	switch (this->cmd_step){
		case 0:
			this->cmd_step=1;
			break;
		case 1:
			this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::VRM(){
	switch (this->cmd_step){
		case 0:
			this->cmd_step=1;
			break;
		case 1:
			this->vsync_mask=false;
			if ((this->clk_frame>>7)==21||(this->clk_frame>>7)==22) this->STATUS.fetch_or(this->VSYNC_MASK,std::memory_order_relaxed);
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::VSM(){
	switch (this->cmd_step){
		case 0:
			this->cmd_step=1;
			break;
		case 1:
			this->vsync_mask=true;
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK|this->VSYNC_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::INY(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
		case 2:
			this->incrementY(true);
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::IND(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 2:
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				switch (this->Rx[0].load(std::memory_order_relaxed)&0x07){
					case 0:
						this->Rx[1].store(0,std::memory_order_relaxed);///////////////////////////////////////////////////////////////
						break;
					case 1:
						this->Rx[1].store(this->TGS.load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 2:
						this->Rx[1].store(this->MAT.load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 3:
						this->Rx[1].store(this->PAT.load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 4:
						this->Rx[1].store(this->DOR.load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 7:
						this->Rx[1].store(this->ROR.load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
				}
			}
			else{//write
				switch (this->Rx[0].load(std::memory_order_relaxed)&0x07){
					case 1:
						this->TGS.store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 2:
						this->MAT.store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 3:
						this->PAT.store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 4:
						this->DOR.store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
					case 7:
						this->ROR.store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
						break;
				}
			}
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)) this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			this->cmd_step++;
			break;
		case 3:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::MVB(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 5:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			
			this->cmd_step=2;
			if (this->incrementX(true)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=6;
				}
				else{
					this->incrementY(true);
				}
			}
			if (this->incrementX(false)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=6;
				}
			}
			break;
		}
		
		case 6:
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::MVD(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 5:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->incrementB(false);
			this->cmd_step++;
			break;
		}
		case 9:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			
			this->cmd_step=2;
			this->addB(true,3);
			this->addB(false,3);
			if (this->incrementX(true)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=10;
				}
				else{
					this->incrementY(true);
				}
			}
			if (this->incrementX(false)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=10;
				}
			}
			break;
		}	
		case 10:
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::MVT(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 5:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->incrementB(false);
			this->cmd_step++;
			break;
		}
		case 9:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->incrementB(false);
			this->cmd_step++;
			break;
		}
		case 13:
		{
			bool b=(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x04);
			this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(b)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->VRAM[this->pointer2RAMAddress(!b)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			
			this->cmd_step=2;
			this->addB(true,2);
			this->addB(false,2);
			if (this->incrementX(true)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=14;
				}
				else{
					this->incrementY(true);
				}
			}
			if (this->incrementX(false)){
				if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//stop
					this->cmd_step=14;
				}
			}
			break;
		}
		case 14:
			this->STATUS.fetch_and((unsigned char)~(this->BUSY_MASK|this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			break;
	}
}

void TS9347wVRAM::TBM(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 4:
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->late_cmd_end=true;
			}
			else{
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			}
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(true)){
					this->incrementY(true);
					this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(true)==39){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			break;
	}
}

void TS9347wVRAM::TBA(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 4:
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->late_cmd_end=true;
			}
			else{
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			}
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(false)){
					this->STATUS.fetch_or(this->Al_MASK|this->LXa_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(false)==39){
				this->STATUS.fetch_or(this->LXa_MASK,std::memory_order_relaxed);
			}
			break;
	}
}

void TS9347wVRAM::KRS(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 9:
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->late_cmd_end=true;
			}
			else{
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			}
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if ((bool)(this->getB(true)&1)){
					this->addB(true,3);
					if (this->incrementX(true)){
						this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
					}
				}
				else{
					this->incrementB(true);
				}
			}
			else if (this->getX(true)==39&&((bool)(this->getB(true)&1))){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			break;
	}
}

void TS9347wVRAM::KRL(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 11:
		{
			bool b=(bool)(this->getB(true)&1);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				
				unsigned char a;
				if (b){
					this->incrementB(true);
					a=this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed)&0x0F;
					a|=a<<4;
					this->addB(true,3);
				}
				else{
					this->addB(true,2);
					a=this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed)&0xF0;
					a|=a>>4;
					this->addB(true,2);
				}
				this->Rx[3].store(a,std::memory_order_relaxed);
				
				this->late_cmd_end=true;
			}
			else{
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				
				if (b){
					this->incrementB(true);
					this->VRAM[this->pointer2RAMAddress(true)].store((this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed)&0xF0)|
																	 (this->Rx[3].load(std::memory_order_relaxed)&0x0F),std::memory_order_relaxed);
					this->addB(true,3);
				}
				else{
					this->addB(true,2);
					this->VRAM[this->pointer2RAMAddress(true)].store((this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed)&0x0F)|
																	 (this->Rx[3].load(std::memory_order_relaxed)&0xF0),std::memory_order_relaxed);
					this->addB(true,2);
				}
			}
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (b){
					this->addB(true,3);
					if (this->incrementX(true)){
						this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
					}
				}
				else{
					this->incrementB(true);
				}
			}
			else if (this->getX(true)==39&&b){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			
			this->cmd_step++;
			break;
		}
		case 12:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::TSM(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 3:
		{
			bool b=this->getB(true);
			if (b) this->addB(true,3);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->Rx[2].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			else{
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			
			//if (!b) 
			this->addB(true,3);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(true)){
					this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(true)==39){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			if (!(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){
				this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			}
			
			this->cmd_step++;
			break;
		}
		case 5:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::TSA(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 3:
		{
			bool b=this->getB(false);
			if (b) this->addB(false,3);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->Rx[2].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			else{
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			
			//if (!b) 
			this->addB(false,3);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(false)){
					this->STATUS.fetch_or(this->Al_MASK|this->LXa_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(false)==39){
				this->STATUS.fetch_or(this->LXa_MASK,std::memory_order_relaxed);
			}
			if (!(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)) this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			
			this->cmd_step++;
			break;
		}
		case 5:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::CLS(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 3:
		{
			bool b=this->getB(true);
			if (b) this->addB(true,3);
			
			this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
			
			//if (!b) 
			this->addB(true,3);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if (this->incrementX(true)){
				this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				this->cmd_step++;
			}
			else{
				this->cmd_step=0;
			}
			break;
		}
		case 5:
			this->incrementY(true);
			this->cmd_step=0;
			break;
	}
}

void TS9347wVRAM::TLM(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 4:
		{
			bool b=this->getB(true);
			if (b) this->addB(true,3);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->Rx[2].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->Rx[3].store(this->VRAM[this->pointer2RAMAddress(true)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			else{
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(true);
				this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[3].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			
			//if (b) this->addB(true,3);
			//else 
			this->addB(true,2);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(true)){
					this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(true)==39){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			if (!(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)) this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			
			this->cmd_step++;
			break;
		}
		case 7:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::TLA(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 4:
		{
			bool b=this->getB(false);
			if (b) this->addB(false,3);
			
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)){//read
				this->Rx[1].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->Rx[2].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->Rx[3].store(this->VRAM[this->pointer2RAMAddress(false)].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			else{
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
				this->incrementB(false);
				this->VRAM[this->pointer2RAMAddress(false)].store(this->Rx[3].load(std::memory_order_relaxed),std::memory_order_relaxed);
			}
			
			//if (b) this->addB(false,3);
			//else 
			this->addB(false,2);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if ((bool)(this->Rx[0].load(std::memory_order_relaxed)&0x01)){//inc
				if (this->incrementX(false)){
					this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				}
			}
			else if (this->getX(false)==39){
				this->STATUS.fetch_or(this->LXm_MASK,std::memory_order_relaxed);
			}
			if (!(bool)(this->Rx[0].load(std::memory_order_relaxed)&0x08)) this->STATUS.fetch_and((unsigned char)~this->BUSY_MASK,std::memory_order_relaxed);
			
			this->cmd_step++;
			break;
		}
		case 7:
			this->late_cmd_end=true;
			break;
	}
}

void TS9347wVRAM::CLL(){
	switch (this->cmd_step){
		default:
			this->cmd_step++;
			break;
			
		case 4:
		{
			bool b=this->getB(true);
			if (b) this->addB(true,3);
			
			this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[1].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[2].load(std::memory_order_relaxed),std::memory_order_relaxed);
			this->incrementB(true);
			this->VRAM[this->pointer2RAMAddress(true)].store(this->Rx[3].load(std::memory_order_relaxed),std::memory_order_relaxed);
			
			//if (b) this->addB(true,3);
			//else 
			this->addB(true,2);
			
			this->STATUS.fetch_and(~(this->Al_MASK|this->LXm_MASK|this->LXa_MASK),std::memory_order_relaxed);
			if (this->incrementX(true)){
				this->STATUS.fetch_or(this->Al_MASK|this->LXm_MASK,std::memory_order_relaxed);
				this->cmd_step++;
			}
			else{
				this->cmd_step=0;
			}
			break;
		}
		case 6:
			this->incrementY(true);
			this->cmd_step=0;
			break;
	}
}