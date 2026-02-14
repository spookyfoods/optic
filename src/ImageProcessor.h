
#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H
#include "Pixel.h"
#include <cstdint>
#include <mdspan>
#include <string>

class ImageProcessor {
  private:
    int width;
    int height;
    int channels;
    unsigned char* pixelData;
    uint32_t* satPixelData;

    using paddedDataAndGrid =
        std::pair<std::unique_ptr<unsigned char[]>, std::mdspan<Pixel, std::dextents<size_t, 2>>>;
    paddedDataAndGrid createPadding(int newWidth, int newHeight, int borderWidth,
                                    std::mdspan<Pixel, std::dextents<size_t, 2>> inputGrid);
    using satDataAndGrid = std::pair<std::unique_ptr<uint32_t[]>,
                                     std::mdspan<SatPixel, std::dextents<size_t, 2>>>;
    satDataAndGrid computeSAT(int newWidth, int newHeight, int borderWidth,
                                std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid);

  public:
    ImageProcessor();
    ~ImageProcessor();

    bool loadImage(uintptr_t bufferPtr, int size);

    void applyFilter(int kernelSize, std::string filterType);

    int getWidth() const;
    int getHeight() const;
    uintptr_t getPixelDataPtr() const;
};
#endif
