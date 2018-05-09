#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <forward_list>

#include "Exception.hpp"
#include "Env.hpp"
#include "Shader.hpp"
#include "Camera.hpp"

class Renderer {

public:
    Renderer( Env* env );
    ~Renderer( void );

    void	loop( void );

private:
    Env*        env;
    Shader      shader;
    Camera      camera;

    void    updateShaderUniforms( void );

};
