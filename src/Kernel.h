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

    static Kernel<int> createBoxBlur(int size) {
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
};

#endif
