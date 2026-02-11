#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "ImageProcessor.h"
#include <Pixel.h>
#include <mdspan>
#include <vector>


void placeholderFilter(const std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid, 
                       std::mdspan<Pixel, std::dextents<size_t, 2>>& outputGrid, 
                       size_t rowNum, size_t colNum) {
    
    // Kernel: 
    // -1  -1  -1
    // -1   8  -1
    // -1  -1  -1
    
    int sumR = 0;
    int sumG = 0;
    int sumB = 0;

    for(int dy = -1; dy <= 1; ++dy) {
        for(int dx = -1; dx <= 1; ++dx) {
            
            long long inY = static_cast<long long>(rowNum) - 1 + dy;
            long long inX = static_cast<long long>(colNum) - 1 + dx;

            if(inY >= 0 && inY < static_cast<long long>(inputGrid.extent(0)) && 
               inX >= 0 && inX < static_cast<long long>(inputGrid.extent(1))) {
                
                Pixel p = inputGrid[inY, inX];
                int kernelVal = (dy == 0 && dx == 0) ? 8 : -1;

                sumR += p.r * kernelVal;
                sumG += p.g * kernelVal;
                sumB += p.b * kernelVal;
            }
        }
    }

    outputGrid[rowNum, colNum] = Pixel{
        static_cast<uint8_t>(std::clamp(sumR, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumG, 0, 255)),
        static_cast<uint8_t>(std::clamp(sumB, 0, 255)),
        255
    };
}
#endif
