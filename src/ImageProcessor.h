#pragma once
#include <cstdint>
#include <string>

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


    void applyFilter(int kernelSize);

    int getWidth() const;
    int getHeight() const;
    uintptr_t getPixelDataPtr() const;
};
