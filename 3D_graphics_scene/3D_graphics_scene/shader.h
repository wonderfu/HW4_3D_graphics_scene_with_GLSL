#ifndef _SHADER_H
#define _SHADER_H

#include "GL/glew.h"
#include <gl/glaux.h>
#include <GL/glut.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

class Shader
{
public:
	Shader();
	Shader(const char*, const char*);
	~Shader();
	
	void init(const char*, const char*);
	unsigned int id();
	void bind();
	void unbind();

private:
	GLuint shader_vp, shader_fp, shader_id;
};

char* textFileRead(const char*);
void validateShader(GLuint, const char*);
void validateProgram(GLuint);

#endif