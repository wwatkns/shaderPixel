#include "Model.hpp"

Model::Model( const std::string& src, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale), shader(Shader("./shader/vertex.glsl", "./shader/fragment.glsl")) {
    this->nIndices = 0;
    tObj obj = this->loadObjFromFile(src);
    this->initBufferObjects(obj, GL_STATIC_DRAW);
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
    this->shader.setVec4UniformValue("customColor", glm::vec4(1, 0, 0, 1)); // TODO: replace by texture
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

void    Model::initBufferObjects( const tObj& obj, int mode ) {
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
    // set the vertex attribute pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), static_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

tObj    Model::loadObjFromFile( const std::string& src ) {
    tObj    res;
    createCube(res.vertices, res.indices);
    return (res);
}
