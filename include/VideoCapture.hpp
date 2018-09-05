#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/videoio/videoio.hpp>
#include <opencv2/imgproc.hpp>

enum class eCodec {
    jpeg,
    mp4v,
    avc1
};

class VideoCapture {

public:
    VideoCapture( const std::string source, int width, int height, int framerate, eCodec codec=eCodec::avc1, float recordingTime=0. );
    ~VideoCapture( void );
 
    void	        write( bool vflip=true );

private:
    cv::VideoWriter videoWriter;
    cv::Size        size;
    cv::Mat         frame;
    float           framerate;
    int             maxFrames;
    int             currentFrame;

    uint8_t*        data;

    int             getCodecFourcc(eCodec codec);
};