#include "desktop/window_creation.h"
#include <cstdio>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include "GLFW/glfw3native.h"
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#endif


void WindowsDarkTitleBar(GLFWwindow* window){
#ifdef _WIN32
	//black titlebar for windows
	//DwmSetWindowAttribute defined since Windows Vista
	BOOL USE_DARK_MODE = true;
	BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(glfwGetWin32Window(window), DWMWA_USE_IMMERSIVE_DARK_MODE,&USE_DARK_MODE, sizeof(USE_DARK_MODE)));
	if (!SET_IMMERSIVE_DARK_MODE_SUCCESS) printf("Dark mode titlebar failed");
#endif
}

#if defined(unix) || defined(__unix__)
#include <cstdlib>
#include <cctype>
#include <cstring>
#endif

void LinuxInitHintBackend(){
#if defined(unix) || defined(__unix__)
	const char* backend=getenv("M12_BACKEND");
	if (backend==NULL) return;
	char* bm=(char*)malloc(strlen(backend)+1);
	for(size_t i=0;i<strlen(backend);i++){
		bm[i]=std::tolower(backend[i]);
	}
	bm[strlen(backend)]=0;
	if (strcmp(bm,"x11")==0){
		glfwInitHint(GLFW_PLATFORM,GLFW_PLATFORM_X11);
	}
	else if (strcmp(bm,"wayland")==0){
		glfwInitHint(GLFW_PLATFORM,GLFW_PLATFORM_WAYLAND);
	}
#endif
}