#include "ImageProcessor.h"
#include "Pixel.h"
#include <Filters.h>
#include <cstdint>
#include <iostream>
#include <mdspan>
#include <memory>
#include <span>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

ImageProcessor::ImageProcessor() : width(0), height(0), channels(0), pixelData(nullptr) {
    std::cout << "[C++] ImageProcessor Initialized" << std::endl;
}

ImageProcessor::~ImageProcessor() { delete[] pixelData; }

ImageProcessor::satDataAndGrid
ImageProcessor::computeSAT(int newWidth, int newHeight, int borderWidth,
                           std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid, bool parallel) {
    auto satData = std::make_unique_for_overwrite<uint32_t[]>(4 * newHeight * newWidth);
    std::mdspan satGrid(reinterpret_cast<SatPixel*>(satData.get()), newHeight, newWidth);
    if(!parallel) {
        std::cout << "linear sat\n";

        // Write ZERO to the top border and left border
        for(int i{0}; i < borderWidth; i++) {
            for(int j{0}; j < newWidth; j++) {
                // Top Border
                satGrid[i, j] = 0;
            }
        }
        for(int i{borderWidth}; i < newHeight - borderWidth; i++) {
            for(int j{0}; j < borderWidth; j++) {
                // Write Left Border
                satGrid[i, j] = 0;

            }
        }
        // Create the SAT values of all the middle pixels and the right and bottom border
        // excluding the very last rows and columns, because they won't be accessed
        for(int i{borderWidth}; i < newHeight; i++) {
            for(int k{borderWidth}; k < newWidth; k++) {
                satGrid[i, k] = paddedGrid[i, k] + satGrid[i - 1, k] + satGrid[i, k - 1] -
                                satGrid[i - 1, k - 1];
            }
        }
    } else {
        std::cout << "parallel sat\n";
    }

    return std::make_pair(std::move(satData), satGrid);
}

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
ImageProcessor::paddedDataAndGrid
ImageProcessor::createPadding(int newWidth, int newHeight, int borderWidth,
                              std::mdspan<Pixel, std::dextents<size_t, 2>> inputGrid) {

    auto paddedData = std::make_unique_for_overwrite<unsigned char[]>(newWidth * newHeight * 4);
    std::mdspan paddedGrid(reinterpret_cast<Pixel*>(paddedData.get()), newHeight, newWidth);
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

    return std::make_pair(std::move(paddedData), paddedGrid);
}
void ImageProcessor::applyFilter(int kernelSize, std::string filterType) {
    if(!pixelData) {
        std::cerr << "[C++] Failed to process image." << std::endl;
    }
    // kernel size must be odd and a square => (2n+1) x (2n+1)
    bool create_sat{filterType == "sat"};
    int borderWidth = ((kernelSize - 1) / 2) + static_cast<int>(create_sat);
    int newWidth{width + 2 * (borderWidth)};
    int newHeight{height + 2 * (borderWidth)};

    // Height represents Number of Rows
    // Width rerpresents Number of Cols
    std::mdspan inputGrid(reinterpret_cast<Pixel*>(pixelData), height, width);

    auto [paddedData, paddedGrid] = createPadding(newWidth, newHeight, borderWidth, inputGrid);

    std::cout << "\nInput Pix[0,0]:\t" << (int)inputGrid[0, 0].r << " " << (int)inputGrid[0, 0].g
              << " " << (int)inputGrid[0, 0].b << "\n";

    // For iterating through the cells of input grid
    auto traverse = [&](auto operation) {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                operation(i, j);
            }
        }
    };

    if(create_sat) {
        auto [satData, satGrid] = computeSAT(newWidth, newHeight, borderWidth, paddedGrid, false);
        std::cout << "\nRUNNING SAT BOX BLUR" << std::endl;
        traverse([&](int i, int j) { satBoxBlur(inputGrid, satGrid, i, j); });
    } else {
        std::cout << "\nRUNNING NAIVE BOX BLUR" << std::endl;
        traverse([&](int i, int j) { naiveBoxBlur(inputGrid, paddedGrid, i, j); });
    }
    std::cout << "\nInput Pix[0,0]:\t" << (int)inputGrid[0, 0].r << " " << (int)inputGrid[0, 0].g
              << " " << (int)inputGrid[0, 0].b << "\n";
}

int ImageProcessor::getWidth() const { return width; }
int ImageProcessor::getHeight() const { return height; }
uintptr_t ImageProcessor::getPixelDataPtr() const { return reinterpret_cast<uintptr_t>(pixelData); }
