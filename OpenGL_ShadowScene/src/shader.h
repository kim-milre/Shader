//
//  toys.h
//  ComputerGraphics_2
//
//  Created by Hyun Joon Shin on 13/03/2018.
//  Copyright © 2018 Hyun Joon Shin. All rights reserved.
//

#ifndef toys_h
#define toys_h

#ifdef WIN32
#include <Windows.h>
#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#elif defined __APPLE__
#pragma clang diagnostic ignored "-Wdocumentation"
#include <OpenGL/gl3.h>
#endif

#include <string>
#include <fstream>
#include <iostream>


#ifdef WIN32
typedef wchar_t CHAR_T;
#include <windows.h>
inline std::wstring utf82Unicode(const std::string& s) {
	int len = MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,NULL,0);
	wchar_t* wstr = new wchar_t[len+1];
	len = MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,wstr,len);
	wstr[len]=L'\0';
	std::wstring ws(L""); ws+=wstr;
	delete wstr;
	return ws;
}
#else
typedef char CHAR_T;
#define utf82Unicode(X) (X)
#endif

inline std::string getFilenameFromAbsPath( const std::string& filename )
{
	size_t slashPos = filename.find_last_of('/');
	if( slashPos == std::string::npos ) return filename;
	if( slashPos == filename.length()-1 ) return "";
	return filename.substr(slashPos+1);
}


inline std::string loadText(const std::string& filename) {
	std::ifstream t(utf82Unicode(filename));
	if( !t.is_open() ) {
		std::cerr<<"[ERROR] Text file: "<<getFilenameFromAbsPath(filename)<<" is not found\n";
		return "";
	}
	std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	return str;
}

static inline void printInfoProgramLog(GLuint obj )
{
	int infologLength = 0, charsWritten  = 0;
	glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength <=0 ) return;
	std::cerr<<"Program Info:"<<std::endl;
	char *infoLog = new char[infologLength];
	glGetProgramInfoLog( obj, infologLength, &charsWritten, infoLog );
	std::cerr<<infoLog<<std::endl;
	delete [] infoLog;
}
static inline void printInfoShaderLog(GLuint obj )
{
	int infologLength = 0, charsWritten  = 0;
	glGetShaderiv ( obj, GL_INFO_LOG_LENGTH, &infologLength );
	if( infologLength <=0 ) return;
	std::cerr<<"Shader Info:"<<std::endl;
	char *infoLog = new char[infologLength];
	glGetShaderInfoLog ( obj, infologLength, &charsWritten, infoLog );
	std::cerr<<infoLog<<std::endl;
	delete [] infoLog;
}

inline GLuint loadShaders( const char* vsFilename, const char* fsFilename ) {
	GLuint vertShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint programID = glCreateProgram();

	std::string vertCode = loadText( vsFilename );
	if( vertCode.length() <1 ) {
		std::cerr<<"Vertex shader code is not loaded properly"<<std::endl;
		return 0;
	}
	std::string fragCode = loadText( fsFilename );
	if( fragCode.length() <1 ) {
		std::cerr<<"Fragment shader code is not loaded properly"<<std::endl;
		return 0;
	}


	const GLchar* vshaderCode = vertCode.c_str();
	glShaderSource	( vertShaderID, 1, &vshaderCode, nullptr );
	glCompileShader	( vertShaderID );
	printInfoShaderLog	( vertShaderID );
	glAttachShader  ( programID, vertShaderID );
	
	const GLchar* fshaderCode = fragCode.c_str();
	glShaderSource	( fragShaderID, 1, &fshaderCode, nullptr );
	glCompileShader	( fragShaderID );
	printInfoShaderLog	( fragShaderID ) ;
	glAttachShader  ( programID, fragShaderID );
	
	glLinkProgram( programID );
	glUseProgram ( programID );
	printInfoProgramLog( programID );
	return programID;
}


#endif /* toys_h */
