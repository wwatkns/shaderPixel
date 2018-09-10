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

cv::Mat correctGamma( cv::Mat& img, double gamma ) {
    double inverse_gamma = 1.0 / gamma;

    cv::Mat lut_matrix(1, 256, CV_8UC1);
    uchar * ptr = lut_matrix.ptr();
    for( int i = 0; i < 256; i++ )
        ptr[i] = (int)( pow( (double) i / 255.0, inverse_gamma ) * 255.0 );

    cv::Mat result;
    cv::LUT( img, lut_matrix, result );
    return result;
}

cv::Mat sRGBtoLinear( cv::Mat& img ) {
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
    cv::Mat lut_matrix(1, 256, CV_8UC1);
    uchar * ptr = lut_matrix.ptr();
    for( int i = 0; i < 256; i++ ) {
        double cs = (double)i / 255.;
        ptr[i] = (int)((cs <= 0.04045 ? cs / 12.92 : cv::pow((cs + 0.055)/1.055, 2.4) ) * 255.0);
    }
    cv::Mat res;
    cv::LUT( img, lut_matrix, res );
    return res;
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
        // this->frame = sRGBtoLinear(tmp);
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
