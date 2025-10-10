#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "linmath.h"
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <thread>
#include "thread_messaging.h"

const char* window_title="Minitel 12 Philips";

bool show_imgui=true;
bool imgui_rgb_video=false;

const char* imgui_notification_message_list[]={
	"BIP!!!",
	"Suspension de l'émulation",
	"Reprise de l'émulation",
	"Redémarrage du minitel",
	"Appuyez sur F1 pour faire apparaitre le menu"};
typedef struct imgui_notification{
	const char* message;
	ImVec4 color;
	double timestamp;
	imgui_notification* older;
} imgui_notification;
imgui_notification* imgui_notification_list=NULL;
void imgui_notify(int message,ImVec4 color=ImVec4(1.,1.,1.,1.)){
	imgui_notification* notif=new imgui_notification;
	notif->message=imgui_notification_message_list[message];
	notif->color=color;
	notif->timestamp=glfwGetTime();
	notif->older=imgui_notification_list;
	imgui_notification_list=notif;
}
bool imgui_delete_old_notifications(double duration=5,int max_notification=5){
	double t=glfwGetTime()-duration;
	imgui_notification* tmp=imgui_notification_list;
	bool show=false;
	int n=1;
	while (tmp!=NULL&&tmp->older!=NULL&&tmp->older->timestamp>=t&&(n<max_notification||max_notification<0)){
		show=true;
		tmp=tmp->older;
		n++;
	}
	if (tmp!=NULL){
		imgui_notification* tmp2=tmp2=tmp->older;
		tmp->older=NULL;
		tmp=tmp2;
		while (tmp!=NULL){
			tmp2=tmp;
			tmp=tmp->older;
			delete tmp2;
		}
	}
	return show;
}
void imgui_notification_window(double duration=5,double duration_fading=1,int max_notification=-1){
	if (imgui_delete_old_notifications(duration,max_notification)){
		ImGui::SetNextWindowBgAlpha(0.75f);
		ImGui::SetNextWindowPos(ImVec2(10,10));
		ImGui::Begin("Notifications",NULL,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoBringToFrontOnFocus|ImGuiWindowFlags_NoScrollbar);
		imgui_notification* tmp=imgui_notification_list;
		double t=glfwGetTime()-duration;
		while (tmp!=NULL){
			double t_f=(tmp->timestamp-t)/duration_fading;
			if (t_f>0&&t_f<1){
				ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(tmp->color.x,tmp->color.y,tmp->color.z,tmp->color.w*t_f));
			}
			else{
				ImGui::PushStyleColor(ImGuiCol_Text,tmp->color);
			}
			ImGui::Text(tmp->message);
			ImGui::PopStyleColor();
			tmp=tmp->older;
		}
		ImGui::End();
	}	
}

