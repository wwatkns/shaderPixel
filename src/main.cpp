#include "Renderer.hpp"
#include "Env.hpp"

int main( void ) {
    try {
        Env         environment;
        Renderer    renderer(&environment);
        renderer.loop();
    }
    catch (const std::exception& err) {
        std::cout << err.what() << std::endl;
    }
    return (0);
}
