#ifndef KERNEL_H
#define KERNEL_H

#include <vector>

template <typename T>
struct Kernel {
    int width;
    int height;
    std::vector<T> matrix;

    float normalizationFactor;
    
    bool isSeparable{};
    std::vector<T> rowVector;
    std::vector<T> colVector;
};

class KernelFactory {
    public:
    Kernel<int> createBoxBlur(int size){
        Kernel<int> k;
        k.width=size;
        k.height=size;
        k.normalizationFactor = size*size;

        k.matrix.assign(size*size, 1);

        k.isSeparable=false;

        

    }
};

#endif
