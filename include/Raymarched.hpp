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
#include "Mesh.hpp" // for material struct

enum class eRaymarchObject {
    mandelbox,
    mandelbulb,
    ifs,
    marble,
    cloud,
    blob
};

typedef struct  sQuadVertex {
    glm::vec3   Position;
    glm::vec2   TexCoords;
}               tQuadVertex;

typedef struct  sObject {
    eRaymarchObject id;
    glm::vec3       position;
    glm::vec3       orientation;
    float           scale;
    float           boundingSphereScale;
    float           speedMod;
    tMaterial       material;
}               tObject;

class Raymarched {

public:
    Raymarched( const std::vector<tObject>& objects );
    ~Raymarched( void );

    void            render( Shader shader );
    float           computeSpeedModifier( const glm::vec3& cameraPos );
    unsigned int                skyboxId;
    unsigned int                noiseSamplerId;

private:
    std::vector<tObject>        objects;

    /* render quad variables */
    std::vector<tQuadVertex>    vertices;
    std::vector<unsigned int>   indices;
    unsigned int                vao;
    unsigned int                vbo;
    unsigned int                ebo;

    void                        createRenderQuad( void );
    void                        setup( int mode );

};
