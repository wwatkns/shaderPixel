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
#include "Mesh.hpp"

typedef struct  sQuadVertex2 {
    glm::vec3   Position;
    glm::vec2   TexCoords;
}               tQuadVertex2;

class RaymarchedSurface {

public:
    RaymarchedSurface( const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale, unsigned int skybox, unsigned int noise );
    ~RaymarchedSurface( void );

    void                        update( void );
    void                        render( Shader shader );
    unsigned int                noiseSamplerId;
    unsigned int                skyboxId;
    glm::mat4&                  getTransform( void ) { return transform; };

private:
    glm::mat4                   transform;
    glm::vec3                   position;
    glm::vec3                   orientation;
    glm::vec3                   scale;
    /* render quad variables */
    std::vector<tQuadVertex2>   vertices;
    std::vector<unsigned int>   indices;
    unsigned int                vao;
    unsigned int                vbo;
    unsigned int                ebo;

    void                        createRenderQuad( void );
    void                        setup( int mode );

};
