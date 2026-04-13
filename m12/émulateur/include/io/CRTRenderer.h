#ifndef VIDEO_H
#define VIDEO_H
#include "Parameters.h"
#include "circuit/CRTBuffer.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <cstdio>
#include <cmath>
 


class CRTRenderer{
	public:
		CRTRenderer(GLFWwindow* window,Parameters* p_PARAMETERS){
			this->p_PARAMETERS=p_PARAMETERS;
			this->window=window;
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
			
			glGenVertexArrays(1,&(this->VertexArrayID));
			glBindVertexArray(this->VertexArrayID);
			
			glGenBuffers(1,&(this->VertexBufferID));
			glBindBuffer(GL_ARRAY_BUFFER,this->VertexBufferID);
			
			glBufferData(GL_ARRAY_BUFFER,sizeof(this->VertexBufferData),this->VertexBufferData,GL_STATIC_DRAW);
			
			this->loadShaders();
			
			this->TextureID=glGetUniformLocation(this->ProgramID,"textureSampler");
			glGenTextures(1,&(this->TextureID));
			
			this->ScaleID=glGetUniformLocation(this->ProgramID,"scale");
			
			glClearColor(0,0,0,0);
		}
		void setBuffer(CRTBuffer* p_buffer){
			this->p_buffer=p_buffer;
		}
		void render(){
			if (this->p_buffer!=NULL){
				glUseProgram(this->ProgramID);
				
				glActiveTexture(GL_TEXTURE0);
				this->updateTexture();
				glUniform1i(this->TextureID,0);
				
				int width, height;
				glfwGetFramebufferSize(this->window, &width, &height);
				GLfloat scale[2];
				float l2_1=width*this->p_PARAMETERS->io.crt.height;
				float l2_2=height*this->p_PARAMETERS->io.crt.width*this->p_PARAMETERS->io.crt.width_factor;
				if (l2_1<l2_2){
					scale[0]=0.5;
					scale[1]=-0.5*l2_2/l2_1;
				}
				else{
					scale[0]=0.5*l2_1/l2_2;
					scale[1]=-0.5;
				}
				glUniform2fv(this->ScaleID,1,scale);
				
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER,this->VertexBufferID);
				glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,(void*)0);
				
