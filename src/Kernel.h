#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

template <typename T> struct Kernel {
    int width;
    int height;
    std::vector<T> matrix;

    float normalizationFactor;

    int basePadding = 0;
    int extraBoundaryPadding = 0;

    bool isSeparable = false;
    std::vector<T> rowVector;
    std::vector<T> colVector;
};

class KernelFactory {
  public:
    // ---------------------------------------------------------
    // DYNAMIC KERNELS (Size is variable)
    // ---------------------------------------------------------

    static Kernel<int> BoxBlur(int size) {
        Kernel<int> k;
        k.width = size;
        k.height = size;
        k.basePadding = size / 2;

        float area = static_cast<float>(size * size);
        k.normalizationFactor = area;

        k.matrix.assign(size * size, 1);

        k.isSeparable = true;
        k.rowVector.assign(size, 1);
        k.colVector.assign(size, 1);

        return k;
    }
    static Kernel<float> GaussianBlur(int size, float sigma = 0.0f) {
        if(sigma <= 0.0f) {
            sigma = 0.3f * ((size - 1) * 0.5f - 1.0f) + 0.8f;
            if(size <= 3)
                sigma = 0.8f;
        }

        Kernel<float> k;
        k.width = size;
        k.height = size;
        k.normalizationFactor=1.0f;
        k.matrix.resize(size * size);

        int radius = size / 2;
        float sum2D = 0.0f;

        // Calculate the raw 2D weights
        for(int y = -radius; y <= radius; ++y) {
            for(int x = -radius; x <= radius; ++x) {
                float weight = std::exp(-(x * x + y * y) / (2.0f * sigma * sigma));

                int index = (y + radius) * size + (x + radius);
                k.matrix[index] = weight;
                sum2D += weight;
            }
        }

        // Normalize the entire 2D matrix
        for(int i = 0; i < size * size; ++i) {
            k.matrix[i] /= sum2D;
        }

        return k;
    }

    // ---------------------------------------------------------
    // STATIC KERNELS
    // ---------------------------------------------------------

    static Kernel<int> SobelX() {
        Kernel<int> k;
        k.width = 3;
        k.height = 3;
        k.basePadding = 1;

        k.matrix.reserve(9);

        k.normalizationFactor = 1.0f;

        k.matrix = {-1, 0, 1, -2, 0, 2, -1, 0, 1}; // SOBEL X only

        return k;
    }
    static Kernel<int> SobelY() {
        Kernel<int> k;
        k.width = 3;
        k.height = 3;
        k.basePadding = 1;

        k.matrix.reserve(9);

        k.normalizationFactor = 1.0f;

        k.matrix = {-1, -2, -1, 0, 0, 0, 1, 2, 1}; // SOBEL Y only

        return k;
    }
};

#endif
