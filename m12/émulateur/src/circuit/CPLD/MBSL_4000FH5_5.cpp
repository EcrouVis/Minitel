#include "circuit/CPLD.h"

//#define timegm _mkgmtime
//using namespace std;

///////////////////////////////////////////////////////////////////////
//timegm is non standard -> fix to implement timegm
// Algorithm: http://howardhinnant.github.io/date_algorithms.html
int days_from_epoch(int y, int m, int d)
{
    y -= m <= 2;
    int era = y / 400;
    int yoe = y - era * 400;                                   // [0, 399]
    int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // [0, 365]
    int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;           // [0, 146096]
    return era * 146097 + doe - 719468;
}

// It  does not modify broken-down time
time_t timegm(struct tm const* t)     
{
    int year = t->tm_year + 1900;
    int month = t->tm_mon;          // 0-11
    if (month > 11)
    {
        year += month / 12;
        month %= 12;
    }
    else if (month < 0)
    {
        int years_diff = (11 - month) / 12;
        year -= years_diff;
        month += 12 * years_diff;
    }
    int days_since_epoch = days_from_epoch(year, month + 1, t->tm_mday);

    return 60 * (60 * (((time_t)24) * days_since_epoch + t->tm_hour) + t->tm_min) + t->tm_sec;
}
///////////////////////////////////////////////////////////////////////

void MBSL_4000FH5_5::DChangeIn(unsigned char d){
	this->data=d;
}
void MBSL_4000FH5_5::ALEChangeIn(bool b){
	if (b&&!this->ALE){
		this->address=this->data;
		this->sendAL(this->data);
	}
	this->ALE=b;
}
void MBSL_4000FH5_5::nOEChangeIn(bool b){
	if((!b)&&this->nOE&&(!this->nCS)) {//to uc
		this->CPLD2UC();
	}
	this->nOE=b;
}
void MBSL_4000FH5_5::nWEChangeIn(bool b){
	if(b&&(!this->nWE)&&(!this->nCS)){//from uc
		this->UC2CPLD();
	}
	this->nWE=b;
}
void MBSL_4000FH5_5::nCSChangeIn(bool b){
	this->nCS=b;
	this->sendnCSRAM(!b);
}
void MBSL_4000FH5_5::WATCHDOGChangeIn(bool b){
	this->WATCHDOG=b;
	//this->sendRST(b);
}

void MBSL_4000FH5_5::subscribenCSRAM(std::function<void(bool)> f){
	this->sendnCSRAM=f;
}
void MBSL_4000FH5_5::subscribeAL(std::function<void(unsigned char)> f){
	this->sendAL=f;
}
void MBSL_4000FH5_5::subscribeD(std::function<void(unsigned char)> f){
	this->sendD=f;
}
void MBSL_4000FH5_5::subscribePIO(std::function<void(unsigned char)> f){
	this->sendPIO=f;
}
void MBSL_4000FH5_5::subscribeRST(std::function<void(bool)> f){
	this->sendRST=f;
}

void MBSL_4000FH5_5::subscribeSerial(std::function<void(bool)> f){
	this->sendSerial=f;
}

void MBSL_4000FH5_5::serialChangeIn(bool b){
	if (b&&(!this->S_in)&&this->S_in_step==0) this->S_in_step=1;
	this->S_in=b;
}

void MBSL_4000FH5_5::CLKTickIn(){
	if (this->S_in_step>0){
		switch (this->S_in_step){
			case 1:this->S_in_step++;break;
			
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:this->SBUF_in_tmp=(this->SBUF_in_tmp>>1)|(this->S_in?0:0x80);this->S_in_step++;break;
			
			case 10:
				this->SBUF_in=this->SBUF_in_tmp;
				if ((bool)(this->STATUS&(1<<5))) this->STATUS|=0x03;
				else this->STATUS&=0xFC;
				this->STATUS|=1<<5;
				this->S_in_step=0;
				break;
				
		}
	}
	if (this->S_out_step>0){
		switch (this->S_out_step){
			case 1:this->sendSerial(true);this->S_out_step++;break;
			
			case 2:
			case 4:
			case 6:
			case 8:
			case 10:
			case 12:
			case 14:
			case 16:
			case 18:
			case 20:
			case 21:
			case 22:
			case 23:this->S_out_step++;break;
			
			case 3:
			case 5:
			case 7:
			case 9:
			case 11:
			case 13:
			case 15:
			case 17:
				this->sendSerial(!(bool)(this->SBUF_out&1));
				this->SBUF_out=this->SBUF_out>>1;
				this->S_out_step++;
				break;
			
			case 19:this->sendSerial(false);this->S_out_step++;break;
			
			case 24:this->S_out_step=0;this->STATUS|=0x0C;break;
		}
	}
}


