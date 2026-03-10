#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ImageProcessor.h"
#include "Pixel.h"
#include <iostream>
#include <mdspan>
#include <vector>

void sharpenFilter(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                   const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                   size_t inputGridRowNum, size_t inputGridColNum) {
    // 3x3 Sharpen Kernel
    const int kernel[3][3] = {
        { 0, -1,  0},
        {-1,  5, -1},
        { 0, -1,  0}
    };

    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;
    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    // Iterate over the 3x3 kernel area
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            const Pixel& p = paddedGrid[paddedGridRowNum + i, paddedGridColNum + j];
            int weight = kernel[i + 1][j + 1];

            sumR += p.r * weight;
            sumG += p.g * weight;
            sumB += p.b * weight;
        }
    }

    // Clamp results to 0-255 range (Sharpening can produce negative values or >255)
    inputGrid[inputGridRowNum, inputGridColNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB, 0, 255)),
        255
    };
}

void edgeDetectionFilter(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                         const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                         size_t inputGridRowNum, size_t inputGridColNum) {
    // 3x3 Edge Detection Kernel
    const int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;
    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            const Pixel& p = paddedGrid[paddedGridRowNum + i, paddedGridColNum + j];
            int weight = kernel[i + 1][j + 1];

            sumR += p.r * weight;
            sumG += p.g * weight;
            sumB += p.b * weight;
        }
    }

    inputGrid[inputGridRowNum, inputGridColNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB, 0, 255)),
        255
    };
}

void gaussianBlurFilter(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                        const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                        size_t inputGridRowNum, size_t inputGridColNum) {
    // 3x3 Gaussian Kernel
    const int kernel[3][3] = {
        {1, 2, 1},
        {2, 4, 2},
        {1, 2, 1}
    };
    
    // The sum of weights is 16, so we divide by 16 at the end
    const int weightSum = 16; 

    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;
    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            const Pixel& p = paddedGrid[paddedGridRowNum + i, paddedGridColNum + j];
            int weight = kernel[i + 1][j + 1];

            sumR += p.r * weight;
            sumG += p.g * weight;
            sumB += p.b * weight;
        }
    }

    // Normalize by dividing by the sum of weights
    inputGrid[inputGridRowNum, inputGridColNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR / weightSum, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG / weightSum, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB / weightSum, 0, 255)),
        255
    };
}

void embossFilter(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                  const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                  size_t inputGridRowNum, size_t inputGridColNum) {
    // 3x3 Emboss Kernel
    const int kernel[3][3] = {
        {-2, -1,  0},
        {-1,  1,  1},
        { 0,  1,  2}
    };

    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;
    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            const Pixel& p = paddedGrid[paddedGridRowNum + i, paddedGridColNum + j];
            int weight = kernel[i + 1][j + 1];

            sumR += p.r * weight;
            sumG += p.g * weight;
            sumB += p.b * weight;
        }
    }

    // Emboss often results in low values; clamping handles the negatives.
    // Sometimes an offset (e.g., +128) is added to make the base grey, 
    // but here we stick to the standard kernel result clamped.
    inputGrid[inputGridRowNum, inputGridColNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB, 0, 255)),
        255
    };
}


void naiveBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                  const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                  size_t inputGridRowNum, size_t inputGridColNum) {
    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;


    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    // paddedGrid Equivalent for a Pixel A on the inputGrid => (rowNum+borderWidth) ,
    // (colNum+borderWidth)
    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    for(int i{paddedGridRowNum - borderWidth}; i <= paddedGridRowNum + borderWidth; i++) {
        for(int j{paddedGridColNum - borderWidth}; j <= paddedGridColNum + borderWidth; j++) {
            sumR += paddedGrid[i, j].r;
            sumG += paddedGrid[i, j].g;
            sumB += paddedGrid[i, j].b;
        }
    }
    sumR /= (2 * borderWidth + 1) * (2 * borderWidth + 1);
    sumG /= (2 * borderWidth + 1) * (2 * borderWidth + 1);
    sumB /= (2 * borderWidth + 1) * (2 * borderWidth + 1);
    inputGrid[inputGridRowNum, inputGridColNum] =
        Pixel{static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
              static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
              static_cast<uint8_t>(std::clamp(sumB, 0, 255)), 255};
}
void satBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                const std::mdspan<SatPixel, std::dextents<size_t, 2>>& satGrid,
                size_t inputGridRowNum, size_t inputGridColNum) {

    // paddedGrid Equivalent for a Pixel A on the inputGrid => (rowNum+borderWidth) ,
    // (colNum+borderWidth)

    int borderWidth = (satGrid.extent(0) - inputGrid.extent(0))/2;
    int radius = borderWidth-1;
    int area = (2*radius+1) * (2*radius+1);

    int paddedGridRowNum = inputGridRowNum + borderWidth;
    int paddedGridColNum = inputGridColNum + borderWidth;

    int r1 = paddedGridRowNum - radius;
    int c1 = paddedGridColNum - radius;
    int r2 = paddedGridRowNum + radius;
    int c2 = paddedGridColNum + radius;

    const SatPixel& p1 = satGrid[r2, c2];
    const SatPixel& p2 = satGrid[r2, c1 - 1];
    const SatPixel& p3 = satGrid[r1 - 1, c2];
    const SatPixel& p4 = satGrid[r1 - 1, c1 - 1];


    uint32_t sumR = p1.r - p2.r - p3.r + p4.r;
    uint32_t sumG = p1.g - p2.g - p3.g + p4.g;
    uint32_t sumB = p1.b - p2.b - p3.b + p4.b;


    inputGrid[inputGridRowNum, inputGridColNum] = {
        static_cast<uint8_t>(sumR / area), static_cast<uint8_t>(sumG / area),
        static_cast<uint8_t>(sumB / area), 255};
}

#endif
