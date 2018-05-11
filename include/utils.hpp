#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

void        createCube( std::vector<GLfloat>& vertices, std::vector<unsigned int>& indices );
glm::vec4   hex2vec( int64_t hex );
glm::vec2   mousePosToClipSpace( const glm::dvec2& pos, int winWidth, int winHeight );
