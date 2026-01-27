#ifndef VIDEO_H
#define VIDEO_H
#include "Parameters.h"
#include "circuit/TS9347.h"
#include "Shaders.h"

#define GLAD_GL_IMPLEMENTATION
#include "glad/glad.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstdio>
#include <cmath>

struct Vertex{
    float pos[2];
};
 
static const Vertex squareVertices[6]={
    {{0.f,0.f}},
    {{1.f,0.f}},
    {{1.f,1.f}},
    {{1.f,1.f}},
    {{0.f,1.f}},
    {{0.f,0.f}}
};

class TS9347Renderer{
	public:
		TS9347Renderer(GLFWwindow* window,Parameters* p_PARAMETERS){
			this->p_PARAMETERS=p_PARAMETERS;
			this->window=window;
			
			glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
			
			this->loadTS9347Shaders();
			
			this->mvp_location = glGetUniformLocation(program, "MVP");
    		this->charset_location = glGetUniformLocation(program, "charset");
    		this->vpos_location = glGetAttribLocation(program, "vPos");
    		this->vp_location = glGetUniformLocation(program, "vP");
    		this->vcharp_location = glGetUniformLocation(program, "vCharP");
    		this->double_location = glGetUniformLocation(program, "chr_dbl");
    		this->color_location = glGetUniformLocation(program, "chr_clr");
			const char* path="./ressources/TS9347_Texture_Character_Set_Datasheet.bmp";
			this->loadCharacterSetTexture(path);
			glUniform1i(this->charset_location,0);
			
			glGenVertexArrays(1, &this->vertex_array);
			glBindVertexArray(this->vertex_array);
			glEnableVertexAttribArray(this->vpos_location);
			glVertexAttribPointer(this->vpos_location, 2, GL_FLOAT, GL_FALSE,
								  sizeof(Vertex), (void*) offsetof(Vertex, pos));
			
			glEnable(GL_SCISSOR_TEST);
		}
		void setIC(TS9347wVRAM* p_ic){ this->p_ic=p_ic;}
		void render(){
			//unsigned char bgr2rgb_3bit[8]={0,4,2,6,1,5,3,7};
			int width, height;
			glfwGetFramebufferSize(this->window, &width, &height);
			GLfloat mvp[4][4]={{0,0,0,0},
							   {0,0,0,0},
							   {0,0,1,0},
							   {0,0,0,1}};
			float l2_1=width*this->p_PARAMETERS->io.crt.height;
			float l2_2=height*this->p_PARAMETERS->io.crt.width*this->p_PARAMETERS->io.crt.width_factor;
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
	 
			glUseProgram(this->program);
			glUniformMatrix4fv(this->mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->charset_texture);
			glBindVertexArray(this->vertex_array);
			
			
			if (this->p_ic!=NULL){
				unsigned char tgs=this->p_ic->TGS.load(std::memory_order_relaxed);
				unsigned char display_mode=tgs>>6;
				bool srow_low=(bool)(tgs&1);
				unsigned char pat=this->p_ic->PAT.load(std::memory_order_relaxed);
				bool srow_enable=(bool)(pat&1);
				bool blk_enable=(bool)(pat&2);
				bool conceal_enable=(bool)(pat&8);
				bool flash_enable=(bool)(pat&0x40);
				unsigned char mat=this->p_ic->MAT.load(std::memory_order_relaxed);
				unsigned char cursor_mode=(mat>>4)&3;
				bool cursor_enable=(bool)(mat&0x40);
				unsigned char cursor_pos[2];
				cursor_pos[0]=this->p_ic->Rx[6].load(std::memory_order_relaxed)&0x1F;//Y
				cursor_pos[1]=this->p_ic->Rx[7].load(std::memory_order_relaxed)&0x3F;//X
				bool double_height=(bool)(mat&0x80);
				unsigned char dor=this->p_ic->DOR.load(std::memory_order_relaxed);
				unsigned char ror=this->p_ic->ROR.load(std::memory_order_relaxed);
				
				AddressDecomposition addr_srow;
				addr_srow.District=(((bool)(dor&0x80))?4:0)|(((bool)(ror&0x80))?2:0)|(((bool)(ror&0x20))?1:0);
				addr_srow.Block=((bool)(ror&0x40))?2:0;
				addr_srow.X=0;
				addr_srow.Y=0;
				
				AddressDecomposition addr_blk=addr_srow;
				addr_blk.Y=ror&0x1F;
				
				//display_mode=3;///////////////////////////////////////////////////////////////////////////////////////////////////////////////
				
				unsigned int double_base=((bool)(display_mode&2)?0:S_40CPL);
				unsigned int color_base=this->p_PARAMETERS->io.crt.rgb?C_RGB:0;
				
				//srow_enable=true;/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//blk_enable=true;/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
				
				if (srow_enable){
					GLint char_pos[2]={0,srow_low?24:0};
					GLint char_index[2]={0,0};
					if (display_mode<2){
						unsigned char C=0;
						unsigned char B=0;
						unsigned char A=0;
						for (int i=0;i<40;i++){
							addr_srow.X=i;
							if (display_mode==0){//40 char long
								C=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
								addr_srow.Block=(addr_srow.Block+1)&3;
								B=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
								addr_srow.Block=(addr_srow.Block+1)&3;
								A=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
								addr_srow.Block=(addr_srow.Block+2)&3;
							}
							else if (display_mode==1){//40 char short
								unsigned char tmp=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
								addr_srow.Block=(addr_srow.Block+1)&3;
								C=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
								addr_srow.Block=(addr_srow.Block+3)&3;
								B&=0x55;
								B|=(C&0x80);
								B|=(tmp&0x80)>>2;
								
								A&=0x07;
								A|=(tmp&7)<<4;//C1
								A|=(tmp&0x08);//F
								if ((bool)(tmp&0x80)){
									A&=0xF8;
									A|=(tmp&0x70)>>4;//C0
								}
								else{
									A|=(tmp&0x40)<<1;//N
									B|=(tmp&0x20)>>2;//L
									B|=(tmp&0x10)>>3;//H
								}
								
								if ((C&0xE0)==0x80){//DEL
									B&=0x20;
									B|=(C&2)>>1;//I1
									B|=(C&1)>>2;//m
									B|=(C&4)<<2;//U
									B|=(C&8)<<3;//I2
									A&=0xF7;//F
									A|=0x80;//N
									C=0;
								}
								else if ((bool)(tmp&0x80)){
									B&=0xEF;//U
								}
							}
							
							unsigned char fg_color=(A&0x70)>>4;//bgr2rgb_3bit[(A&0x70)>>4];
							unsigned char bg_color=A&0x07;//bgr2rgb_3bit[A&0x07];
							bool dbl_h=(bool)(B&2);
							bool dbl_w=false;
							bool conceal=(bool)(B&4);
							bool underline=false;
							bool flash=(bool)(A&8);
							bool on_cursor=(addr_srow.X==cursor_pos[1])&&(addr_srow.Y==cursor_pos[0]);
							bool neg=(bool)(A&0x80);
							
							if (neg){//color inverted
								unsigned char tmp=fg_color;
								fg_color=bg_color;
								bg_color=tmp;
							}
							
							
							if (cursor_enable&&((bool)(cursor_mode&1))&&on_cursor) underline=true;
							if (cursor_enable&&(!(bool)(cursor_mode&1))&&on_cursor){//complement
								fg_color^=0x07;
								bg_color^=0x07;
							}
							if (flash_enable&&(flash||(cursor_enable&&((bool)(cursor_mode&2))&&on_cursor))){//flash
								double t_r;
								if (modf(glfwGetTime(),&t_r)>0.5){
								fg_color^=0x07;
								bg_color^=0x07;
								}
							}
							
							char_pos[0]=i*2;
							if(!(bool)(B&0x80)){//rom char
								
								if((bool)(B&0x20)){
									if((bool)(B&0x10)){//GOE
										char_index[1]=2;
									}
									else{//G10
										char_index[1]=1;
									}
								}
								else{//G0
									char_index[1]=0;
									underline=(bool)(B&0x10);
								}
								char_index[0]=C&0x7F;
								
								dbl_w=(bool)(B&8);
							}
							
							if(!(bool)(B&0x80)){///wip
								glUniform1ui(this->double_location,double_base|(dbl_w?S_DOUBLE_CHAR_WIDTH:0)|(dbl_h?S_DOUBLE_CHAR_HEIGHT:0));
								glUniform1ui(this->color_location,color_base|(fg_color)|(bg_color<<3)|((conceal_enable&&conceal)?C_CONCEAL:0)|(underline?C_UNDERLINE:0));
								glUniform2iv(this->vp_location,1,(const GLint*)&char_pos);
								glUniform2iv(this->vcharp_location,1,(const GLint*)&char_index);
								glDrawArrays(GL_TRIANGLES, 0, 6);
								
							}
						}
					}
					else{
						for(int i=0;i<80;i++){
							addr_srow.Block=(addr_srow.Block&2)+(i&1);
							addr_srow.X=i>>1;
							
							unsigned char A=0;
							unsigned char C=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed);
							if(display_mode!=2){//80 char long
								addr_srow.Block=(addr_srow.Block&2)^2;
								if((bool)(i&1)) A=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed)&0x0F;
								else A=this->p_ic->VRAM[address2PAddress(&addr_srow)].load(std::memory_order_relaxed)>>4;
								addr_srow.Block^=2;
							}
							
							char_index[0]=C&0x7F;
							if ((bool)(C&0x80)){
								char_index[1]=2;
							}
							else{
								char_index[1]=0;
							}
							
							char_pos[0]=i;
							
							unsigned char fg_color=(dor>>((A&1)<<2))&0x07;//bgr2rgb_3bit[(dor>>((A&1)<<2))&0x07];//D
							unsigned char bg_color=mat&0x07;//bgr2rgb_3bit[mat&0x07];
							if ((bool)(A&0x08)){//N
								unsigned char tmp=fg_color;
								fg_color=bg_color;
								bg_color=tmp;
							}
							if ((bool)(A&0x04)){//F
								fg_color=bg_color;
							}
							bool underline=(bool)(A&0x02);//U
							
							glUniform1ui(this->double_location,double_base);
							glUniform1ui(this->color_location,color_base|(fg_color)|(bg_color<<3)|(underline?C_UNDERLINE:0));
							glUniform2iv(this->vp_location,1,(const GLint*)&char_pos);
							glUniform2iv(this->vcharp_location,1,(const GLint*)&char_index);
							glDrawArrays(GL_TRIANGLES, 0, 6);
							
						}
					}
				}
				double_base|=(double_height?S_DOUBLE_HEIGHT:0);
				if (blk_enable){
					if (display_mode<2){
						for(int j=0;j<24;j++){//j++////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							GLint char_pos[2]={0,j+(srow_low?0:1)};
							GLint char_index[2]={0,0};
							unsigned char C=0;
							unsigned char B=0;
							unsigned char A=0;
							for (int i=0;i<40;i++){
								addr_blk.X=i;
								if (display_mode==0){//40 char long
									C=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
									addr_blk.Block=(addr_blk.Block+1)&3;
									B=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
									addr_blk.Block=(addr_blk.Block+1)&3;
									A=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
									addr_blk.Block=(addr_blk.Block+2)&3;
								}
								else if (display_mode==1){//40 char short
									unsigned char tmp=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
									addr_blk.Block=(addr_blk.Block+1)&3;
									C=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
									addr_blk.Block=(addr_blk.Block+3)&3;
									B&=0x55;
									B|=(C&0x80);
									B|=(tmp&0x80)>>2;
									
									A&=0x07;
									A|=(tmp&7)<<4;//C1
									A|=(tmp&0x08);//F
									if ((bool)(tmp&0x80)){
										A&=0xF8;
										A|=(tmp&0x70)>>4;//C0
									}
									else{
										A|=(tmp&0x40)<<1;//N
										B|=(tmp&0x20)>>2;//L
										B|=(tmp&0x10)>>3;//H
									}
									
									if ((C&0xE0)==0x80){//DEL
										B&=0x20;
										B|=(C&2)>>1;//I1
										B|=(C&1)>>2;//m
										B|=(C&4)<<2;//U
										B|=(C&8)<<3;//I2
										A&=0xF7;//F
										A|=0x80;//N
										C=0;
									}
									else if ((bool)(tmp&0x80)){
										B&=0xEF;//U
									}
								}
								
								unsigned char fg_color=(A&0x70)>>4;//bgr2rgb_3bit[(A&0x70)>>4];
								unsigned char bg_color=A&0x07;//bgr2rgb_3bit[A&0x07];
								bool dbl_h=(bool)(B&2);
								bool dbl_w=false;
								bool conceal=(bool)(B&4);
								bool underline=false;
								bool flash=(bool)(A&8);
								bool on_cursor=(addr_blk.X==cursor_pos[1])&&(addr_blk.Y==cursor_pos[0]);
								bool neg=(bool)(A&0x80);
								
								if (neg){//color inverted
									unsigned char tmp=fg_color;
									fg_color=bg_color;
									bg_color=tmp;
								}
								
								
								if (cursor_enable&&((bool)(cursor_mode&1))&&on_cursor) underline=true;
								if (cursor_enable&&(!(bool)(cursor_mode&1))&&on_cursor){//complement
									fg_color^=0x07;
									bg_color^=0x07;
								}
								if (flash_enable&&(flash||(cursor_enable&&((bool)(cursor_mode&2))&&on_cursor))){//flash
									double t_r;
									if (modf(glfwGetTime(),&t_r)>0.5){
									fg_color^=0x07;
									bg_color^=0x07;
									}
								}
								
								char_pos[0]=i*(((bool)(display_mode&2))?1:2);
								if(!(bool)(B&0x80)){//rom char
									
									if((bool)(B&0x20)){
										if((bool)(B&0x10)){//GOE
											char_index[1]=2;
										}
										else{//G10
											char_index[1]=1;
										}
									}
									else{//G0
										char_index[1]=0;
										underline=(bool)(B&0x10);
									}
									char_index[0]=C&0x7F;
									
									dbl_w=(bool)(B&8);
								}
								
								if(!(bool)(B&0x80)){///wip
									glUniform1ui(this->double_location,double_base|(dbl_w?S_DOUBLE_CHAR_WIDTH:0)|(dbl_h?S_DOUBLE_CHAR_HEIGHT:0));
									glUniform1ui(this->color_location,color_base|(fg_color)|(bg_color<<3)|((conceal_enable&&conceal)?C_CONCEAL:0)|(underline?C_UNDERLINE:0));
									glUniform2iv(this->vp_location,1,(const GLint*)&char_pos);
									glUniform2iv(this->vcharp_location,1,(const GLint*)&char_index);
									glDrawArrays(GL_TRIANGLES, 0, 6);
									
								}
							}
							
							addr_blk.Y++;
							if (addr_blk.Y>=32) addr_blk.Y=8;
						}
					}
					else{
						GLint char_index[2]={0,0};
						for(int j=0;j<24;j++){//j++////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							GLint char_pos[2]={0,j+(srow_low?0:1)};
							
							for(int i=0;i<80;i++){
								addr_blk.Block=(addr_blk.Block&2)+(i&1);
								addr_blk.X=i>>1;
								
								unsigned char A=0;
								unsigned char C=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed);
								if(display_mode!=2){//80 char long
									addr_blk.Block=(addr_blk.Block&2)^2;
									if((bool)(i&1)) A=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed)&0x0F;
									else A=this->p_ic->VRAM[address2PAddress(&addr_blk)].load(std::memory_order_relaxed)>>4;
									addr_blk.Block^=2;
								}
								
								char_index[0]=C&0x7F;
								if ((bool)(C&0x80)){
									char_index[1]=2;
								}
								else{
									char_index[1]=0;
								}
								
								char_pos[0]=i;
								
								unsigned char fg_color=(dor>>((A&1)<<2))&0x07;//bgr2rgb_3bit[(dor>>((A&1)<<2))&0x07];//D
								unsigned char bg_color=mat&0x07;//bgr2rgb_3bit[mat&0x07];
								if ((bool)(A&0x08)){//N
									unsigned char tmp=fg_color;
									fg_color=bg_color;
									bg_color=tmp;
								}
								if ((bool)(A&0x04)){//F
									fg_color=bg_color;
								}
								bool underline=(bool)(A&0x02);//U
								
								glUniform1ui(this->double_location,double_base);
								glUniform1ui(this->color_location,color_base|(fg_color)|(bg_color<<3)|(underline?C_UNDERLINE:0));
								glUniform2iv(this->vp_location,1,(const GLint*)&char_pos);
								glUniform2iv(this->vcharp_location,1,(const GLint*)&char_index);
								glDrawArrays(GL_TRIANGLES, 0, 6);
								
							}
							addr_blk.Y++;
							if (addr_blk.Y>=32) addr_blk.Y=8;
							
						}
					}
				}
			}
			
			/*GLint vp[2]={0,0};
			GLint vcp[2]={0,0};
			
			do{
				glUniform1ui(this->color_location,(this->p_PARAMETERS->io.crt.rgb?C_RGB:0)|(vp[0]&7)|(((~vp[0])&7)<<3));
				glUniform2iv(this->vp_location,1,(const GLint*)&vp);
				glUniform2iv(this->vcharp_location,1,(const GLint*)&vcp);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				
				vp[0]+=2;
				vp[1]+=1*(vp[0]/80);
				vp[0]=vp[0]%80;
				vcp[0]++;
				vcp[1]=(vcp[0]/128+vcp[1])%3;
				vcp[0]=vcp[0]%128;
			}
			while (vp[1]<25);*/
			
			glScissor(0,0,width,height);
		}
	private:
		TS9347wVRAM* p_ic=NULL;
		GLFWwindow* window;
		Parameters* p_PARAMETERS;
		GLuint vertex_array;
		GLuint charset_texture;
		GLuint program;
		GLint mvp_location;
		GLint vp_location;
		GLint charset_location;
		GLint vpos_location;
		GLint vcharp_location;
		GLint double_location;
		GLint color_location;
		
		void loadCharacterSetTexture(const char* path){
			glGenTextures(1, &this->charset_texture);
			glBindTexture(GL_TEXTURE_2D, this->charset_texture);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			
			int width, height, nrChannels;
			//stbi_set_flip_vertically_on_load(true);
			unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
			if (data){
				//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else printf("Failed to load texture");
			stbi_image_free(data);
		}
		void loadTS9347Shaders(){
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
				printf("vertex shader");
				printf((const char*)errorLog);
				fprintf(stderr,vertex_shader_text);
			}
		 
			const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &isCompiled);
			if(isCompiled == GL_FALSE){
				glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);
				char errorLog[maxLength];
				glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);
				printf("fragment shader");
				printf(&errorLog[0]);
				fprintf(stderr,fragment_shader_text);
			}
		 
			this->program = glCreateProgram();
			glAttachShader(this->program, vertex_shader);
			glAttachShader(this->program, fragment_shader);
			glLinkProgram(this->program);
		}
	
};

#endif