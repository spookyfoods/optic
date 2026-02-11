#include <memory>
#include <cstdint>
#include <span>
#include <mdspan>
#include "ImageProcessor.h"
#include <iostream>
#include "isPrime.h"
#include "Pixel.h"
#include <Filters.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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

    std::mdspan inputGrid(reinterpret_cast<Pixel*>(pixelData), height, width);

    int newWidth{width+2};
    int newHeight{height+2};
    auto borderedPixelData = std::make_unique_for_overwrite<unsigned char[]>(newWidth * newHeight * 4);
    
    std::mdspan outputGrid(reinterpret_cast<Pixel*>(borderedPixelData.get()), newHeight, newWidth);


    for(int i{};i<newWidth;i++){
        outputGrid[0, i] = 0;
        outputGrid[newHeight - 1, i] = 0;
    }

    for(int i{};i<height;i++){
        outputGrid[i + 1, 0] = 0;
        outputGrid[i + 1, newWidth - 1] = 0;
        for(int j{};j<width;j++){

            outputGrid[i + 1, j + 1] = inputGrid[i, j]; 
        }
    }
    this->height=newHeight;
    this->width=newWidth;

    for(int i{1};i<height-1;i++){
        for(int j{1};j<width-1;j++){
            placeholderFilter(inputGrid,outputGrid,i,j);
        }
    }

    std::cout << "Pixel 0:\t" << (int)inputGrid[0, 0].r<<" "<<(int)inputGrid[0, 0].g<<" "<<(int)inputGrid[0, 0].b << std::endl;
    std::cout << "Working..." << std::endl;
    pixelData=borderedPixelData.release();
    std::cout << "Pixel 0:\t" << (int)outputGrid[1, 1].r<<" "<<(int)outputGrid[1, 1].g<<" "<<(int)outputGrid[1, 1].b << std::endl;
}

int ImageProcessor::getWidth() const { return width; }
int ImageProcessor::getHeight() const { return height; }
uintptr_t ImageProcessor::getPixelDataPtr() const { return reinterpret_cast<uintptr_t>(pixelData); }
