#include "Model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model( const std::string& src, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale), shader(Shader("./shader/vertex.glsl", "./shader/fragment.glsl")) {
    this->nIndices = 0;
    tObj obj = this->loadObjFromFile(src);
    this->initBufferObjects(obj, GL_STATIC_DRAW);
    this->initTexture("/Users/wwatkins/Downloads/text_04.jpg");
    this->transform = glm::mat4();
    this->update();
}

Model::~Model( void ) {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
}

void    Model::render( const Camera& camera ) {
    this->shader.use();
    this->shader.setMat4UniformValue("view", camera.getViewMatrix());
    this->shader.setMat4UniformValue("projection", camera.getProjectionMatrix());
    this->shader.setMat4UniformValue("model", this->transform);
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->nIndices, GL_UNSIGNED_INT, 0);

}

void    Model::update( void ) {
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
    this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
    this->transform = glm::scale(this->transform, this->scale);
}

// void    Model::initBufferObjects( const tObj& obj, int mode ) {
//     this->nIndices = obj.indices.size();
//     // gen buffers and vertex arrays
// 	glGenVertexArrays(1, &this->vao);
//     glGenBuffers(1, &this->vbo);
// 	glGenBuffers(1, &this->ebo);
//     // bind vertex array object, basically this is an object to allow us to not redo all of this process each time
//     // we want to draw an object to screen, all the states we set are stored in the VAO
// 	glBindVertexArray(this->vao);
//     // copy our vertices array in a buffer for OpenGL to use
// 	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
// 	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * obj.vertices.size(), obj.vertices.data(), mode);
//     // copy our indices array in a buffer for OpenGL to use
// 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
// 	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj.indices.size(), obj.indices.data(), mode);
//     // set the vertex attribute pointers:
//     // position attribute
// 	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), static_cast<GLvoid*>(0));
// 	glEnableVertexAttribArray(0);
//     // texture coord attribute
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
//     glEnableVertexAttribArray(1);
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
// }

void    Model::initBufferObjects( const tObj& obj, int mode ) {
    // "/Users/wwatkins/Downloads/the-picture-gallery/source/161122_HWY_Galleri/161122_HWY_Galleri.obj";
    // "/Users/wwatkins/Downloads/bmw/source/bmw.obj";


    this->nIndices = obj.indices.size();
    // gen buffers and vertex arrays
	glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
    // bind vertex array object, basically this is an object to allow us to not redo all of this process each time
    // we want to draw an object to screen, all the states we set are stored in the VAO
	glBindVertexArray(this->vao);
    // copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * obj.vertices.size(), obj.vertices.data(), mode);
    // copy our indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * obj.indices.size(), obj.indices.data(), mode);
    // set the vertex attribute pointers:
    // position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), static_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<GLvoid*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void    Model::initTexture( const std::string& src ) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, channels;
    unsigned char* data = stbi_load(src.c_str(), &width, &height, &channels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Could not load texture \"" << src << "\"" << std::endl;
        std::exit(1);
        // throw Exception::textureError("");
    }
}

tObj    Model::loadObjFromFile( const std::string& src ) {
    tObj    res;
    createCube(res.vertices, res.indices);
    return (res);
}
