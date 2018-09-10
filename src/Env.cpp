#include "Env.hpp"

/* Models from :
    https://sketchfab.com/models/192bf30a7e28425ab385aef19769d4b0
    https://sketchfab.com/models/5cfc211a49164bf2835a121b5069ee08
*/

Env::Env( void ) {
    try {
        this->initGlfwEnvironment("4.0");
        // this->initGlfwWindow(720, 480); /* 1280x720 */
        this->initGlfwWindow(960, 540); /* 1920x1080 */
        // this->initGlfwWindow(1280, 720); /* 2560x1440 */
        // this->initGlfwWindow(1920, 1080); /* 3840x2160 */
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
            throw Exception::InitError("glad initialization failed");
        this->controller = new Controller(this->window.ptr);

        this->models.push_back( new Model(
            "./resource/models/Inn/theInn.FBX.obj",
            glm::vec3(0.0f, 2.0f, 0.0f),
            glm::vec3(0.0f),
            glm::vec3(75.0f)
        ));
        this->models.push_back( new Model( /* stand emplacement 1 - Marble */
            "./resource/models/Crystal/crystal.obj",
            glm::vec3(10.0f, -0.75f, 14.f),
            glm::vec3(0.0f),
            glm::vec3(2.5f)
        ));
        this->models.push_back( new Model( /* stand emplacement 2 - IFS */
            this->models[1]->getMeshes(),
            this->models[1]->getTextures(),
            glm::vec3(11.3f, 1.15f, -17.65f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(3.0f)
        ));
        this->models.push_back( new Model( /* stand emplacement 3 - Mandelbulb */
            this->models[1]->getMeshes(),
            this->models[1]->getTextures(),
            glm::vec3(-15.f, -1.5f, 3.1f),
            glm::vec3(0.0f, 0.5f, 0.0f),
            glm::vec3(2.5f)
        ));
        this->models.push_back( new Model( /* stand emplacement 4 - Cloud */
            this->models[1]->getMeshes(),
            this->models[1]->getTextures(),
            glm::vec3(-27.6f, 3.53f, -22.55f),
            glm::vec3(0.0f, 2.5f, 0.0f),
            glm::vec3(2.5f)
        ));
        this->models.push_back( new Model( /* stand emplacement 5 - Mandelbox */
            this->models[1]->getMeshes(),
            this->models[1]->getTextures(),
            glm::vec3(-27.6f, -2.75f, 12.5f),
            glm::vec3(0.0f, 2.0f, 0.0f),
            glm::vec3(2.5f, 2.0f, 2.5f)
        ));
        this->models.push_back( new Model( /* quad for raymarched surface (to compute shadowMap correclty) */
            glm::vec3(17.01f, 0.58f, 28.75f),
            glm::vec3(0.0f, 3.1415926536f * 0.5f, 0.0f),
            glm::vec3(4.3f, 11.5f, 0.0f)
        ));

        this->raymarched = new Raymarched({
            (tObject){
                eRaymarchObject::marble,
                glm::vec3(10.0f, 2.5f, 14.f),
                glm::vec3(0.0),
                1.0, // scale
                1.0, // bs scale
                1.0, // speed modifier
                (tMaterial){ glm::vec3(0.0), glm::vec3(1.0), glm::vec3(1.0), 2048.0, 1.0 }
            },
            (tObject){
                eRaymarchObject::cloud,
                glm::vec3(-27.6f, 7.73f, -22.55f),
                glm::vec3(0.0),
                2.0,
                1.0,
                1.0,
                (tMaterial){ glm::vec3(0.0), glm::vec3(1.0), glm::vec3(0.0), 1.0, 1.0 }
            },
            (tObject){
                eRaymarchObject::ifs,
                glm::vec3(11.3f, 5.0f, -17.65f),
                glm::vec3(0.0),
                0.5,
                3.0,
                0.03,
                (tMaterial){
                    glm::vec3(0),
                    glm::vec3(1.0),
                    glm::vec3(0.35),
                    128.0,
                    1.0
                }
            },
            (tObject){
                eRaymarchObject::mandelbox,
                glm::vec3(-27.6f, 0.f, 12.5f),
                glm::vec3(0.0),
                0.5,
                2.0,
                0.015,
                (tMaterial){
                    glm::vec3(0.0),
                    glm::vec3(0.0),
                    glm::vec3(1., 0.659, 0.537),
                    256.0,
                    1.0
                }
            },
            (tObject){
                eRaymarchObject::mandelbulb,
                glm::vec3(-15.f, 2.0f, 3.1f),
                glm::vec3(0.0),
                1.0,
                1.15,
                0.1,
                (tMaterial){
                    glm::vec3(0.0),
                    glm::vec3(0.0),
                    glm::vec3(0.35, 0.35, 0.35),
                    82.0,
                    1.0
                }
            },
            // (tObject){
            //     eRaymarchObject::torus,
            //     glm::vec3(1.0, 1.5, 0.0),
            //     glm::vec3(0.0),
            //     0.1,
            //     1.0,
            //     (tMaterial){
            //         glm::vec3(0.0),
            //         glm::vec3(1.0),
            //         glm::vec3(0.0, 0.0, 1.0),
            //         128.0,
            //         1.0
            //     }
            // }

        });
        this->raymarchedSurfaces.push_back( new RaymarchedSurface(
            glm::vec3(17.0f, 0.58f, 28.75f),
            glm::vec3(0.0f, 3.1415926536f * 0.5f, 0.0f),
            glm::vec3(4.3f, 11.5f, 0.0f)
        ));
        this->raymarchedSurfaces.push_back( new RaymarchedSurface(
            glm::vec3(16.8f, 5.85f, -18.215f),
            glm::vec3(0.0f, 3.1415926536f * 0.5f, 0.0f),
            glm::vec3(2.825f, 3.45f, 0.0f)
        ));
        this->texturedSurfaces.push_back( new RaymarchedSurface(
            glm::vec3(16.88f, 4.8f, -6.015f),
            glm::vec3(0.0f, 3.1415926536f * 0.5f, 0.0f),
            glm::vec3(10.f, 13.f, 0.0f)
        ));
        this->lights = {{
            new Light(
                glm::vec3(30, 30, 18), //glm::vec3(10, 10, 6),
                glm::vec3(0.77f, 0.88f, 1.0f) * 0.075f,
                glm::vec3(1.0f, 0.964f, 0.77f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                eLightType::directional
            ),
            /* lights from fractals stands - it's slow though... */
            // new Light(
            //     glm::vec3(10.0f, 1.f, 14.f),
            //     glm::vec3(0.0f, 0.0f, 0.0f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     1.0f,
            //     0.18f,
            //     0.064f,
            //     eLightType::point
            // ),
            // new Light(
            //     glm::vec3(11.3f, 2.9f, -17.65f),
            //     glm::vec3(0.0f, 0.0f, 0.0f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     1.0f,
            //     0.18f,
            //     0.064f,
            //     eLightType::point
            // ),
            // new Light(
            //     glm::vec3(-15.f, 0.4f, 3.1f),
            //     glm::vec3(0.0f, 0.0f, 0.0f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     1.0f,
            //     0.18f,
            //     0.064f,
            //     eLightType::point
            // ),
            // new Light(
            //     glm::vec3(-27.6f, 4.43f, -22.55f),
            //     glm::vec3(0.0f, 0.0f, 0.0f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     glm::vec3(0.937f, 0.474f, 0.212f),
            //     1.0f,
            //     0.18f,
            //     0.064f,
            //     eLightType::point
            // ),
            // // new Light(
            // //     glm::vec3(-27.6f, -0.85f, 12.5f),
            // //     glm::vec3(0.0f, 0.0f, 0.0f),
            // //     glm::vec3(0.937f, 0.474f, 0.212f),
            // //     glm::vec3(0.937f, 0.474f, 0.212f),
            // //     1.0f,
            // //     0.18f,
            // //     0.064f,
            // //     eLightType::point
            // // )
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
