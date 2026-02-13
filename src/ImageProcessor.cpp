#include "ImageProcessor.h"
#include "Pixel.h"
#include "isPrime.h"
#include <Filters.h>
#include <cstdint>
#include <iostream>
#include <mdspan>
#include <memory>
#include <span>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageProcessor::ImageProcessor() : width(0), height(0), channels(0), pixelData(nullptr) {
    std::cout << "[C++] ImageProcessor Initialized" << std::endl;
}

ImageProcessor::~ImageProcessor() { delete[] pixelData; }

bool ImageProcessor::loadImage(uintptr_t bufferPtr, int size) {
    const unsigned char* rawData = reinterpret_cast<const unsigned char*>(bufferPtr);
    int tempW, tempH, tempC;
    unsigned char* tempStbData = stbi_load_from_memory(rawData, size, &tempW, &tempH, &tempC, 4);
    if(!tempStbData) {
        std::cerr << "[C++] Failed to load image." << '\n';
        return false;
    }
    if(pixelData) {
        delete[] pixelData;
    }
    width = tempW;
    height = tempH;
    channels = 4;

    pixelData = new unsigned char[width * height * 4];

    std::memcpy(pixelData, tempStbData, width * height * 4);

    stbi_image_free(tempStbData);

    std::cout << "[C++] Loaded Image: " << width << "x" << height << " (RGBA)" << '\n';
    return true;
}
bool ImageProcessor::createPadding(int newWidth, int newHeight, int borderWidth,
                                   std::mdspan<Pixel, std::dextents<size_t, 2>> inputGrid,
                                   std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid) {

    // 1. Create top and bottom borders
    // We iterate through the height of the border (0 to borderWidth)
    for(int i{0}; i < borderWidth; i++) {
        for(int j{0}; j < newWidth; j++) {
            paddedGrid[i, j] = 0;                           // Top Border
            paddedGrid[newHeight - borderWidth + i, j] = 0; // Bottom Border
        }
    }

    // 2. Fill the middle: Left Border + Image Copy + Right Border
    // We iterate from the first valid image row (borderWidth) to the last
    for(int i{borderWidth}; i < newHeight - borderWidth; i++) {

        // Write Left Border
        for(int j{0}; j < borderWidth; j++) {
            paddedGrid[i, j] = 0;
        }

        // Copy Original Image
        // 'k' represents the column in the new Padded Grid
        for(int k{borderWidth}; k < newWidth - borderWidth; k++) {
            // We map the Padded Grid coordinates back to Input Grid coordinates
            // by subtracting the borderWidth
            paddedGrid[i, k] = inputGrid[i - borderWidth, k - borderWidth];
        }

        // Write Right Border
        for(int j{0}; j < borderWidth; j++) {
            paddedGrid[i, newWidth - borderWidth + j] = 0;
        }
    }

    return true;
}
void ImageProcessor::applyFilter(int kernelSize) {
    if(!pixelData) {
        std::cerr << "[C++] Failed to process image." << std::endl;
    }
    // kernel size must be odd and a square => (2n+1) x (2n+1)
    int borderWidth = (kernelSize - 1) / 2;
    bool create_sat{true};
    int newWidth{width + 2 * (borderWidth)};
    int newHeight{height + 2 * (borderWidth)};
    if(create_sat) {
        newWidth=width + 2 * (borderWidth+1);
        newHeight=height + 2 * (borderWidth+1);
    }
    auto paddedData = std::make_unique_for_overwrite<unsigned char[]>(newWidth * newHeight * 4);

    // Height represents Number of Rows
    // Width rerpresents Number of Cols
    std::mdspan inputGrid(reinterpret_cast<Pixel*>(pixelData), height, width);
    std::mdspan paddedGrid(reinterpret_cast<Pixel*>(paddedData.get()), newHeight, newWidth);

    auto SAT = std::make_unique_for_overwrite<uint32_t[]>(4 * newHeight * newWidth);
    std::mdspan satGrid(reinterpret_cast<SatPixel*>(SAT.get()), newHeight, newWidth);

    createPadding(newWidth, newHeight, borderWidth, inputGrid, paddedGrid);
    if(create_sat) {
        // Fill the top and bottom border of the SAT Table
        for(int i{0}; i < borderWidth; i++) {
            for(int j{0}; j < newWidth; j++) {
                satGrid[i, j] = 0;
                satGrid[newHeight - borderWidth + i, j] = 0;
            }
        }

        // Filling in the original image in the middle and also creating the side borders
        for(int i{borderWidth}; i < newHeight - borderWidth; i++) {
            // Fill the Left Border border of the SAT Table
            for(int j{0}; j < borderWidth; j++) {
                satGrid[i, j] = 0;
            }
            // Create the SAT values
            for(int k{borderWidth}; k < newWidth - borderWidth; k++) {
                satGrid[i, k] = paddedGrid[i, k] + satGrid[i - 1, k] + satGrid[i, k - 1] -
                                satGrid[i - 1, k - 1];
            }
            // Fill the Right Border border of the SAT Table
            for(int j{0}; j < borderWidth; j++) {
                satGrid[i, newWidth - borderWidth + j] = 0;
            }
        }

    } else {
        SAT.reset();
    }
    // Iterating through the cells of input grid

    // for(int i{0};i<height;i++){
    // for(int j{0};j<width;j++){
    satBoxBlur(inputGrid, paddedGrid, satGrid, 0, 0,borderWidth);
    // }
    // }

    satPixelData = SAT.release();
    std::cout << "Pixel 0:\t" << (int)inputGrid[0, 0].r << " " << (int)inputGrid[0, 0].g << " "
              << (int)inputGrid[0, 0].b << std::endl;
    std::cout << "Working..." << std::endl;
    std::cout << "Pixel 0:\t" << (int)paddedGrid[1, 1].r << " " << (int)paddedGrid[1, 1].g << " "
              << (int)paddedGrid[1, 1].b << std::endl;
    std::cout << '\n'
              << satGrid[newHeight / 4, newWidth / 4].r << "\t\t"
              << satGrid[newHeight / 2, newWidth / 2].r;
}

int ImageProcessor::getWidth() const { return width; }
int ImageProcessor::getHeight() const { return height; }
uintptr_t ImageProcessor::getPixelDataPtr() const { return reinterpret_cast<uintptr_t>(pixelData); }
