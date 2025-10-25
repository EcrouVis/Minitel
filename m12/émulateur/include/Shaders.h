#ifndef SHADERS_H
#define SHADERS_H

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
"#version 330\n\
uniform mat4 MVP;\n\
uniform ivec2 vP;\n\
uniform ivec2 vCharP;\n\
uniform uint chr_dbl;\n\
in vec2 vPos;\n\
out vec2 coord_texture;\n\
const float mul[4]=float[](1.,2.,2.,4.);\n\
void main(){\n\
	vec2 p_screen=vPos*vec2(mul[(chr_dbl<<30)>>30],mul[chr_dbl>>2])+vec2(vP);\n\
    gl_Position = MVP*vec4(p_screen, 0.0, 1.0);\n\
    coord_texture=vec2(bool((chr_dbl<<31)>>31)?1.:0.75,1)*vPos-vec2(0,(bool(chr_dbl>>3)&&(vCharP.y==0))?0.05:0);\n\
}\n";
 
static const char* fragment_shader_text =
"#version 330\n\
in vec2 coord_texture;\n\
out vec4 fragment;\n\
uniform ivec2 vCharP;\n\
uniform uint chr_clr;\n\
uniform sampler2D charset;\n\
const float grey[8]=float[](0.,2/7.,4/7.,6/7.,1/7.,3/7.,5/7.,1.);\n\
void main(){\n\
    vec2 ct=(coord_texture+vec2(vCharP))*vec2(8,10);\n\
	bool underline=bool((chr_clr<<25)>>31);\n\
	bool conceal=bool((chr_clr<<24)>>31);\n\
	bool rgb=bool((chr_clr<<23)>>31);\n\
    bool fg=(((coord_texture.t>0.9)&&underline)?1.:texture(charset,ct/vec2(textureSize(charset,0))).r)>0.5;\n\
    fg=fg&&(!conceal);\n\
	uint c=(chr_clr<<(fg?29:26))>>29;\n\
	vec3 color;\n\
	if (rgb){\n\
		color=vec3((c<<31)>>31,(c<<30)>>31,(c<<29)>>31);\n\
	}\n\
	else{\n\
		color=vec3(1,1,1)*grey[c];\n\
	}\n\
	fragment=vec4(color,1);\n\
}\n";
#endif