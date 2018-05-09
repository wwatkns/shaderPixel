#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
// #include <stddef.h>

#include "Exception.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "utils.hpp"

typedef struct  sVertex {
    glm::vec3   Position;
    glm::vec3   Normal;
    glm::vec2   TexCoords;
}               tVertex;

typedef struct  sTexture {
    unsigned int    id;
    std::string     type;
    std::string     path;
}               tTexture;

class Mesh {

public:
    Mesh( const std::vector<tVertex>& vertices, const std::vector<GLuint>& indices, const std::vector<tTexture>& textures );
    ~Mesh( void );

    void                render( GLuint shaderId, const Camera& camera );

    /* getters */
    const GLuint&       getVao( void ) const { return (vao); };

private:
    GLuint                  vao;               // Vertex Array Object
    GLuint                  vbo;               // Vertex Buffer Object
    GLuint                  ebo;               // Element Buffer Object (or indices buffer object, ibo)

    std::vector<tVertex>    vertices;
    std::vector<GLuint>     indices;
    std::vector<tTexture>   textures;

    void                    setup( int mode );

};
