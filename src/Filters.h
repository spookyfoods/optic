#ifndef FILTERS_H
#define FILTERS_H

#include "ImageProcessor.h"
#include "Kernel.h"
#include "Pixel.h"
#include <mdspan>

namespace {
template <typename T> class ConvolutionState {
  private:
    // Holds the math rules
    const Kernel<T>& kernel;

    // Holds the memory position (The iterator aspect)
    std::span<const T> weights;
    size_t currentIndex = 0;

    // Holds the running totals
    float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;

  public:
    ConvolutionState(const Kernel<T>& k) : kernel(k), weights(k.matrix) {}

    // Consumer method
    inline void consume(const Pixel& neighbor) {
        T weight = weights[currentIndex++];

        sumR += neighbor.r * weight;
        sumG += neighbor.g * weight;
        sumB += neighbor.b * weight;
    }

    // The Resolution method
    inline Pixel resolve() const {
        return Pixel{static_cast<uint8_t>(
                         std::clamp(static_cast<int>(sumR / kernel.normalizationFactor), 0, 255)),
                     static_cast<uint8_t>(
                         std::clamp(static_cast<int>(sumG / kernel.normalizationFactor), 0, 255)),
                     static_cast<uint8_t>(
                         std::clamp(static_cast<int>(sumB / kernel.normalizationFactor), 0, 255)),
                     255};
    }
};
} // namespace

template <typename T>
void applyKernel(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                 const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                 size_t inputGridRowNum, size_t inputGridColNum, const Kernel<T>& kernel) {

    int halfW = kernel.width / 2;
    int halfH = kernel.height / 2;

    int paddedGridRowNum = inputGridRowNum + halfH;
    int paddedGridColNum = inputGridColNum + halfW;

    ConvolutionState<T> state(kernel);

    for(int i = -halfH; i <= halfH; i++) {
        for(int j = -halfW; j <= halfW; j++) {
            state.consume(paddedGrid[paddedGridRowNum + i, paddedGridColNum + j]);
        }
    }

    inputGrid[inputGridRowNum, inputGridColNum] = state.resolve();
}
void naiveBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                  const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                  size_t inputGridRowNum, size_t inputGridColNum);
void satBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                const std::mdspan<SatPixel, std::dextents<size_t, 2>>& satGrid,
                size_t inputGridRowNum, size_t inputGridColNum);
#endif
