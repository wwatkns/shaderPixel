#include "utils.hpp"

glm::vec4    hex2vec( int64_t hex ) {
    return glm::vec4(
        ((hex >> 16) & 0xFF) / 255.0f,
        ((hex >>  8) & 0xFF) / 255.0f,
        ((hex      ) & 0xFF) / 255.0f,
        1.0f
    );
}

glm::vec2    mousePosToClipSpace( const glm::dvec2& pos, int winWidth, int winHeight ) {
    glm::vec2    mouse = glm::vec2({(float)pos[0] / winWidth, (float)pos[1] / winHeight}) * 2.0f - 1.0f;
    mouse.y = -mouse.y;
    return (mouse);
}
