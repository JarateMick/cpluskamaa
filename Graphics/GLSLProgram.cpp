
#include "GLSLProgram.h"
// #include "UpiEngineErrors.h"
#include <fstream>
#include <vector>
#include "IOManager.h"

namespace UpiEngine{

	GLSLProgram::GLSLProgram() : _numAttributes(0), _programID(0), _vertexShaderID(0), _fragmentShaderID(0)
	{

	}

	GLSLProgram::~GLSLProgram()
	{
	}

	void GLSLProgram::compileShaders(const std::string &vertexShaderFilePath, const std::string &fragmentShaderFilePath)
	{
		std::string vertSource;
		std::string fragSource;

		IOManager::readFileToBuffer(vertexShaderFilePath, vertSource);
		IOManager::readFileToBuffer(fragmentShaderFilePath, fragSource);

		compileShadersFromSource(vertSource.c_str(), fragSource.c_str());
	}

	void GLSLProgram::compileShadersFromSource(const char* vertexSource, const char* fragmentSource)
	{
		//Vertex and fragment shaders are successfully compiled.
		//Now time to link them together into a program.
		//Get a program object.
		_programID = glCreateProgram();

		_vertexShaderID = glCreateShader(GL_VERTEX_SHADER);

		if (_vertexShaderID == 0) {
			// fatalError("Vertex shader failed to be created!");
			printf("Vertex shader failed to create");
		}

		_fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		if (_fragmentShaderID == 0) {
			// fatalError("Fragment shader failed to be created!");
			printf("Fragment shader failed to create");
		}

		compileShader(vertexSource, "Vertex Shader", _vertexShaderID);
		compileShader(fragmentSource, "Fragment Shader", _fragmentShaderID);
	}

	bool GLSLProgram::linkShaders()
	{

		//Attach our shaders to our program
		glAttachShader(_programID, _vertexShaderID);
		glAttachShader(_programID, _fragmentShaderID);

		//Link our program
		glLinkProgram(_programID);

		//Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(_programID, GL_LINK_STATUS, (int *)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(_programID, GL_INFO_LOG_LENGTH, &maxLength);

			//The maxLength includes the NULL character
			std::vector<char> errorLog(maxLength);
			glGetProgramInfoLog(_programID, maxLength, &maxLength, &errorLog[0]);

			//We don't need the program anymore.
			glDeleteProgram(_programID);
			//Don't leak shaders either.
			glDeleteShader(_vertexShaderID);
			glDeleteShader(_fragmentShaderID);

			std::printf("%s\n", &(errorLog[0]));

			// fatalError("shaders failed to link!");
			printf("shaders failed to link!");
			return false;
		}

		//Always detach shaders after a successful link.
		glDetachShader(_programID, _vertexShaderID);
		glDetachShader(_programID, _fragmentShaderID);
		glDeleteShader(_vertexShaderID);
		glDeleteShader(_fragmentShaderID);
		return true;
	}

	void GLSLProgram::addAttribute(const std::string &attributeName)
	{
		glBindAttribLocation(_programID, _numAttributes++, attributeName.c_str());
	}

	GLint GLSLProgram::getUniformLocation(const std::string &uniformName)
	{
		GLint location = glGetUniformLocation(_programID, uniformName.c_str());
		if (location == GL_INVALID_INDEX) {
			// fatalError("Uniform" + uniformName + " not found in shader!");
			printf("Uniform %s not found in shader\n", uniformName.c_str());
		}
		return location;
	}

	void GLSLProgram::use()
	{
		glUseProgram(_programID);
		 for (int i = 0; i < _numAttributes; i++)
		 {
			 glEnableVertexAttribArray(i);
		 }
	}
	void GLSLProgram::unuse()
	{
		glUseProgram(0);
		 for (int i = 0; i < _numAttributes; i++)
		 {
			 glDisableVertexAttribArray(i);
		 }
	}

	void GLSLProgram::dispose()
	{
		if (_programID != 0) glDeleteProgram(_programID);
		_numAttributes = 0;
	}

	//compiles a single shader file
	void GLSLProgram::compileShader(const char* source, const std::string& name, GLuint id)
	{
		

		glShaderSource(id, 1, &source, nullptr); //nullptr koska vain yksi string

		glCompileShader(id);

		GLint success = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);

		if (success == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);


			std::vector<char> errorLog(maxLength);
			glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

			//exittaa jos failure
			glDeleteShader(id); //don't leak shader

			std::printf("%s\n", &(errorLog[0]));
			printf("shader %s failed to compile", name.c_str());
			// fatalError("shader " + name + " failed to compile");
		}
	}
}