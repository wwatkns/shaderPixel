#include "RaymarchedSurface.hpp"
#include "glm/ext.hpp"
#include "Model.hpp"

RaymarchedSurface::RaymarchedSurface( const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale) {
    this->createRenderQuad();
    this->setup(GL_STATIC_DRAW);
    this->update();
    this->noiseSamplerId = loadTexture("./resource/RGBAnoiseMedium.png");
}

RaymarchedSurface::~RaymarchedSurface( void ) {
    glDeleteBuffers(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
}

void    RaymarchedSurface::createRenderQuad( void ) {
    /* create quad */
    std::vector<float> v = {{
        -0.5,-0.5, 0.0,  0.0, 1.0, // top-left
         0.5,-0.5, 0.0,  1.0, 1.0, // top-right
         0.5, 0.5, 0.0,  1.0, 0.0, // bottom-right
        -0.5, 0.5, 0.0,  0.0, 0.0  // bottom-left
    }};
    for (size_t i = 5; i < v.size()+1; i += 5) {
        tQuadVertex2 vertex;
        vertex.Position = glm::vec3(v[i-5], v[i-4], v[i-3]);
        vertex.TexCoords = glm::vec2(v[i-2], v[i-1]);
        this->vertices.push_back(vertex);
    }
    this->indices = {{ 0, 1, 2,  2, 3, 0 }};
}

void    RaymarchedSurface::update( void ) {
    this->transform = glm::mat4();
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
    this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
    this->transform = glm::scale(this->transform, this->scale);
}

void    RaymarchedSurface::render( Shader shader ) {
    this->update();
    shader.setMat4UniformValue("model", this->transform);

    glActiveTexture(GL_TEXTURE0);
    shader.setIntUniformValue("noiseSampler", 0);
    glBindTexture(GL_TEXTURE_2D, this->noiseSamplerId);
    /* render */
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}


void    RaymarchedSurface::setup( int mode ) {
    // gen buffers and vertex arrays
	glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
    // bind vertex array object, basically this is an object to allow us to not redo all of this process each time
    // we want to draw an object to screen, all the states we set are stored in the VAO
	glBindVertexArray(this->vao);
    // copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(tQuadVertex2), this->vertices.data(), mode);
    // copy our indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), mode);
    // set the vertex attribute pointers:
    // position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tQuadVertex2), static_cast<GLvoid*>(0));
    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(tQuadVertex2), reinterpret_cast<GLvoid*>(offsetof(tQuadVertex2, TexCoords)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