unsigned char MBSL_4000FH5_5::D2BCD(unsigned char D){
	D=D%100;
	return (D%10)|((D/10)<<4);
}
unsigned char MBSL_4000FH5_5::BCD2D(unsigned char BCD){
	return (BCD&0x0F)+(BCD>>4)*10;
}

void MBSL_4000FH5_5::UC2CPLD(){
	std::time_t ts;
	struct tm date;
	switch(this->address){
		case 0x50://serial buffer out
			//printf("to keyboard %02X\n",this->data);
			this->STATUS&=0xF3;
			this->STATUS|=(1<<4);
			this->SBUF_out=this->data;
			this->S_out_step=1;
			break;
		case 0x51:
			//printf("to CPLD status %02X\n",this->data);
			this->STATUS=this->data;
			if ((bool)(this->STATUS&(1<<4))) this->STATUS|=0x0C;
			break;
		case 0x70://speculations: 0: MC/nBC / 1: modem/nDMTF / 2: disable comunication to and reset keyboard? / 3: watchdog timer in / 4: close modem line? / 5: enable CRT / 6: M/V / 7: ??? - true when in settings
			//printf("to CPLD IO %02X\n",this->data);
			this->IO=this->data;
			this->sendPIO(this->data);
			break;
		case 0x40:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_min=this->BCD2D(this->data);
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		case 0x41:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_hour=this->BCD2D(this->data);
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		case 0x42:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_mday=this->BCD2D(this->data);
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		case 0x43:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_mon=this->BCD2D(this->data)-1;
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		case 0x44:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_year=(date.tm_year/100)*100+this->BCD2D(this->data);
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		case 0x45:
			ts=std::time(NULL);
			ts+=this->diff_time;
			date=*std::gmtime(&ts);
			date.tm_year=(date.tm_year%100)+this->BCD2D(this->data)*100-1900;
			this->diff_time+=std::difftime(timegm(&date),ts);
			break;
		default:
			break;
	}
}

void MBSL_4000FH5_5::CPLD2UC(){
	std::time_t ts;
	struct tm date;
	switch(this->address){
		case 0x50://serial buffer in
		{
			this->sendD(this->SBUF_in);
			this->STATUS&=~(1<<5);
			break;
		}
		case 0x51://0x51: 5: data in SBUFin / !0&&!1: set i25.6 after reading SBUFin / 2&&3&&i22.6: accept data to SBUFout / 4: data in SBUF out? - start serial? / 6: reset/pause RTC? - used only when changing date /7: reset?
		{//end init->0b01010001
			unsigned char d=this->STATUS;
			//printf("from CPLD status %02X\n",d);
			this->sendD(d);
			break;
		}
		case 0x40://Minutes BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			this->sendD(this->D2BCD((unsigned char)date.tm_min));
			break;
		case 0x41://Hours BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			this->sendD(this->D2BCD((unsigned char)date.tm_hour));
			break;
		case 0x42://Day BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			this->sendD(this->D2BCD((unsigned char)date.tm_mday));
			break;
		case 0x43://Month BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			date.tm_mon++;
			this->sendD(this->D2BCD((unsigned char)date.tm_mon));
			break;
		case 0x44://Year low BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			this->sendD(this->D2BCD((unsigned char)date.tm_year));
			break;
		case 0x45://Year high BCD format
			ts=std::time(NULL);
			if (this->OS_RTC.load(std::memory_order_relaxed)) date=*std::localtime(&ts);
			else{
				ts+=this->diff_time;
				date=*std::gmtime(&ts);
			}
			date.tm_year+=1900;
			this->sendD(this->D2BCD((unsigned char)(date.tm_year/100)));
			break;
		case 0x70:
			this->sendD(this->IO);
			break;
		case 0x96:
			printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			break;
		default:
			break;//4 minutes CRT off
	}
}