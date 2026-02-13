#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <mdspan>
#include "ImageProcessor.h"
#include <Pixel.h>
#include <mdspan>
#include <vector>


void placeholderFilter(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid, 
                       const std::mdspan<Pixel, std::dextents<size_t, 2>>& outputGrid, 
                       size_t inputGridRowNum, size_t inputGridColNum) {
    int borderWidth = (outputGrid.extent(0)-inputGrid.extent(0))/2;


    // Kernel: Box Blur
    
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    // outputGrid Equivalent for a Pixel A on the inputGrid => (rowNum+borderWidth) , (colNum+borderWidth) 
    int outputGridRowNum = inputGridRowNum+borderWidth;
    int outputGridColNum = inputGridColNum+borderWidth;

    for(int i {outputGridRowNum-borderWidth};i<=outputGridRowNum+borderWidth;i++){
        for(int j {outputGridColNum-borderWidth};j<=outputGridColNum+borderWidth;j++){
            sumR += outputGrid[i,j].r;
            sumG += outputGrid[i,j].g;
            sumB += outputGrid[i,j].b;
        }
    }
    sumR/=(2*borderWidth+1)*(2*borderWidth+1);
    sumG/=(2*borderWidth+1)*(2*borderWidth+1);
    sumB/=(2*borderWidth+1)*(2*borderWidth+1);
    inputGrid[inputGridRowNum, inputGridColNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB, 0, 255)),
        255
    };
}
#endif
