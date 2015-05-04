#ifndef _SHADER_H
#define _SHADER_H

#include "GL/glew.h"
#include <gl/glaux.h>
#include <GL/glut.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>

using namespace std;

class Shader
{
public:
	Shader();
	Shader(const char*, const char*);
	~Shader();
	
	char* textFileRead(const char*);
	void validateShader(GLuint, const char*);
	void validateProgram(GLuint);
	void init(const char*, const char*);
	unsigned int id();
	void bind();
	void unbind();

private:
	GLuint v = 0;
	GLuint f = 0;
	GLuint p = 0;
};



#endif