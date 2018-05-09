#include "Mesh.hpp"

Mesh::Mesh( const std::vector<tVertex>& vertices, const std::vector<GLuint>& indices, const std::vector<tTexture>& textures );
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;
    this->setup(GL_STATIC_DRAW);
}

Mesh::~Mesh( void ) {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
}

void    Mesh::render( GLuint shaderId, const Camera& camera ) {
    // bind appropriate textures
    unsigned int dn = 1;
    unsigned int sp = 1;
    unsigned int nn = 1;
    unsigned int hn = 1;
    // std::array<GLuint, 4> n = { 1, 1, 1, 1 };
    for (size_t i = 0; i < this->textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
        std::string number;
        std::string name = this->textures[i].type;
        switch (name) {
            case "texture_diffuse":  number = std::to_string(dn++); break;
            case "texture_specular": number = std::to_string(sn++); break;
            case "texture_normal":   number = std::to_string(nn++); break;
            case "texture_height":   number = std::to_string(hn++); break;
        };
        glUniform1i(glGetUniformLocation(shaderId, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }
    // draw mesh
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void    Mesh::setup( int mode ) {
    // gen buffers and vertex arrays
	glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
    // bind vertex array object, basically this is an object to allow us to not redo all of this process each time
    // we want to draw an object to screen, all the states we set are stored in the VAO
	glBindVertexArray(this->vao);
    // copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(tVertex), this->vertices.data(), mode);
    // copy our indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), this->indices.data(), mode);
    // set the vertex attribute pointers:
    // position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), static_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);
    // normal coord attributes
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(tVertex), reinterpret_cast<GLvoid*>(offsetof(tVertex, Normal));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(tVertex), reinterpret_cast<GLvoid*>(offsetof(tVertex, TexCoords));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// void    Mesh::initTexture( const std::string& src ) {
//     unsigned int texture;
//     glGenTextures(1, &texture);
//     glBindTexture(GL_TEXTURE_2D, texture);
//     // set the texture wrapping parameters
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     // set texture filtering parameters
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     int width, height, channels;
//     unsigned char* data = stbi_load(src.c_str(), &width, &height, &channels, 0);
//     if (data) {
//         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//         glGenerateMipmap(GL_TEXTURE_2D);
//     }
//     else {
//         std::cout << "Could not load texture \"" << src << "\"" << std::endl;
//         std::exit(1);
//         // throw Exception::textureError("");
//     }
// }

// "/Users/wwatkins/Downloads/the-picture-gallery/source/161122_HWY_Galleri/161122_HWY_Galleri.obj";
// "/Users/wwatkins/Downloads/bmw/source/bmw.obj";
