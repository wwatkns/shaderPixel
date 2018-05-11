#include "Env.hpp"

Env::Env( void ) {
    try {
        this->initGlfwEnvironment("4.0");
        this->initGlfwWindow(960, 720);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw Exception::InitError("glad initialization failed");
        this->controller = new Controller(this->window.ptr);
        this->model = new Model(
            "/Users/wwatkins/Downloads/nanosuit/nanosuit.obj",
            // "/Users/wwatkins/Downloads/amaravati-guardian-lion/source/amaravatiGuardianLion/lion.obj",
            // "/Users/wwatkins/Downloads/rpg-reptile-mage/source/LizPosed.obj",
            // "/Users/wwatkins/Downloads/swimsuit-succubus/source/SS.obj",
            // glm::vec3(0.0f, -1.75f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f)
            // glm::vec3(0.2f, 0.2f, 0.2f)
        );
        this->setupController();
    } catch (const std::exception& err) {
        std::cout << err.what() << std::endl;
    }
}

Env::~Env( void ) {
    delete this->controller;
    glfwDestroyWindow(this->window.ptr);
    glfwTerminate();
}

void	Env::initGlfwEnvironment( const std::string& glVersion ) {
	if (!glfwInit())
		throw Exception::InitError("glfw initialization failed");
    if (!std::regex_match(glVersion, static_cast<std::regex>("^[0-9]{1}.[0-9]{1}$")))
        throw Exception::InitError("invalid openGL version specified");
    size_t  p = glVersion.find('.');
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, std::stoi(glVersion.substr(0,p)));
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, std::stoi(glVersion.substr(p+1)));
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

void	Env::initGlfwWindow( size_t width, size_t height ) {
    // glfwWindowHint(GLFW_SAMPLES, 4); // NOTE: check if anti-aliasing is slow
	if (!(this->window.ptr = glfwCreateWindow(width, height, "shaderPixel", NULL, NULL)))
        throw Exception::InitError("glfwCreateWindow failed");
	glfwMakeContextCurrent(this->window.ptr);
	glfwSetFramebufferSizeCallback(this->window.ptr, this->framebufferSizeCallback);
	glfwSetInputMode(this->window.ptr, GLFW_STICKY_KEYS, 1);
    this->window.width = width;
    this->window.height = height;
}

void    Env::setupController( void ) {
    /* set key properties here */
    // this->controller->setKeyProperties(GLFW_KEY_C, eKeyMode::toggle, 0, 1000);
    // this->controller->setKeyProperties(GLFW_KEY_M, eKeyMode::cycle, 1, 300, 3);
}

void    Env::framebufferSizeCallback( GLFWwindow* window, int width, int height ) {
    glViewport(0, 0, width, height);
}
