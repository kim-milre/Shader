//
//  triMesh.hpp
//  OpenGLTest
//
//  Created by Hyun Joon Shin on 11/28/24.
//

#ifndef triMesh_h
#define triMesh_h

#ifdef WIN32
#define GLEW_STATIC
#include <GL/glew.h>
#elif defined __APPLE__
#pragma clang diagnostic ignored "-Wdocumentation"
#include <OpenGL/gl3.h>
#endif
#include <string>
#include <glm/glm.hpp>

struct TriMesh {
	GLuint _vertexArrayID = 0;
	GLuint _vertexBufferID = 0;
	GLuint _normalBufferID = 0;
	GLuint _texCoordBufferID = 0;
	GLuint _indexBufferID = 0;
	size_t _nVertices = 0;
	size_t _nTriangles = 0;

	void create(int nVertices, const glm::vec3* vertices, const glm::vec3* normals, const glm::vec2* texCoords,
				int nTriangles, const glm::uvec3* triangles) {
		if( _vertexArrayID ) clear();

		_nVertices = nVertices;
		_nTriangles = nTriangles;
		
		glGenVertexArrays(1, &_vertexArrayID);
		glBindVertexArray(_vertexArrayID);
		
		glGenBuffers(1, &_vertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER, _nVertices * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glGenBuffers(1, &_texCoordBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, _texCoordBufferID);
		glBufferData(GL_ARRAY_BUFFER, _nVertices * sizeof(glm::vec2), texCoords, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glGenBuffers(1, &_normalBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, _normalBufferID);
		glBufferData(GL_ARRAY_BUFFER, _nVertices * sizeof(glm::vec3), normals, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		
		glGenBuffers(1, &_indexBufferID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _nTriangles * sizeof(glm::u32vec3), triangles, GL_STATIC_DRAW);
	}
	void create(const std::vector<glm::vec3> vertices, const std::vector<glm::vec3> normals,
				const std::vector<glm::vec2> texCoords, const std::vector<glm::uvec3> triangles) {
		assert( vertices.size() == normals.size() );
		assert( vertices.size() == texCoords.size() );
		create(int(vertices.size()), vertices.data(), normals.data(), texCoords.data(),
			   int(triangles.size()), triangles.data() );
	}
	void draw() const {
		if( _vertexArrayID==0 || _indexBufferID==0 || _nTriangles==0 ) return;
		glBindVertexArray(_vertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferID);
		glDrawElements(GL_TRIANGLES, int(_nTriangles)*3, GL_UNSIGNED_INT, 0);
	}
	void clear() {
		if( _vertexArrayID )	glDeleteVertexArrays(1, &_vertexArrayID);
		if( _texCoordBufferID )	glDeleteBuffers(1, &_texCoordBufferID);
		if( _vertexBufferID )	glDeleteBuffers(1, &_vertexBufferID);
		if( _normalBufferID )	glDeleteBuffers(1, &_normalBufferID);
		if( _indexBufferID )	glDeleteBuffers(1, &_indexBufferID);
	}
	~TriMesh() {
		clear();
	}
};



#endif /* triMesh_h */
