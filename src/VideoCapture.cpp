#include "VideoCapture.hpp"

VideoCapture::VideoCapture( const std::string source, int width, int height, int framerate, eCodec codec, float recordingTime ) {
    /* source extension should be .mov */
    this->framerate = framerate;
    this->maxFrames = (int)(framerate * recordingTime);
    this->currentFrame = 0;
    this->size = cv::Size(width, height);

    this->data = (uint8_t*)malloc(this->size.width * this->size.height * 3);

    this->videoWriter.open(source, VideoCapture::getCodecFourcc(codec), framerate, this->size, true);
    if (!videoWriter.isOpened())
        std::cout  << "Could not open the output video for write: " << source << std::endl;
    this->videoWriter.set(cv::VIDEOWRITER_PROP_QUALITY, 100.0);
}

VideoCapture::~VideoCapture( void ) {
    if (this->videoWriter.isOpened())
        this->videoWriter.release();
}

void    VideoCapture::write( bool vflip ) {
    if (!this->videoWriter.isOpened())
        return ;
    /* end capture when max recording time is reached */
    if (this->videoWriter.isOpened() && this->currentFrame > this->maxFrames) {
        this->videoWriter.release();
        std::cout << "> Capture completed" << std::endl;
    }
    /* capture frame from openGL */
    if (this->videoWriter.isOpened()) {
        
        glReadPixels(0, 0, (GLsizei)this->size.width, (GLsizei)this->size.height, GL_BGR, GL_UNSIGNED_BYTE, data);
        cv::Mat tmp(this->size.height, this->size.width, CV_8UC3, data);
        this->frame = tmp;

        if (vflip)
            cv::flip(this->frame, this->frame, 0);
        this->videoWriter.write(this->frame);
        this->currentFrame++;
    }
}

int    VideoCapture::getCodecFourcc(eCodec codec) {
     /* https://gist.github.com/takuma7/44f9ecb028ff00e2132e */
    const int fourcc[3] = {
        cv::VideoWriter::fourcc('j','p','e','g'), // .mov       (apple photo, jpeg iso standard)
        cv::VideoWriter::fourcc('m','p','4','v'), // .mov/.mp4  (apple MPEG4)
        cv::VideoWriter::fourcc('a','v','c','1')  // .mov/.mp4  (h264), best and default
    };
    if (int(codec) < 0 || int(codec) > 2)
        return fourcc[2];
    return fourcc[int(codec)];
}
