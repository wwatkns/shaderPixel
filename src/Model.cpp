#include "Model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/ext.hpp"

static inline glm::vec3    copyAssimpVector( const aiVector3D& aiv ) {
    return (glm::vec3(aiv.x, aiv.y, aiv.z));
}

Model::Model( const std::string& path, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale) {
    this->loadModel(path);

    /* Create debug cube */
    // std::vector<float>          v;
    // std::vector<tVertex>        vertices; // good
    // std::vector<unsigned int>   indices; // good
    // std::vector<tTexture>       textures;
    // createCube(v, indices);
    // for (size_t i = 5; i < v.size()+1; i += 5) {
    //     tVertex vertex;
    //     vertex.Position = glm::vec3(v[i-5], v[i-4], v[i-3]);
    //     vertex.Normal = glm::vec3(0, 0, 0);
    //     vertex.TexCoords = glm::vec2(v[i-2], v[i-1]);
    //     vertices.push_back(vertex);
    // }
    // this->meshes.push_back(Mesh(vertices, indices, textures));

    this->update();
}

Model::~Model( void ) {
}

void    Model::render( Shader shader ) {
    this->update();
    shader.setMat4UniformValue("model", this->transform);
    for (unsigned int i = 0; i < this->meshes.size(); ++i)
        this->meshes[i].render(shader);
}

void    Model::update( void ) {
    this->transform = glm::mat4();
    this->transform = glm::translate(this->transform, this->position);
    this->transform = glm::rotate(this->transform, this->orientation.z, glm::vec3(0, 0, 1));
    this->transform = glm::rotate(this->transform, this->orientation.y, glm::vec3(0, 1, 0));
    this->transform = glm::rotate(this->transform, this->orientation.x, glm::vec3(1, 0, 0));
    this->transform = glm::scale(this->transform, this->scale);
}

void    Model::loadModel( const std::string& path ) {
    std::cout << "Loading: " << path << std::endl;
    Assimp::Importer import;
    // const aiScene*  scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
    const aiScene*  scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        throw Exception::ModelError("AssimpLoader", import.GetErrorString());

    this->directory = path.substr(0, path.find_last_of('/'));
    this->processNode(scene->mRootNode, scene);
}

void    Model::processNode( aiNode* node, const aiScene* scene ) {
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->processMesh(mesh, scene));
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        this->processNode(node->mChildren[i], scene);
}

Mesh    Model::processMesh( aiMesh* mesh, const aiScene* scene ) {
    std::vector<tVertex>        vertices;
    std::vector<unsigned int>   indices;
    std::vector<tTexture>       textures;

    /* vertices */
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        tVertex vertex;
        vertex.Position = copyAssimpVector(mesh->mVertices[i]);
        vertex.Normal = copyAssimpVector(mesh->mNormals[i]);
        vertex.TexCoords = (mesh->mTextureCoords[0] ? glm::vec2(copyAssimpVector(mesh->mTextureCoords[0][i])) : glm::vec2(0.0f, 0.0f));
        // vertex.Tangent = copyAssimpVector(mesh->mTangents[i]);
        // vertex.Bitangent = copyAssimpVector(mesh->mBitangents[i]);
        vertices.push_back(vertex);
    }
    /* indices */
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }
    /* materials */
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<tTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<tTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<tTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<tTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return (Mesh(vertices, indices, textures));
}

std::vector<tTexture>   Model::loadMaterialTextures( aiMaterial* mat, aiTextureType type, std::string typeName ) {
    std::vector<tTexture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < this->textures_loaded.size(); ++j) {
            if (std::strcmp(this->textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(this->textures_loaded[j]);
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
            this->textures_loaded.push_back(texture);
        }
    }
    return (textures);
}

unsigned int    TextureFromFile( const char* path, const std::string& directory, bool gamma ) {
    std::string filename = directory + '/' + std::string(path);
    std::cout << "> texture: " << filename << std::endl;

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
        stbi_image_free(data);
    }
    else {
        stbi_image_free(data);
        throw Exception::ModelError("TextureLoader", path);
    }
    return (textureID);
}
