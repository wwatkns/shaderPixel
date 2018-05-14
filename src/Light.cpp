#include "Light.hpp"

int Light::pointLightCount = 0;

Light::Light( const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, eLightType type ) : direction(direction), ambient(ambient), diffuse(diffuse), specular(specular), type(type), lconst(1), linear(0.09), quadratic(0.032) {
    if (type == eLightType::point) {
        this->id = this->pointLightCount;
        this->pointLightCount++;
    }
}

Light::Light( const glm::vec3& position, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float lconst, float linear, float quadratic, eLightType type ) : position(position), ambient(ambient), diffuse(diffuse), specular(specular), type(type), lconst(lconst), linear(linear), quadratic(quadratic) {
    if (type == eLightType::point) {
        this->id = this->pointLightCount;
        this->pointLightCount++;
    }
}

Light::~Light( void ) {
    if (type == eLightType::point)
        this->pointLightCount--;
}

void    Light::render( Shader shader ) {
    if (this->type == eLightType::directional) {
        shader.setVec3UniformValue("directionalLight.direction", this->direction);
        shader.setVec3UniformValue("directionalLight.ambient", this->ambient);
        shader.setVec3UniformValue("directionalLight.diffuse", this->diffuse);
        shader.setVec3UniformValue("directionalLight.specular", this->specular);
    }
    else if (this->type == eLightType::point) {
        std::string var = "pointLights[";
        var += std::to_string(this->id) + "].";
        shader.setVec3UniformValue((var+std::string("position")).c_str(), this->position);
        shader.setVec3UniformValue((var+std::string("ambient")).c_str(), this->ambient);
        shader.setVec3UniformValue((var+std::string("diffuse")).c_str(), this->diffuse);
        shader.setVec3UniformValue((var+std::string("specular")).c_str(), this->specular);
        shader.setFloatUniformValue((var+std::string("const")).c_str(), this->lconst);
        shader.setFloatUniformValue((var+std::string("linear")).c_str(), this->linear);
        shader.setFloatUniformValue((var+std::string("quadratic")).c_str(), this->quadratic);
    }
}

// void    Light::update( void ) {
//     this->transform = glm::mat4();
//     this->transform = glm::translate(this->transform, this->position);
//     this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
//     this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
//     this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
//     this->transform = glm::scale(this->transform, this->scale);
// }
