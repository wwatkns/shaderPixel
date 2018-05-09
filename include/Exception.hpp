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

    class MatrixError : public std::exception {

    public:
        MatrixError( const std::string& str ) {
            std::stringstream ss;
            ss << "MatrixError : " << str;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };

    class MatrixAccessError : public std::exception {

    public:
        MatrixAccessError( size_t j, size_t i, size_t h, size_t w ) {
            std::stringstream ss;
            ss << "MatrixAccessError : index [" << j << "," << i << "] is invalid, dimensions are ("
            << h << "," << w << ")";
            msg = ss.str();
        }
        MatrixAccessError( size_t i, size_t size ) {
            std::stringstream ss;
            ss << "MatrixAccessError : index [" << i << "] is invalid, size is " << size;
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };

    class MatrixTypeError : public std::exception {

    public:
        MatrixTypeError( size_t h, size_t w ) {
            std::stringstream ss;
            ss << "MatrixTypeError : matrix has invalid dims, " << h << " != " << w
            << ", should be N x N";
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };

    class MatrixOperationError : public std::exception {

    public:
        MatrixOperationError( size_t aSize, size_t bSize ) {
            std::stringstream ss;
            ss << "MatrixOperationError : matrices have invalid sizes, " << aSize << " and " << bSize;
            msg = ss.str();
        }
        MatrixOperationError( size_t aH, size_t aW, size_t bH, size_t bW ) {
            std::stringstream ss;
            ss << "MatrixOperationError : matrices have invalid dimensions, (" << aH << "," << aW
            << ") and (" << bH << "," << bW << ")";
            msg = ss.str();
        }
        virtual const char* what() const noexcept {
            return (msg.c_str());
        }

    private:
        std::string msg;
    };

    class SkeletonMapAccessError : public std::exception {

    public:
        SkeletonMapAccessError( const std::string& id ) {
            std::stringstream ss;
            ss << "SkeletonMapAccessError : id " << id << " is invalid, skeleton does not contain such a key";
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
