#pragma once

#include <glm/glm.hpp>

glm::vec4    hex2vec( int64_t hex );
glm::vec2    mousePosToClipSpace( const glm::dvec2& pos, int winWidth, int winHeight );
