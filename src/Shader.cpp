#include "Shader.hpp"

Shader::Shader( const std::string& vertexShader, const std::string& fragmentShader ) {
    std::string vSrc = getFromFile(vertexShader);
    std::string fSrc = getFromFile(fragmentShader);

    GLuint vertShader = this->create(vSrc.c_str(), GL_VERTEX_SHADER);
    GLuint fragShader = this->create(fSrc.c_str(), GL_FRAGMENT_SHADER);
    this->id = this->createProgram({{ vertShader, fragShader }});
}

Shader::~Shader( void ) {
}

void    Shader::use( void ) const {
    glUseProgram(this->id);
}

/*  we load the content of a file in a string (we need that because the shader compilation is done at
    runtime and glCompileShader expects a <const GLchar *> value)
*/
std::string   Shader::getFromFile( const std::string& filename ) {
    std::ifstream   ifs(filename);
    std::string     content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    return (content);
}

/*  we create the shader from a file in format glsl. The shaderType defines what type of shader it is
    and it returns the id to the created shader (the shader object is allocated by OpenGL in the back)
*/
GLuint  Shader::create( const char* shaderSource, GLenum shaderType ) {
	GLint success;
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderSource, nullptr);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    this->isCompilationSuccess(shader, success, shaderType);
	return (shader);
}

/*  here we create the shader program that will be used to render our objects. It takes a list of shaders
    that will instruct the GPU how to manage the vertices, etc... and we delete the compiled shaders at the
    end because we no longer need them. We return the id of the created shader program.
*/
GLuint  Shader::createProgram( const std::forward_list<GLuint>& shaders ) {
	GLint success;
	GLuint shaderProgram = glCreateProgram();
    for (std::forward_list<GLuint>::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
        glAttachShader(shaderProgram, *it);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    this->isCompilationSuccess(shaderProgram, success, -1);
    for (std::forward_list<GLuint>::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
        glDeleteShader(*it);
	return (shaderProgram);
}

/*  check if the shader or shader program compilation returned an error, if so throw an exception
    with the message specified by glGetShaderInfoLog or glGetProgramInfoLog.
*/
void    Shader::isCompilationSuccess( GLint handle, GLint success, int shaderType ) {
    if (!success) {
        char infoLog[512];
        if (shaderType != -1)
            glGetShaderInfoLog(handle, 512, nullptr, infoLog);
        else
            glGetProgramInfoLog(handle, 512, nullptr, infoLog);
        throw Exception::ShaderError(shaderType, infoLog);
    }
}

/*  find the uniform location in the shader and store it in an unordered_map.
    next time we want to use it we just have to get the location from the map
*/
unsigned int    Shader::getUniformLocation( const std::string& name ) {
    if (this->uniformLocations.find(name) != this->uniformLocations.end())
        return (this->uniformLocations[name]);
    unsigned int newLoc = glGetUniformLocation(this->id, name.c_str());
    this->uniformLocations[name] = newLoc;
    return (newLoc);
}

void    Shader::setIntUniformValue( const std::string& name, const int i ) {
    glUniform1i(getUniformLocation(name), i);
}
void    Shader::setFloatUniformValue( const std::string& name, const float f ) {
    glUniform1f(getUniformLocation(name), f);
}
void    Shader::setMat2UniformValue( const std::string& name, const glm::mat2& m ) {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}
void    Shader::setMat3UniformValue( const std::string& name, const glm::mat3& m ) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}
void    Shader::setMat4UniformValue( const std::string& name, const glm::mat4& m ) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(m));
}
void    Shader::setVec2UniformValue( const std::string& name, const glm::vec2& v ) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(v));
}
void    Shader::setVec3UniformValue( const std::string& name, const glm::vec3& v ) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(v));
}
void    Shader::setVec4UniformValue( const std::string& name, const glm::vec4& v ) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(v));
}