				glDrawArrays(GL_TRIANGLES,0,6);
				glDisableVertexAttribArray(0);
			}
		}
	private:
		bool current_rgb=false;
	
		CRTBuffer* p_buffer=NULL;
		GLFWwindow* window;
		Parameters* p_PARAMETERS;
		GLuint VertexArrayID;
		GLuint VertexBufferID;
		static constexpr GLfloat VertexBufferData[12]={
			-1, -1,
			 1, -1,
			 1,  1,
			 1,  1,
			-1,  1,
			-1, -1,
		};
		
		const char* VS="\
#version 330 core\n\
layout(location=0) in vec2 vertexPosition;\n\
out vec2 UV;\n\
uniform vec2 scale;\n\
void main(){\n\
	gl_Position.xy=vertexPosition;\n\
	gl_Position.zw=vec2(0.,1.);\n\
	UV=scale*vertexPosition+vec2(0.5,0.5);\n\
}\n\
			";
		GLuint VertexShaderID;
		const char* FS="\
#version 330 core\n\
in vec2 UV;\n\
out vec4 color;\n\
uniform sampler2D textureSampler;\n\
void main(){\n\
	color=texture(textureSampler,UV);\n\
}\n\
			";
		GLuint FragmentShaderID;
		GLuint ProgramID;
		
		GLuint TextureID;
		GLuint ScaleID;
		
		void loadShaders(){
			this->VertexShaderID=glCreateShader(GL_VERTEX_SHADER);
			this->FragmentShaderID=glCreateShader(GL_FRAGMENT_SHADER);
			
			GLint R=GL_FALSE;
			int Ilength=0;
			char* info=NULL;
			
			glShaderSource(this->VertexShaderID,1,&this->VS,NULL);
			glCompileShader(this->VertexShaderID);
			glGetShaderiv(this->VertexShaderID, GL_COMPILE_STATUS, &R);
			glGetShaderiv(this->VertexShaderID, GL_INFO_LOG_LENGTH, &Ilength);
			if (Ilength>0){
				info=(char*)malloc(Ilength+1);
				glGetShaderInfoLog(this->VertexShaderID,Ilength,NULL,info);
				printf("Vertex shader: %s\n",info);
				free((void*)info);
				info=NULL;
			}
			
			glShaderSource(this->FragmentShaderID,1,&this->FS,NULL);
			glCompileShader(this->FragmentShaderID);
			glGetShaderiv(this->FragmentShaderID, GL_COMPILE_STATUS, &R);
			glGetShaderiv(this->FragmentShaderID, GL_INFO_LOG_LENGTH, &Ilength);
			if (Ilength>0){
				info=(char*)malloc(Ilength+1);
				glGetShaderInfoLog(this->FragmentShaderID,Ilength,NULL,info);
				printf("Fragment shader: %s\n",info);
				free((void*)info);
				info=NULL;
			}
			
			this->ProgramID=glCreateProgram();
			glAttachShader(this->ProgramID,this->VertexShaderID);
			glAttachShader(this->ProgramID,this->FragmentShaderID);
			glLinkProgram(this->ProgramID);
			glGetProgramiv(this->ProgramID, GL_COMPILE_STATUS, &R);
			glGetProgramiv(this->ProgramID, GL_INFO_LOG_LENGTH, &Ilength);
			if (Ilength>0){
				info=(char*)malloc(Ilength+1);
				glGetProgramInfoLog(this->ProgramID,Ilength,NULL,info);
				printf("Program: %s\n",info);
				free((void*)info);
				info=NULL;
			}
			
			glDetachShader(this->ProgramID,this->VertexShaderID);
			glDetachShader(this->ProgramID,this->FragmentShaderID);
			glDeleteShader(this->VertexShaderID);
			glDeleteShader(this->FragmentShaderID);
		}
		void updateTexture(){
			if (this->p_buffer->frameChanged()||(this->current_rgb!=this->p_PARAMETERS->io.crt.rgb)){
				this->current_rgb=this->p_PARAMETERS->io.crt.rgb;
				static unsigned char crtbuffer_data[VIDEO_FRAME_SIZE];
				this->p_buffer->getVideoFrame(crtbuffer_data);
				glBindTexture(GL_TEXTURE_2D,this->TextureID);
				static unsigned char data[(250+2)*(3*40*8+2)*4];
				const unsigned char l[]={0,102,179,204,128,153,230,255};
				if (this->p_PARAMETERS->io.crt.rgb){
					for (int i=0;i<(250+2)*(3*40*8+2);i++){
						data[4*i]=crtbuffer_data[i>>1];
						if ((bool)(i&1)) data[4*i]&=0x0F;
						else data[4*i]=data[4*i]>>4;
						// 0000 ABGR
						data[4*i+3]=(data[4*i]&0x08)?0xFF:0;
						data[4*i+2]=(data[4*i]&0x04)?0xFF:0;
						data[4*i+1]=(data[4*i]&0x02)?0xFF:0;
						data[4*i]=(data[4*i]&0x01)?0xFF:0;
					}
				}
				else{
					for (int i=0;i<(250+2)*(3*40*8+2);i++){
						data[4*i]=crtbuffer_data[i>>1];
						if ((bool)(i&1)) data[4*i]&=0x0F;
						else data[4*i]=data[4*i]>>4;
						data[4*i+3]=(data[4*i]&0x08)?0xFF:0;
						data[4*i]=l[data[4*i]&0x07];
						data[4*i+1]=data[4*i];
						data[4*i+2]=data[4*i];
					}
				}
				glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,3*40*8+2,250+2,0, GL_RGBA, GL_UNSIGNED_BYTE,data);
				/*unsigned char data[(250+2)*(3*40*8+2)];
				for (int i=0;i<(250+2)*(3*40*8+2);i++) data[i]=this->p_buffer->VIDEO_OUTPUT[i].load(std::memory_order_acquire);
				glTexImage2D(GL_TEXTURE_2D,0,GL_RED,3*40*8+2,250+2,0, GL_RED, GL_UNSIGNED_BYTE,data);*/
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			}
		}
};

#endif