#include "Raymarched.hpp"
#include "glm/ext.hpp"

Raymarched::Raymarched( const std::vector<tObject>& objects ) : objects(objects) {
    this->createRenderQuad();
    this->setup(GL_STATIC_DRAW);
    // this->update();
}

Raymarched::~Raymarched( void ) {
    glDeleteBuffers(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
}

void    Raymarched::createRenderQuad( void ) {
    /* create quad */
    std::vector<float> v = {{
        -0.5,-0.5, 0.0,  0.0, 1.0, // top-left
         0.5,-0.5, 0.0,  1.0, 1.0, // top-right
         0.5, 0.5, 0.0,  1.0, 0.0, // bottom-right
        -0.5, 0.5, 0.0,  0.0, 0.0  // bottom-left
    }};
    for (size_t i = 5; i < v.size()+1; i += 5) {
        tQuadVertex vertex;
        vertex.Position = glm::vec3(v[i-5], v[i-4], v[i-3]);
        vertex.TexCoords = glm::vec2(v[i-2], v[i-1]);
        this->vertices.push_back(vertex);
    }
    this->indices = {{ 0, 1, 2,  2, 3, 0 }};
}

// void    Raymarched::update( void ) {
//     this->transform = glm::mat4();
//     this->transform = glm::translate(this->transform, this->position);
//     this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
//     this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
//     this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
//     // this->transform = glm::scale(this->transform, this->scale);
// }

void    Raymarched::render( Shader shader ) {
    // this->update();

    shader.setMat4UniformValue("model", glm::mat4());
    shader.setIntUniformValue("nObjects", this->objects.size());

    std::string name;
    for (int i = 0; i < this->objects.size(); ++i) {
        glm::mat4 mat = glm::mat4();
        mat = glm::translate(mat, this->objects[i].position);
        mat = glm::rotate(mat, this->objects[i].orientation.z, glm::vec3(0, 0, 1));
        mat = glm::rotate(mat, this->objects[i].orientation.y, glm::vec3(0, 1, 0));
        mat = glm::rotate(mat, this->objects[i].orientation.x, glm::vec3(1, 0, 0));

        name = std::string("object[")+std::to_string(i)+std::string("].");
        /* set material attributes */
        shader.setVec3UniformValue(name+"material.ambient", this->objects[i].material.ambient);
        shader.setVec3UniformValue(name+"material.diffuse", this->objects[i].material.diffuse);
        shader.setVec3UniformValue(name+"material.specular", this->objects[i].material.specular);
        shader.setFloatUniformValue(name+"material.shininess", this->objects[i].material.shininess);
        shader.setFloatUniformValue(name+"material.opacity", this->objects[i].material.opacity);

        shader.setIntUniformValue(name+"id", static_cast<int>(this->objects[i].id));
        shader.setFloatUniformValue(name+"scale", this->objects[i].scale);
        shader.setMat4UniformValue(name+"invMat", glm::inverse(mat));
    }

    /* render */
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}


void    Raymarched::setup( int mode ) {
    // gen buffers and vertex arrays
	glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
    // bind vertex array object, basically this is an object to allow us to not redo all of this process each time
    // we want to draw an object to screen, all the states we set are stored in the VAO
	glBindVertexArray(this->vao);
    // copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(tQuadVertex), this->vertices.data(), mode);
    // copy our indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), this->indices.data(), mode);
    // set the vertex attribute pointers:
    // position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tQuadVertex), static_cast<GLvoid*>(0));
    // texture coord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(tQuadVertex), reinterpret_cast<GLvoid*>(offsetof(tQuadVertex, TexCoords)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
