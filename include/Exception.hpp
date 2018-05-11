#pragma once

#include <glad/glad.h>
#include <sstream>
#include <string>

namespace Exception {
    class InitError : public std::exception {

    public:
        InitError( const std::string& str ) {
            std::stringstream ss;
            ss << "InitError : " << str;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };

    class ShaderError : public std::exception {

    public:
        ShaderError( int type, const std::string& err ) {
            std::stringstream ss;
            if (type == GL_VERTEX_SHADER)
                ss << "SHADER::VERTEX::COMPILATION_FAILED_";
            else if (type == GL_FRAGMENT_SHADER)
                ss << "SHADER::FRAGMENT::COMPILATION_FAILED_";
            else
                ss << "SHADER::PROGRAM::COMPILATION_FAILED_";
            ss << err;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };


    class ModelError : public std::exception {

    public:
        ModelError( const std::string& type, const std::string& str ) {
            std::stringstream ss;
            ss << "ModelError::" << type << ": " << str;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };


    class RuntimeError : public std::exception {

    public:
        RuntimeError( const std::string& str ) {
            std::stringstream ss;
            ss << "RuntimeError : " << str;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };


}
