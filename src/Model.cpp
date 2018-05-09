#include "Model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Model::Model( const std::string& path, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale), shader(Shader("./shader/vertex.glsl", "./shader/fragment.glsl")) {
    this->loadModel(path);
    this->transform = glm::mat4();
    this->update();
}

Model::~Model( void ) {
}

void    Model::render( const Camera& camera ) {
    this->shader.use();
    this->shader.setMat4UniformValue("view", camera.getViewMatrix());
    this->shader.setMat4UniformValue("projection", camera.getProjectionMatrix());
    this->shader.setMat4UniformValue("model", this->transform);

    for (auto it = this->meshes.begin(); it != this->meshes.end(); it++)
        it->render(this->shader.id, camera);
}

void    Model::update( void ) {
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
    this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
    this->transform = glm::scale(this->transform, this->scale);
}

void    Model::loadModel( const std::string& path ) {
    Assimp::Importer import;
    const aiScene*  scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of('/'));
    this->processNode(scene->mRootNode, scene);
}

void    Model::processNode( aiNode* node, const aiScene* scene ) {
    // process all the node's meshes (if any)
    for (size_t i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(this->processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (size_t i = 0; i < node->mNumChildren; ++i)
        this->processNode(node->mChildren[i], scene);
}

Mesh    Model::processMesh( aiMesh* mesh, const aiScene* scene ) {
    std::vector<tVertex>    vertices;
    std::vector<GLuint>     indices;
    std::vector<tTexture>   textures;

    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        // texture coordinates
        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        // // tangent
        // vector.x = mesh->mTangents[i].x;
        // vector.y = mesh->mTangents[i].y;
        // vector.z = mesh->mTangents[i].z;
        // vertex.Tangent = vector;
        // // bitangent
        // vector.x = mesh->mBitangents[i].x;
        // vector.y = mesh->mBitangents[i].y;
        // vector.z = mesh->mBitangents[i].z;
        // vertex.Bitangent = vector;
        vertices.push_back(vertex);
    }
    // indices
    for (size_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    vector<tTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    vector<tTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<tTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<tTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
    return (Mesh(vertices, indices, textures));
}

std::vector<tTexture>   Model::loadMaterialTextures( aiMaterial* mat, aiTextureType type, std::string typeName ) {
    std::vector<tTexture> textures;
    for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (size_t j = 0; j < textures_loaded.size(); ++j) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip) {
            tTexture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return (textures);
}

unsigned int    Model::TextureFromFile( const char* path, const string& directory, bool gamma) {
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    unsigned char*  data = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    if (data) {
        GLenum format;
        switch (channels) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
        };
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);
    return (textureID);
}

// tObj    Model::loadObjFromFile( const std::string& src ) {
//     tObj    res;
//     createCube(res.vertices, res.indices);
//     return (res);
// }
