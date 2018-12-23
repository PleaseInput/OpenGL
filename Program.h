#pragma once
// #include "Common.h"
#include <vector>

using namespace glm;
using namespace std;

char** loadShaderSource(const char* file);
void freeShaderSource(char** srcp);

class Program
{
	public:
		// the program ID
		GLuint program_ID;

		// set program
		void set_program(const char *vertex_path, const char *fragment_path);

		// activate the program
		void use();

		// set uniform variables
		void set_bool(const string &variable_name, bool value);
		void set_int(const string &variable_name, int value);
		void set_float(const string &variable_name, float value);
		void set_mat4(const string &variable_name, mat4 matrix);
};
void Program::set_program(const char *vertex_path, const char *fragment_path)
{
	// Create Shader Program
	program_ID = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vsSource = loadShaderSource(vertex_path);
	char** fsSource = loadShaderSource(fragment_path);

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vs, 1, vsSource, NULL);
	glShaderSource(fs, 1, fsSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vsSource);
	freeShaderSource(fsSource);

	// Compile these shaders
	glCompileShader(vs);
	glCompileShader(fs);

	// Assign the program we created before with these shaders
	glAttachShader(program_ID, vs);
	glAttachShader(program_ID, fs);
	glLinkProgram(program_ID);
}

void Program::use()
{
	glUseProgram(program_ID);
}

void Program::set_bool(const string &variable_name, bool value)
{	
	glUniform1i(glGetUniformLocation(program_ID, variable_name.c_str()), (int)value);
}
void Program::set_int(const string &variable_name, int value)
{
	glUniform1i(glGetUniformLocation(program_ID, variable_name.c_str()), value);
}
void Program::set_float(const string &variable_name, float value)
{
	glUniform1f(glGetUniformLocation(program_ID, variable_name.c_str()), value);
}
void Program::set_mat4(const string &variable_name, mat4 matrix)
{
	glUniformMatrix4fv(glGetUniformLocation(program_ID, variable_name.c_str()), 1, GL_FALSE, value_ptr(matrix));
}

// Load shader file to program
char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

// Free shader file
void freeShaderSource(char** srcp)
{
	delete srcp[0];
	delete srcp;
}
