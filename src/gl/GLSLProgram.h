#pragma once
#include <string>
#include <glad\glad.h>

namespace UpiEngine {
	class GLSLProgram
	{
	public:
		GLSLProgram();
		~GLSLProgram();

		void compileShaders(const std::string &vertexShaderFilePath, const std::string &fragmentShaderFilePath);
		void compileShadersFromSource(const char* vertexSource, const char* fragmentSource);

		bool linkShaders();

		void addAttribute(const std::string &attributeName);

		GLint getUniformLocation(const std::string &uniformName);
		void use();
		void unuse();

		void dispose();

	private:
		
		void compileShader(const char* source, const std::string& name, GLuint id);


		int _numAttributes;
		GLuint _programID;

		GLuint _vertexShaderID;
		GLuint _fragmentShaderID;
	};

}