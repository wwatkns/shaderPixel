#include "Env.hpp"

Env::Env( void ) {
    try {
        this->initGlfwEnvironment("4.0");
        // this->initGlfwWindow(480, 480);
        this->initGlfwWindow(720, 480);
        // this->initGlfwWindow(1280, 720);
        // this->initGlfwWindow(2560, 1200);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw Exception::InitError("glad initialization failed");
        this->controller = new Controller(this->window.ptr);
        // this->models = {{
        //     // new Model(
        //     //     "/Users/wwatkins/Downloads/chester-accent-table-bronze-405009/source/405009/405009.obj",
        //     //     glm::vec3(0.0f, 0.0f, 0.0f),
        //     //     glm::vec3(0.0f),
        //     //     glm::vec3(1.0f)
        //     // ),
        //     new Model(
        //         "/Users/wwatkins/Downloads/old-romanic-pilar/source/model/model.obj",
        //         glm::vec3(0.0f, 0.0f, 0.0f),
        //         glm::vec3(0.0f),
        //         glm::vec3(5.0f)
        //     ),
        //     // new Model(
        //     //     "/Users/wwatkins/Downloads/old-romanic-pilar/source/model/model.obj",
        //     //     glm::vec3(4.0f, 0.0f, 0.0f),
        //     //     glm::vec3(0.0f),
        //     //     glm::vec3(5.0f)
        //     // ),
        //     // new Model(
        //     //     "/Users/wwatkins/Downloads/old-romanic-pilar/source/model/model.obj",
        //     //     glm::vec3(8.0f, 0.0f, 0.0f),
        //     //     glm::vec3(0.0f),
        //     //     glm::vec3(5.0f)
        //     // ),
        // }};
        this->raymarched = new Raymarched({
            // (tObject){
            //     eRaymarchObject::ifs,
            //     glm::vec3(0.0, 2.0, 0.0),
            //     glm::vec3(0.0),
            //     0.25,
            //     0.03,
            //     (tMaterial){
            //         glm::vec3(0),
            //         glm::vec3(1.0),
            //         glm::vec3(0.35),
            //         128.0,
            //         1.0
            //     }
            // },
            // (tObject){
            //     eRaymarchObject::mandelbox,
            //     glm::vec3(0.0, 4.0, 0.0),
            //     glm::vec3(0.0),
            //     0.5,
            //     0.015,
            //     (tMaterial){
            //         glm::vec3(0.0),
            //         glm::vec3(0.0),
            //         glm::vec3(0.659, 1.0, 0.537),
            //         50.0,
            //         1.0
            //     }
            // },
            // (tObject){
            //     eRaymarchObject::mandelbulb,
            //     glm::vec3(4.0, 4.0, 0.0),
            //     glm::vec3(0.0),
            //     0.5,
            //     0.1,
            //     (tMaterial){
            //         glm::vec3(0.0),
            //         glm::vec3(0.0),
            //         glm::vec3(0.35, 0.35, 0.35),
            //         82.0,
            //         1.0
            //     }
            // },
            // (tObject){
            //     eRaymarchObject::toruscloud,,
            //     glm::vec3(8.0, 3.5, 0.0),
            //     glm::vec3(0.0),
            //     0.1,
            //     1.0,
            //     (tMaterial){
            //         glm::vec3(0.0),
            //         glm::vec3(1.0),
            //         glm::vec3(1.0, 1.0, 1.0),
            //         128.0,
            //         1.0
            //     }
            // }
            (tObject){
                eRaymarchObject::cloud,
                glm::vec3(0.0, 0.0, 0.0),
                glm::vec3(0.0),
                1.0,
                1.0,
                (tMaterial){
                    glm::vec3(0.0),
                    glm::vec3(1.0),
                    glm::vec3(1.0),
                    2048.0,
                    1.0
                }
            }
        });
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
    for (size_t i = 0; i < this->lights.size(); ++i)
        delete this->lights[i];
    delete this->skybox;
    delete this->controller;
    delete this->raymarched;
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
    // glfwWindowHint(GLFW_SAMPLES, 4);
}

void	Env::initGlfwWindow( size_t width, size_t height ) {
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
