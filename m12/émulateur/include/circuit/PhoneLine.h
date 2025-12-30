#ifndef PHONELINE_H
#define PHONELINE_H

const unsigned short line_update_frequency=4800;

const unsigned short line_Closed=0x0001;//0Hz
const unsigned short line_Ringing=0x0002;//50Hz
const unsigned short line_v23_75bps_1=0x0004;//390Hz
const unsigned short line_call_progress_tone=0x0008;//440Hz
const unsigned short line_v23_75bps_0=0x0010;//450Hz
const unsigned short line_DTMF_697Hz=0x0020;
const unsigned short line_DTMF_770Hz=0x0040;
const unsigned short line_DTMF_852Hz=0x0080;
const unsigned short line_DTMF_941Hz=0x0100;
const unsigned short line_DTMF_1209Hz=0x0200;
const unsigned short line_v23_1200bps_1=0x0400;//1300Hz
const unsigned short line_DTMF_1336Hz=0x0800;
const unsigned short line_DTMF_1477Hz=0x1000;
const unsigned short line_DTMF_1633Hz=0x2000;
const unsigned short line_v23_1200bps_0=0x4000;//2100Hz

const unsigned short line_DTMF_1=line_DTMF_697Hz|line_DTMF_1209Hz;
const unsigned short line_DTMF_2=line_DTMF_697Hz|line_DTMF_1336Hz;
const unsigned short line_DTMF_3=line_DTMF_697Hz|line_DTMF_1477Hz;
const unsigned short line_DTMF_A=line_DTMF_697Hz|line_DTMF_1633Hz;
const unsigned short line_DTMF_4=line_DTMF_770Hz|line_DTMF_1209Hz;
const unsigned short line_DTMF_5=line_DTMF_770Hz|line_DTMF_1336Hz;
const unsigned short line_DTMF_6=line_DTMF_770Hz|line_DTMF_1477Hz;
const unsigned short line_DTMF_B=line_DTMF_770Hz|line_DTMF_1633Hz;
const unsigned short line_DTMF_7=line_DTMF_852Hz|line_DTMF_1209Hz;
const unsigned short line_DTMF_8=line_DTMF_852Hz|line_DTMF_1336Hz;
const unsigned short line_DTMF_9=line_DTMF_852Hz|line_DTMF_1477Hz;
const unsigned short line_DTMF_C=line_DTMF_852Hz|line_DTMF_1633Hz;
const unsigned short line_DTMF_S=line_DTMF_941Hz|line_DTMF_1209Hz;
const unsigned short line_DTMF_0=line_DTMF_941Hz|line_DTMF_1336Hz;
const unsigned short line_DTMF_H=line_DTMF_941Hz|line_DTMF_1477Hz;
const unsigned short line_DTMF_D=line_DTMF_941Hz|line_DTMF_1633Hz;

const float line_DTMF_HF_Attenuation=0.3548;
const float line_DTMF_LF_Attenuation=0.2818;
#endif