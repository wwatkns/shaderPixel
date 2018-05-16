#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "Exception.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "utils.hpp"

typedef struct  sVertex {
    glm::vec3   Position;
    glm::vec3   Normal;
    glm::vec2   TexCoords;
    glm::vec3   Tangent;
    glm::vec3   Bitangent;
}               tVertex;

typedef struct  sMaterial {
    glm::vec3   ambient;
    glm::vec3   diffuse;
    glm::vec3   specular;
    float       shininess;
    float       opacity;
}               tMaterial;

typedef struct  sTexture {
    unsigned int    id;
    std::string     type;
    std::string     path;
}               tTexture;

class Mesh {

public:
    Mesh( std::vector<tVertex> vertices, std::vector<unsigned int> indices, std::vector<tTexture> textures, tMaterial material );
    ~Mesh( void );

    void                render( Shader shader );
    /* getters */
    const GLuint&       getVao( void ) const { return (vao); };
    const tMaterial&    getMaterial( void ) const { return (material); };

private:
    unsigned int                vao;               // Vertex Array Object
    unsigned int                vbo;               // Vertex Buffer Object
    unsigned int                ebo;               // Element Buffer Object (or indices buffer object, ibo)

    std::vector<tVertex>        vertices;
    std::vector<unsigned int>   indices;
    std::vector<tTexture>       textures;
    tMaterial                   material;

    void                    setup( int mode );

};

bool sortByTransparency( const Mesh* a, const Mesh* b );
