#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ImageProcessor.h"
#include <Pixel.h>
#include <mdspan>
#include <vector>
#include <iostream>

void naiveBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                  const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                  size_t inputGridRowNum, size_t inputGridColNum) {
    int borderWidth = (paddedGrid.extent(0) - inputGrid.extent(0)) / 2;

    // Kernel: Box Blur

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
                const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                const std::mdspan<SatPixel, std::dextents<size_t, 2>>& satGrid,
                size_t inputGridRowNum, size_t inputGridColNum, int borderWidth) {


    // paddedGrid Equivalent for a Pixel A on the inputGrid => (rowNum+borderWidth) ,
    // (colNum+borderWidth)
    int borderWidthAccountingEdge=borderWidth+1;
    int paddedGridRowNum = inputGridRowNum + borderWidthAccountingEdge;
    int paddedGridColNum = inputGridColNum + borderWidthAccountingEdge;

    const SatPixel& p1 = satGrid[paddedGridRowNum + borderWidth, paddedGridColNum + borderWidth];
    const SatPixel& p2 = satGrid[paddedGridRowNum, (paddedGridColNum - borderWidth) - 1];
    const SatPixel& p3 = satGrid[(paddedGridRowNum - borderWidth) - 1, paddedGridColNum];
    const SatPixel& p4 = satGrid[paddedGridRowNum - borderWidth, paddedGridColNum - borderWidth];

    std::cout << "\nHERE" << int(p1.r) << "HERE\n";
    int kernelSize = (2 * borderWidth + 1);
    int area = kernelSize * kernelSize;

    int32_t sumR = (static_cast<int32_t>(p1.r) - static_cast<int32_t>(p2.r) -
                   static_cast<int32_t>(p3.r) + static_cast<int32_t>(p4.r))/area;
    __builtin_trap();
}

#endif
