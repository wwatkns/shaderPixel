#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

typedef std::unordered_map<std::string, Shader*> tShaderMap;

class Renderer {

public:
    Renderer( Env* env );
    ~Renderer( void );

    void	loop( void );
    void    renderMeshes( void );
    void    renderSkybox( void );

private:
    Env*        env;
    Camera      camera;
    tShaderMap  shader;

};
