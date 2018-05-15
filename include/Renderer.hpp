#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "Exception.hpp"
#include "Env.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Light.hpp"

typedef struct  sShadowDepthMap {
    unsigned int    id;
    unsigned int    fbo;
    size_t          width;
    size_t          height;
}               tShadowDepthMap;

typedef std::unordered_map<std::string, Shader*> tShaderMap;

class Renderer {

public:
    Renderer( Env* env );
    ~Renderer( void );

    void	loop( void );
    void    updateShadowDepthMap( void );
    void    renderLights( void );
    void    renderMeshes( void );
    void    renderSkybox( void );

private:
    Env*            env;
    Camera          camera;
    tShaderMap      shader;
    tShadowDepthMap shadowDepthMap;
    glm::mat4       lightSpaceMat;

    void    initShadowDepthMap( const size_t width = 1024, const size_t height = 1024 );

};