void imgui_main_menu_window(bool* p_open){
	ImGui::Begin("Menu",p_open,ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Text("Appuyez sur F1 pour afficher/cacher le menu");
	if (ImGui::CollapsingHeader("Hacks")){
		ImGui::Checkbox("Sortie vidéo RGB",&imgui_rgb_video);
	}
	if (ImGui::CollapsingHeader("Débuggage")){
	}
	if (ImGui::CollapsingHeader("À propos")){
		ImGui::Text(window_title);
	}
	ImGui::End();
}


extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

//vertex x y
//uniform A B C
//position x0=(6 or 12)*x y0=10*y h=10 w=(6 or 8)
//w=480 h=250
const float screen_width=480;//320;
const float screen_height=250;
 
 
typedef struct Vertex{
    vec2 pos;
} Vertex;
 
static const Vertex vertices[6] =
{
    { {0.f,0.f}},
    { {1.f,0.f}},
    { {1.f,1.f}},
    { {1.f,1.f}},
    { {0.f,1.f}},
    { {0.f,0.f}}
};

const unsigned int S_40CPL=1;
const unsigned int S_DOUBLE_HEIGHT=1<<1;
const unsigned int S_DOUBLE_CHAR_WIDTH=1<<2;
const unsigned int S_DOUBLE_CHAR_HEIGHT=1<<3;
//double:
//0: double_width 80/40
//1: double_char_width
//2: double_height
//3: double_char_height

const unsigned int C_UNDERLINE=1<<6;
const unsigned int C_CONCEAL=1<<7;
const unsigned int C_RGB=1<<8;
//color:
//0,1,2: fg_color
//3,4,5: bg_color
//6: underline
//7: conceal
//8: rgb mode

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"uniform ivec2 vP;\n"
"uniform ivec2 vCharP;\n"
"uniform unsigned int double;\n"
"in vec2 vPos;\n"
"out vec2 coord_texture;\n"
"const float mul[4]=float[](1.,2.,2.,4.);\n"
"void main(){\n"
"	 vec2 p_screen=vPos*vec2(mul[(double<<30)>>30],mul[double>>2])+vec2(vP);\n"
"    gl_Position = MVP*vec4(p_screen, 0.0, 1.0);\n"
"    coord_texture=vec2(bool((double<<31)>>31)?1.:0.75,1)*vPos-vec2(0,(bool(double>>3)&&(vCharP.y==0))?0.05:0);\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec2 coord_texture;\n"
"out vec4 fragment;\n"
"uniform ivec2 vCharP;\n"
"uniform unsigned int color;\n"
"uniform sampler2D charset;\n"
"const float grey[8]=float[](0.,2/7.,4/7.,6/7.,1/7.,3/7.,5/7.,1.);\n"
"void main(){\n"
"    vec2 ct=coord_texture+vec2(vCharP);\n"
"	 bool underline=bool((color<<25)>>31);\n"
"	 bool conceal=bool((color<<24)>>31);\n"
"	 bool rgb=bool((color<<23)>>31);\n"
"	 vec2 f=vec2(8,10)/vec2(textureSize(charset,0));\n"
"    bool fg=(((coord_texture.t>0.9)&&underline)?1.:texture(charset,ct*f).r)>0.5;\n"
"    fg=fg&&(!conceal);\n"
"	 unsigned int c=(color<<(fg?29:26))>>29;\n"
"	 if (rgb){\n"
"	 	 fragment=vec4((c<<31)>>31,(c<<30)>>31,(c<<29)>>31,1);\n"
"	 }\n"
"	 else{\n"
"    	 fragment=vec4(vec3(1,1,1)*grey[c],1);\n"
"	 }\n"
"}\n";
 
static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
	if (key==GLFW_KEY_F1&&action==GLFW_PRESS) show_imgui=!show_imgui;
	imgui_notify(3,ImVec4(0.,1.,1.,1.));
}
static void char_callback(GLFWwindow* window, unsigned int codepoint){
	fprintf(stdout,"%c\n",codepoint);
}

static void window_close_callback(GLFWwindow* window){
	/////////////////////////////////////////////////////
}
 
