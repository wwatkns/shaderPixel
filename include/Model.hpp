#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <string>
#include <vector>

#include "Exception.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "utils.hpp"
#include "Mesh.hpp"

class Model {

public:
    Model( const std::string& path, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale ); // constructor from existing meshes
    Model( const std::vector<Mesh*> meshes, const std::vector<tTexture> textures, const glm::vec3& position, const glm::vec3& orientation, const glm::vec3& scale );
    Model( const std::vector<std::string>& paths );  // cubemap constructor
    Model( const glm::vec3& position, const glm::vec3& scale ); // billboard constructor
    ~Model( void );

    void            update( void );
    void            render( Shader shader );

    /* getters */
    const glm::mat4&    getTransform( void ) const { return (transform); };
    const glm::vec3&    getPosition( void ) const { return (position); };
    const glm::vec3&    getOrientation( void ) const { return (orientation); };
    const glm::vec3&    getScale( void ) const { return (scale); };

    const std::vector<Mesh*>    getMeshes( void ) const { return (meshes); };
    const std::vector<tTexture> getTextures( void ) const { return (textures_loaded); };
    /* setters */
    void                setPosition( const glm::vec3& t ) { position = t; };
    void                setOrientation( const glm::vec3& r ) { orientation = r; };
    void                setScale( const glm::vec3& s ) { scale = s; };

private:
    glm::mat4               transform;          // the transform applied to the model
    glm::vec3               position;           // the position
    glm::vec3               orientation;        // the orientation
    glm::vec3               scale;              // the scale

    std::vector<Mesh*>      meshes;
    std::string             directory;
    std::vector<tTexture>   textures_loaded;
    bool                    meshClone;

    void                    loadModel( const std::string& path );
    void                    processNode( aiNode* node, const aiScene* scene );
    Mesh*                   processMesh( aiMesh* mesh, const aiScene* scene );
    std::vector<tTexture>   loadMaterialTextures( aiMaterial* mat, aiTextureType type, std::string typeName );

};

unsigned int            loadTexture( const char* path, const std::string& directory );
unsigned int            loadTexture( const char* path );
unsigned int            loadCubemap( const std::vector<std::string>& paths );
