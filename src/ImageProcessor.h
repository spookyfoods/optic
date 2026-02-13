
#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H
#include <cstdint>
#include <string>
#include <mdspan>
#include "Pixel.h"

class ImageProcessor {
private:
    int width;
    int height;
    int channels;
    unsigned char* pixelData;
    uint32_t* satPixelData;

public:
    ImageProcessor();
    ~ImageProcessor();


    bool loadImage(uintptr_t bufferPtr, int size);
    bool createPadding  ( int newWidth, int newHeight, int borderWidth,
                        std::mdspan<Pixel, std::dextents<size_t, 2>>     inputGrid,
                        std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid
                        );

    void applyFilter(int kernelSize, std::string filterType);

    int getWidth() const;
    int getHeight() const;
    uintptr_t getPixelDataPtr() const;
};
#endif
