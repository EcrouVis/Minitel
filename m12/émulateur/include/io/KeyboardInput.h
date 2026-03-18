#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H

#include "thread_messaging.h"

void KeyboardInput(Mailbox* p_mb,bool focus, int scancode, int action, int mods){
	if (action==GLFW_PRESS||action==GLFW_RELEASE){
		thread_message ms_p_kb;
		keyboard_message* kbm=new keyboard_message;
		kbm->focus=focus;
		kbm->scancode=scancode;
		kbm->action=action;
		kbm->mods=mods;
		ms_p_kb.p=(void*)kbm;
		ms_p_kb.cmd=KEYBOARD_STATE_UPDATE;
		p_mb->send(&ms_p_kb);
	}
}

#endif