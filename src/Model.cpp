#include "Model.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/ext.hpp"

static inline glm::vec3    copyAssimpVector( const aiVector3D& aiv ) {
    return (glm::vec3(aiv.x, aiv.y, aiv.z));
}

static inline glm::vec4    copyAssimpColor( const aiColor4D& aic ) {
    return (glm::vec4(aic.r, aic.g, aic.b, aic.a));
}
static inline glm::vec3    copyAssimpColor( const aiColor3D& aic ) {
    return (glm::vec3(aic.r, aic.g, aic.b));
}

Model::Model( const std::string& path, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ) : position(position), orientation(orientation), scale(scale) {
    this->loadModel(path);
    /* sort the meshes by transparency of material */
    std::sort(this->meshes.begin(), this->meshes.end(), sortByTransparency);
    this->update();
}

/* this constructor will create a cubemap */
Model::Model( const std::vector<std::string>& paths ) : position(glm::vec3(0, 0, 0)), orientation(glm::vec3(0, 0, 0)), scale(glm::vec3(1, 1, 1)) {
    /* Create debug cube */
    std::vector<float>          v;
    std::vector<tVertex>        vertices;
    std::vector<unsigned int>   indices;
    std::vector<tTexture>       textures;
    createCube(v, indices);
    for (size_t i = 5; i < v.size()+1; i += 5) {
        tVertex vertex;
        vertex.Position = glm::vec3(v[i-5], v[i-4], v[i-3]);
        vertex.Normal = glm::vec3(0, 0, 0);
        vertex.TexCoords = glm::vec2(v[i-2], v[i-1]);
        vertices.push_back(vertex);
    }
    tMaterial material = (tMaterial){ glm::vec3(0,0,0), glm::vec3(0,0,0), glm::vec3(0,0,0), 0.0f };

    /* load the texture */
    tTexture texture;
    texture.id = loadCubemap(paths);
    texture.type = "skybox";
    textures.push_back(texture);

    this->meshes.push_back(Mesh(vertices, indices, textures, material));
    this->update();
}

Model::~Model( void ) {
}

void    Model::render( Shader shader ) {
    // TODO: sort the transparent meshes by distance to camera
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
    const aiScene*  scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals); // aiProcess_CalcTangentSpace

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
    tMaterial                   meshMaterial;

    /* vertices */
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // so it can use a material index defined after
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
    /* process the textures */
    std::vector<tTexture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<tTexture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    std::vector<tTexture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    std::vector<tTexture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    /* process the materials attributes */
    float   f;
    aiColor3D   color(0.0f, 0.0f, 0.0f);
    material->Get(AI_MATKEY_COLOR_AMBIENT, color);
    meshMaterial.ambient = copyAssimpColor(color);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    meshMaterial.diffuse = copyAssimpColor(color);
    material->Get(AI_MATKEY_COLOR_SPECULAR, color);
    meshMaterial.specular = copyAssimpColor(color);
    material->Get(AI_MATKEY_SHININESS, f);
    meshMaterial.shininess = f;
    material->Get(AI_MATKEY_OPACITY, f);
    meshMaterial.opacity = f;

    // std::cout << "ambient: " << glm::to_string(meshMaterial.ambient) << std::endl;
    // std::cout << "diffuse: " << glm::to_string(meshMaterial.diffuse) << std::endl;
    // std::cout << "diffuse: " << glm::to_string(meshMaterial.specular) << std::endl;

    return (Mesh(vertices, indices, textures, meshMaterial));
}

std::vector<tTexture>   Model::loadMaterialTextures( aiMaterial* mat, aiTextureType type, std::string typeName ) {
    std::vector<tTexture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < this->textures_loaded.size(); ++j) {
            if (std::strcmp(this->textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(this->textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) {
            tTexture texture;
            texture.id = loadTexture(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            this->textures_loaded.push_back(texture);
        }
    }
    return (textures);
}

unsigned int    loadTexture( const char* filename ) {
    std::cout << "> texture: " << filename << std::endl;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, channels;
    unsigned char*  data = stbi_load(filename, &width, &height, &channels, 0);

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
        throw Exception::ModelError("TextureLoader", filename);
    }
    return (textureID);
}

unsigned int    loadTexture( const char* path, const std::string& directory ) {
    std::string filename = directory + '/' + std::string(path);
    return (loadTexture(filename.c_str()));
}

unsigned int    loadCubemap( const std::vector<std::string>& paths ) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, channels;
    for (size_t i = 0; i < paths.size(); ++i) {
        unsigned char *data = stbi_load(paths[i].c_str(), &width, &height, &channels, 0);
        if (data) {
            GLenum format;
            switch (channels) {
                case 1: format = GL_RED; break;
                case 3: format = GL_RGB; break;
                case 4: format = GL_RGBA; break;
            };
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            stbi_image_free(data);
            throw Exception::ModelError("CubemapLoader", paths[i]);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    return (textureID);
}
