#include "shader.h"

char* Shader::textFileRead(const char *fileName)
{
	char* text = NULL;

	if (fileName != NULL) 
	{
		FILE *file; 
		fopen_s(&file, fileName, "r");

		if (file != NULL) 
		{
			fseek(file, 0, SEEK_END);
			int count = ftell(file);
			rewind(file);

			if (count > 0) 
			{
				text = (char*)malloc(sizeof(char) * (count + 1));
				count = fread(text, sizeof(char), count, file);
				text[count] = '\0';
			}
			fclose(file);
		}
	}
	return text;
}

void Shader::validateShader(GLuint shader, const char* file = NULL)
{
	char buffer[512];
	GLsizei buffer_len = 0;

	memset(buffer, 0, 512);
	glGetShaderInfoLog(shader, 512, &buffer_len, buffer);
	if (buffer_len > 0)
		cerr << "Shader " << shader << " (" << (file ? file : "") << ") compile error: " << buffer << endl;
}

void Shader::validateProgram(GLuint program)
{
	char buffer[512];
	GLsizei buffer_len = 0;

	memset(buffer, 0, 512);
	glGetProgramInfoLog(program, 512, &buffer_len, buffer);
	if (buffer_len > 0)
		cerr << "Program " << program << " link error: " << buffer << endl;

	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
		cerr << "Error validating shader " << program << endl;
}

Shader::Shader() {}

Shader::Shader(const char *vsFile, const char *fsFile) 
{
	init(vsFile, fsFile);
}

void Shader::init(const char *vsFile, const char *fsFile) 
{
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vsText = textFileRead(vsFile);
	const char* fsText = textFileRead(fsFile);

	if (vsText == NULL || fsText == NULL) 
	{
		cerr << "Either vertex shader or fragment shader file not found." << endl;
		return;
	}

	glShaderSource(v, 1, &vsText, 0);
	glShaderSource(f, 1, &fsText, 0);

	glCompileShader(v);
	validateShader(v, vsFile);
	glCompileShader(f);
	validateShader(f, fsFile);

	p = glCreateProgram();
	glAttachShader(p, f);
	glAttachShader(p, v);
	glLinkProgram(p);
	validateProgram(p);
}

Shader::~Shader() 
{
	glDetachShader(p, f);
	glDetachShader(p, v);
	
	glDeleteShader(f);
	glDeleteShader(v);
	glDeleteProgram(p);
}

unsigned int Shader::id() 
{
	return p;
}

void Shader::bind() 
{
	glUseProgram(p);
}

void Shader::unbind() 
{
	glUseProgram(0);
}
