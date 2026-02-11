#include <memory>
#include <cstdint>
#include <span>
#include "ImageProcessor.h"
#include <iostream>
#include "isPrime.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct Pixel{
    uint8_t r,g,b,a;
    Pixel operator+(int value){
        r+=value;
        g+=value;
        b+=value;
        return *this;
    }
    Pixel& operator=(const Pixel& other){
        r=other.r;
        g=other.g;
        b=other.b;
        a=other.a;
        return *this;
    }
    Pixel& operator=(int assignmentValue){
        r=assignmentValue;
        g=assignmentValue;
        b=assignmentValue;
        a=assignmentValue;
        return *this;
    }
};

ImageProcessor::ImageProcessor() : width(0), height(0), channels(0), pixelData(nullptr) {
    std::cout << "[C++] ImageProcessor Initialized" << std::endl;
}

ImageProcessor::~ImageProcessor() {
    delete[] pixelData;
}

bool ImageProcessor::loadImage(uintptr_t bufferPtr, int size) {
    const unsigned char* rawData = reinterpret_cast<const unsigned char*>(bufferPtr);
    int tempW, tempH, tempC;
    unsigned char* tempStbData = stbi_load_from_memory(
        rawData,
        size,
        &tempW,
        &tempH,
        &tempC,
        4
    );
    if (!tempStbData) {
        std::cerr << "[C++] Failed to load image." << '\n';
        return false;
    }
    if (pixelData) {
        delete[] pixelData;
    }
    width = tempW;
    height = tempH;
    channels = 4;

    pixelData = new unsigned char[width * height * 4];

    std::memcpy(pixelData, tempStbData, width * height * 4);

    stbi_image_free(tempStbData);
    
    std::cout << "[C++] Loaded Image: " << width << "x" << height << " (RGBA)" <<'\n';
    return true;
}

void ImageProcessor::applyFilter() {
    if(!pixelData){
        std::cerr << "[C++] Failed to process image." << std::endl;
    }

    std::span<Pixel> perPixelInterface(reinterpret_cast<Pixel*>(pixelData),width*height);

    int newWidth{width+2};
    int newHeight{height+2};
    auto borderedPixelData = std::make_unique_for_overwrite<unsigned char[]>(newWidth * newHeight * 4);
    std::span<Pixel> borderedPerPixelInterface(reinterpret_cast<Pixel*>(borderedPixelData.get()),newWidth*newHeight);
    for(int i{};i<newWidth;i++){
        borderedPerPixelInterface[i]=0;
        borderedPerPixelInterface[(newHeight-1)*newWidth + i]=0;
    }
    for(int i{};i<height;i++){
        borderedPerPixelInterface[newWidth*(i+1)]=0;
        borderedPerPixelInterface[(i+1) * newWidth + (newWidth - 1)]=0;
        for(int j{};j<width;j++){
            borderedPerPixelInterface[(i + 1) * newWidth + (j + 1)]=perPixelInterface[width*i+j];
        }
    }
    this->height=newHeight;
    this->width=newWidth;
    std::cout << "Pixel 0:\t" << (int)perPixelInterface[0].r<<" "<<(int)perPixelInterface[0].g<<" "<<(int)perPixelInterface[0].b << std::endl;
    std::cout << "Pixel 0:\t" << (int)perPixelInterface[0].r<<" "<<(int)perPixelInterface[0].g<<" "<<(int)perPixelInterface[0].b << std::endl;
    std::cout << "Working..." << std::endl;
    pixelData=borderedPixelData.release();
}

int ImageProcessor::getWidth() const { return width; }
int ImageProcessor::getHeight() const { return height; }
uintptr_t ImageProcessor::getPixelDataPtr() const { return reinterpret_cast<uintptr_t>(pixelData); }
