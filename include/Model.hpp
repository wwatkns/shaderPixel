#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "Exception.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "utils.hpp"

typedef struct  sObj {
    std::vector<GLfloat>    vertices;
    std::vector<GLuint>     indices;
}               tObj;

class Model {

public:
    Model( const std::string& src, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale );
    // Model( const std::string& src, Shader* shader, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale );
    ~Model( void );

    void            update( void );
    void            render( const Camera& camera );

    /* getters */
    const GLuint&       getVao( void ) const { return (vao); };
    const glm::mat4&    getTransform( void ) const { return (transform); };
    const glm::vec3&    getPosition( void ) const { return (position); };
    const glm::vec3&    getOrientation( void ) const { return (orientation); };
    const glm::vec3&    getScale( void ) const { return (scale); };
    /* setters */
    void                setPosition( const glm::vec3& t ) { position = t; };
    void                setOrientation( const glm::vec3& r ) { orientation = r; };
    void                setScale( const glm::vec3& s ) { scale = s; };

private:
    int                 nIndices;           // the number of triangles of the model
    GLuint              vao;                // Vertex Array Object
    GLuint              vbo;                // Vertex Buffer Object
    GLuint              ebo;                // Element Buffer Object (or indices buffer object, ibo)

    glm::mat4           transform;          // the transform applied to the model
    glm::vec3           position;           // the position
    glm::vec3           orientation;        // the orientation
    glm::vec3           scale;              // the scale

    Shader              shader;            // the shader used by the model

    void                initBufferObjects( const tObj& obj, int mode = GL_STATIC_DRAW );
    tObj                loadObjFromFile( const std::string& src );

};

/* TODO: implement .obj loading (see scop) */
