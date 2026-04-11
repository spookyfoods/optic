#include <span>
#include <iostream>
#include <fstream>
#include <vector>
#include "ImageProcessor.h"

int main(int argc, char* argv[]){
    if(argc!=5){
        std::cout << "Error!";
        exit(1);
    }
    std::string inputPath {argv[1]};
    std::string outputPath {argv[2]};

    std::ifstream file(inputPath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(),size);
    ImageProcessor processor;
    std::cout << static_cast<int>(atoi(argv[4]))  << argv[3];

    processor.loadImage(buffer, size);
    processor.applyFilter(static_cast<int>(atoi(argv[4])),argv[3]);
    std::ofstream outputImage(outputPath, std::ios::binary);
    char* data = reinterpret_cast<char*>(processor.getPixelDataPtr());
    int totalPixels = processor.getWidth() * processor.getHeight();
    outputImage<<"P6\n" << processor.getWidth() << " " << processor.getHeight() << "\n255\n";
    for (int i = 0; i < totalPixels; ++i) {
        int offset = i * 4; 
        outputImage << data[offset];\
        outputImage << data[offset + 1];
        outputImage << data[offset + 2];
    }
    outputImage.close();
    
}


