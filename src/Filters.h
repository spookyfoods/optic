#ifndef FILTERS_H
#define FILTERS_H

#include "ImageProcessor.h"
#include "Pixel.h"
#include "Kernel.h"
#include <mdspan>

void e_naiveBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                    const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                    size_t inputGridRowNum, size_t inputGridColNum,Kernel<int> k);
void naiveBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                  const std::mdspan<Pixel, std::dextents<size_t, 2>>& paddedGrid,
                  size_t inputGridRowNum, size_t inputGridColNum);
void satBoxBlur(std::mdspan<Pixel, std::dextents<size_t, 2>>& inputGrid,
                const std::mdspan<SatPixel, std::dextents<size_t, 2>>& satGrid,
                size_t inputGridRowNum, size_t inputGridColNum);
void trollGe();
#endif
