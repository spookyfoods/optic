
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
    std::unique_ptr<uint8_t> pixelDataU{};
    unsigned char* pixelData;
    uint32_t* satPixelData;

    enum class SatMethod { SERIAL, WAVEFRONT_PIPELINE, TWO_PASS_BARRIER };
    
    using paddedDataAndGrid =
        std::pair<std::unique_ptr<unsigned char[]>, std::mdspan<Pixel, std::dextents<size_t, 2>>>;
    paddedDataAndGrid createPadding(int newWidth, int newHeight, int borderWidth,
                                    std::mdspan<Pixel, std::dextents<size_t, 2>> inputGrid);
    using satDataAndGrid =
        std::pair<std::unique_ptr<uint32_t[]>, std::mdspan<SatPixel, std::dextents<size_t, 2>>>;
    satDataAndGrid computeSAT(int newWidth, int newHeight, int borderWidth,
                              std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid,
                              ImageProcessor::SatMethod processingType=ImageProcessor::SatMethod::SERIAL);

  public:
    ImageProcessor();
    ~ImageProcessor();

    bool loadImage(std::vector<char> buffer, int size);

    void applyFilter(int kernelSize, std::string filterType);

    int getWidth() const;
    int getHeight() const;
    uintptr_t getPixelDataPtr() const;
};
#endif