GLuint loadCharacterSetTexture(){
	GLuint charset_texture;
	glGenTextures(1, &charset_texture);
	glBindTexture(GL_TEXTURE_2D, charset_texture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	int width, height, nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char *data = stbi_load("TS9347_Texture_Character_Set_Datasheet.bmp", &width, &height, &nrChannels, 0);
	if (data){
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else error_callback(0,"Failed to load texture");
	stbi_image_free(data);
	return charset_texture;
}

void imguiInit(GLFWwindow* window){
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io=ImGui::GetIO();
	io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard;
	
	ImGui_ImplGlfw_InitForOpenGL(window,true);
	ImGui_ImplOpenGL3_Init();
}

void imguiStartFrame(){
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (show_imgui){
		ImGui::ShowDemoWindow(&show_imgui);
		imgui_main_menu_window(&show_imgui);
	}
	imgui_notification_window();
}

void imguiRender(){
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiShutdown(){
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void thread_circuit_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_video,thread_mailbox* p_mb_audio){
	
}

void thread_video_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_video){
	
	
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    GLFWwindow* window = glfwCreateWindow(640, 480, window_title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
	glfwSetWindowCloseCallback(window, window_close_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);
	
	//imgui init
	imguiInit(window);
 
    // NOTE: OpenGL error checks have been omitted for brevity
 
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
 
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
	GLint isCompiled = 0;
	GLint maxLength = 0;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE){
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);
		char errorLog[maxLength];
		glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);
		error_callback(0,"vertex shader");
		error_callback(0,(const char*)errorLog);
	}
 
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE){
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);
		char errorLog[maxLength];
		glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);
		error_callback(0,"fragment shader");
		error_callback(0,&errorLog[0]);
	}
 
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
 
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint charset_location = glGetUniformLocation(program, "charset");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vp_location = glGetUniformLocation(program, "vP");
    const GLint vcharp_location = glGetUniformLocation(program, "vCharP");
    const GLint double_location = glGetUniformLocation(program, "double");
    const GLint color_location = glGetUniformLocation(program, "color");
	fprintf(stdout,"%i %i %i %i %i %i\n",mvp_location,charset_location,vpos_location,vp_location,vcharp_location,double_location);
	
	GLuint charset_texture=loadCharacterSetTexture();
	glUniform1i(charset_location,0);
 
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, pos));
	
	glEnable(GL_SCISSOR_TEST);
	
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
		
		imguiStartFrame();
		
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
		glViewport(0,0,width,height);
		glScissor(0,0,width,height);
        glClear(GL_COLOR_BUFFER_BIT);
        GLfloat mvp[4][4]={{0,0,0,0},
						   {0,0,0,0},
						   {0,0,1,0},
						   {0,0,0,1}};
		float l2_1=width*screen_height;
		float l2_2=height*screen_width;
		if (l2_1<l2_2){
			mvp[1][1]=-l2_1/l2_2;
			mvp[0][0]=1;
		}
		else{
			mvp[1][1]=-1;
			mvp[0][0]=l2_2/l2_1;
		}
		glScissor( width*(1-mvp[0][0])/2.,
				   height*(1+mvp[1][1])/2.,
				   mvp[0][0]*width,
				   -mvp[1][1]*height);
		mvp[3][0]=-mvp[0][0];
		mvp[3][1]=-mvp[1][1];
		mvp[0][0]*=2/80.;
		mvp[1][1]*=2/25.;
 
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, charset_texture);
        glBindVertexArray(vertex_array);
		
		glUniform1ui(double_location,0);
		
		GLint vp[2]={0,0};
		GLint vcp[2]={0,0};
		
		do{
			glUniform1ui(color_location,(imgui_rgb_video?C_RGB:0)|(vp[0]&7)|(((~vp[0])&7)<<3));
			glUniform2iv(vp_location,1,(const GLint*)&vp);
			glUniform2iv(vcharp_location,1,(const GLint*)&vcp);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			
			vp[0]+=1;
			vp[1]+=1*(vp[0]/80);
			vp[0]=vp[0]%80;
			vcp[0]++;
			vcp[1]=(vcp[0]/128+vcp[1])%3;
			vcp[0]=vcp[0]%128;
		}
		while (vp[1]<25);
 
		imguiRender();
 
        glfwSwapBuffers(window);
    }
 
	imguiShutdown();
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
}

void thread_audio_main(thread_mailbox* p_mb_circuit,thread_mailbox* p_mb_audio){
	
}

int main(void)
{
	thread_mailbox mb_circuit;
	thread_mailbox mb_audio;
	thread_mailbox mb_video;
	std::thread thrd_v(thread_video_main,&mb_circuit,&mb_video);
	std::thread thrd_a(thread_audio_main,&mb_circuit,&mb_audio);
	thread_circuit_main(&mb_circuit,&mb_video,&mb_audio);
	
	thrd_a.join();
	thrd_v.join();
	
    exit(EXIT_SUCCESS);
}