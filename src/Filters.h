#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ImageProcessor.h"
#include <Pixel.h>
#include <iostream>
#include <mdspan>
#include <vector>

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
                size_t inputGridRowNum, size_t inputGridColNum, int kernelSize) {

    // paddedGrid Equivalent for a Pixel A on the inputGrid => (rowNum+borderWidth) ,
    // (colNum+borderWidth)
    int radius = (kernelSize - 1) / 2;

    int borderWidth = radius + 1;
    int mike = satGrid.extent(0) -inputGrid.extent(0);
    // if(mike==borderWidth){
    //     std::cout << "HEY GOOD";
    //     EXIT_FAILURE;
    // }

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

    int area = kernelSize * kernelSize;

    uint32_t sumR = p1.r - p2.r - p3.r + p4.r;
    uint32_t sumG = p1.g - p2.g - p3.g + p4.g;
    uint32_t sumB = p1.b - p2.b - p3.b + p4.b;


    inputGrid[inputGridRowNum, inputGridColNum] = {
        static_cast<uint8_t>(sumR / area), static_cast<uint8_t>(sumG / area),
        static_cast<uint8_t>(sumB / area), 255};
}

#endif
