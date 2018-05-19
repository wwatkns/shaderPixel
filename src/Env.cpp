#include "Env.hpp"

Env::Env( void ) {
    try {
        this->initGlfwEnvironment("4.0");
        this->initGlfwWindow(1280, 720);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw Exception::InitError("glad initialization failed");
        this->controller = new Controller(this->window.ptr);
        // this->models = {{
        //     // new Model(
        //     //     "/Users/wwatkins/Downloads/workshop/source/model.obj",
        //     //     glm::vec3(0.0f, 0.0f, 0.0f),
        //     //     glm::vec3(0.0f, -M_PI/2.0f, 0.0f),
        //     //     glm::vec3(10.0f, 10.0f, 10.0f)
        //     // ),
        //     // new Model(
        //     //     "/Users/wwatkins/Downloads/pillar01/source/Pillar_LP.obj",
        //     //     glm::vec3(10.0f, 0.0f, 0.0f),
        //     //     glm::vec3(0.0f),
        //     //     glm::vec3(0.5f)
        //     // ),
        // }};
        this->raymarchedObjects = {{
            new RaymarchedObject(
                glm::vec3(0.0),
                glm::vec3(0.0),
                glm::vec3(1.0),
                (tMaterial){
                    glm::vec3(0.0),
                    glm::vec3(0.0),
                    glm::vec3(0.35, 0.35, 0.35),
                    82.0,
                    1.0
                }
            )
        }};
        this->lights = {{
            new Light(
                glm::vec3(10, 10, 6),
                glm::vec3(0.77f, 0.88f, 1.0f) * 0.05f,
                glm::vec3(1.0f, 0.964f, 0.77f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                eLightType::directional
            ),
            new Light(
                glm::vec3(3.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                1.0f,
                0.09f,
                0.032f,
                eLightType::point
            ),
        }};
        this->skybox = new Model(std::vector<std::string>{{
            "./resource/ThickCloudsWater/ThickCloudsWaterLeft2048.png",
            "./resource/ThickCloudsWater/ThickCloudsWaterRight2048.png",
            "./resource/ThickCloudsWater/ThickCloudsWaterUp2048.png",
            "./resource/ThickCloudsWater/ThickCloudsWaterDown2048.png",
            "./resource/ThickCloudsWater/ThickCloudsWaterFront2048.png",
            "./resource/ThickCloudsWater/ThickCloudsWaterBack2048.png",
        }});

        this->setupController();
    } catch (const std::exception& err) {
        std::cout << err.what() << std::endl;
    }
}

Env::~Env( void ) {
    for (size_t i = 0; i < this->models.size(); ++i)
        delete this->models[i];
    for (size_t i = 0; i < this->raymarchedObjects.size(); ++i)
        delete this->raymarchedObjects[i];
    for (size_t i = 0; i < this->lights.size(); ++i)
        delete this->lights[i];
    delete this->skybox;
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
    // get the size of the framebuffer
    glfwGetFramebufferSize(this->window.ptr, &this->window.width, &this->window.height);
}

void    Env::setupController( void ) {
    this->controller->setKeyProperties(GLFW_KEY_P, eKeyMode::toggle, 1, 1000);
}

void    Env::framebufferSizeCallback( GLFWwindow* window, int width, int height ) {
    glViewport(0, 0, width, height);
}

Light*  Env::getDirectionalLight( void ) {
    for (auto it = this->lights.begin(); it != this->lights.end(); it++)
        if ((*it)->getType() == eLightType::directional)
            return (*it);
    return (nullptr);
}
