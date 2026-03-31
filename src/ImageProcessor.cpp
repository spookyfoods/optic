#include "ImageProcessor.h"
#include "Filters.h"
#include "Pixel.h"
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mdspan>
#include <memory>
#include <span>
#include <thread>
#include "Kernel.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace {
struct WavefrontContext {
    std::mutex m;
    std::condition_variable data_cond;
    int maxSafeRowForAcross = 0; // Starts at 0 because row 0 is already done (initialized to 0)

    // Context references
    std::mdspan<SatPixel, std::dextents<size_t, 2>> satGrid;
    std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid;
    int h, w;

    WavefrontContext(std::mdspan<SatPixel, std::dextents<size_t, 2>>& _satGrid,
                     std::mdspan<Pixel, std::dextents<size_t, 2>>& _paddedGrid, int height,
                     int width)
        : satGrid(_satGrid), paddedGrid(_paddedGrid), h(height), w(width) {}

    // Producer: Vertical Pass (Columns)
    void downCol(int batch_size) {
        // Start at 1 because row 0 is was already initialized with 0, and will have no accumulation
        for(int r{1}; r < h; r++) {
            for(int c{1}; c < w; c++) {
                // Column Prefix Sum: Current = Input + Above
                satGrid[r, c] = paddedGrid[r, c] + satGrid[r - 1, c];
            }

            // Notify periodically to wake up the horizontal thread
            if(r % batch_size == 0) {
                {
                    std::lock_guard<std::mutex> lk(m);
                    maxSafeRowForAcross = r;
                }
                data_cond.notify_one();
            }
        }

        // Final notification for any remaining rows
        {
            std::lock_guard<std::mutex> lk(m);
            maxSafeRowForAcross = h;
        }
        data_cond.notify_one();
    }
    // Consumer: Horizontal Pass (Rows)

    void acrossRow() {
        int currentRow = 1; // Start at 1, row 0 is dummy/boundary

        while(currentRow < h) {
            int limit = 0;

            // 1. Wait for work
            {
                std::unique_lock<std::mutex> lk(m);
                data_cond.wait(lk, [&] { return maxSafeRowForAcross >= currentRow; });
                limit = maxSafeRowForAcross;
            }
            // Lock released here

            // 2. Greedy Loop: Process ALL available rows without locking again
            while(currentRow <= limit && currentRow < h) {
                for(int c{1}; c < w; c++) {
                    // Row Prefix Sum: Current = Previous + Current (which was set by downCol)
                    satGrid[currentRow, c] = satGrid[currentRow, c] + satGrid[currentRow, c - 1];
                }
                currentRow++;
            }
        }
    }
};
struct TwoPassContext {

    // Context references
    std::mdspan<SatPixel, std::dextents<size_t, 2>> satGrid;
    std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid;
    int h, w;
    TwoPassContext(std::mdspan<SatPixel, std::dextents<size_t, 2>>& _satGrid,
                   std::mdspan<Pixel, std::dextents<size_t, 2>>& _paddedGrid, int height, int width)
        : satGrid(_satGrid), paddedGrid(_paddedGrid), h(height), w(width) {}
    void execute() {
        // PASS 1: DOWN COLUMNS
        // Split width into two halves
        int midW = w / 2;
        std::thread t1(&TwoPassContext::downCol, this, 0, midW);
        std::thread t2(&TwoPassContext::downCol, this, midW, w);

        // Columnar Work barrier
        t1.join();
        t2.join();
        // PASS 2: ACROSS ROWS
        // Split height into two halves
        int midH = h / 2;
        std::thread t3(&TwoPassContext::acrossRow, this, 0, midH);
        std::thread t4(&TwoPassContext::acrossRow, this, midH+1, h);

        // Row-wise Work Barrier
        t3.join();
        t4.join();
    }
    void downCol(int startCol, int endCol) {
        if(startCol == 0) {
            ++startCol;
        }
        for(int r{1}; r < h; r++) {
            for(int c{startCol}; c < endCol; c++) {
                // Col Prefix Sum: Current = current + above
                satGrid[r, c] = paddedGrid[r, c] + satGrid[r - 1, c];
            }
        }
    }
    void acrossRow(int startRow, int endRow) {
        if(startRow == 0) {
            ++startRow;
        }
        for(int r{startRow}; r < endRow; r++) {
            for(int c{1}; c < w; c++) {
                // Row Prefix Sum: Current = Current + left, which was setup by downCol
                satGrid[r, c] += satGrid[r, c - 1];
            }
        }
    }
};

} // namespace

ImageProcessor::ImageProcessor() : width(0), height(0), channels(0), pixelData(nullptr) {
    std::cout << "[C++] ImageProcessor Initialized" << std::endl;
}

ImageProcessor::~ImageProcessor() { delete[] pixelData; }

ImageProcessor::satDataAndGrid
ImageProcessor::computeSAT(int newWidth, int newHeight, int borderWidth,
                           std::mdspan<Pixel, std::dextents<size_t, 2>> paddedGrid,
                           ImageProcessor::SatMethod processingType) {

    // 1. Allocate and Initialize
    auto satData = std::make_unique_for_overwrite<uint32_t[]>(4 * newHeight * newWidth);
    std::mdspan satGrid(reinterpret_cast<SatPixel*>(satData.get()), newHeight, newWidth);

    // Initialize first row and first column to 0 (Boundary conditions)
    for(int j{0}; j < newWidth; j++)
        satGrid[0, j] = {0, 0, 0, 0};
    for(int i{1}; i < newHeight; i++)
        satGrid[i, 0] = {0, 0, 0, 0};

    if(processingType == ImageProcessor::SatMethod::SERIAL) {
        std::cout << "Linear SAT Creation\n";
        // Standard SAT formula: I(x,y) + SAT(x-1,y) + SAT(x,y-1) - SAT(x-1,y-1)
        for(int i{1}; i < newHeight; i++) {
            for(int k{1}; k < newWidth; k++) {
                satGrid[i, k] = paddedGrid[i, k] + satGrid[i - 1, k] + satGrid[i, k - 1] -
                                satGrid[i - 1, k - 1];
            }
        }
    } else if(processingType == ImageProcessor::SatMethod::WAVEFRONT_PIPELINE) {
        std::cout << "Parallel Sat Creation (WAVEFRONT)\n";
        WavefrontContext ctx(satGrid, paddedGrid, newHeight, newWidth);

        // Launch threads
        // downCol acts as the Producer (Vertical Pass)
        std::thread t1([&ctx]() { ctx.downCol(32); });

        // acrossRow acts as the Consumer (Horizontal Pass)
        std::thread t2([&ctx]() { ctx.acrossRow(); });

        t1.join();
        t2.join();
    } else if(processingType == ImageProcessor::SatMethod::TWO_PASS_BARRIER) {
        std::cout << "Parallel Sat Creation (TWO PASS)\n";
        TwoPassContext ctx(satGrid, paddedGrid, newHeight, newWidth);
        ctx.execute();
    }
    return std::make_pair(std::move(satData), satGrid);
}

bool ImageProcessor::loadImage(uintptr_t bufferPtr, int size) {

    const unsigned char* rawData = reinterpret_cast<const unsigned char*>(bufferPtr);
    int tempW, tempH, tempC;
    unsigned char* tempStbData = stbi_load_from_memory(rawData, size, &tempW, &tempH, &tempC, 4);
    if(!tempStbData) {
        std::cerr << "[C++] Failed to load image." << '\n';
        return false;
    }
    if(pixelData) {
        delete[] pixelData;
    }
    width = tempW;
    height = tempH;
    channels = 4;

    pixelData = new unsigned char[width * height * 4];

    std::memcpy(pixelData, tempStbData, width * height * 4);

    stbi_image_free(tempStbData);

    std::cout << "[C++] Loaded Image: " << width << "x" << height << " (RGBA)" << '\n';
    return true;
}
ImageProcessor::paddedDataAndGrid
ImageProcessor::createPadding(int newWidth, int newHeight, int borderWidth,
                              std::mdspan<Pixel, std::dextents<size_t, 2>> inputGrid) {

    auto paddedData = std::make_unique_for_overwrite<unsigned char[]>(newWidth * newHeight * 4);
    std::mdspan paddedGrid(reinterpret_cast<Pixel*>(paddedData.get()), newHeight, newWidth);
    for(int i{borderWidth}; i < newHeight - borderWidth; i++) {
        for(int k{borderWidth}; k < newWidth - borderWidth; k++) {
            paddedGrid[i, k] = inputGrid[i - borderWidth, k - borderWidth];
        }
    }

    for(int i{borderWidth}; i < newHeight - borderWidth; i++) {
        for(int j{0}; j < borderWidth; j++) {
            paddedGrid[i, j] = paddedGrid[i, borderWidth];
        }
        for(int j{0}; j < borderWidth; j++) {
            paddedGrid[i, newWidth - borderWidth + j] = paddedGrid[i, newWidth - borderWidth - 1];
        }
    }
    for(int i{0}; i < borderWidth; i++) {
        for(int j{0}; j < newWidth; j++) {
            paddedGrid[i, j] = paddedGrid[borderWidth, j];
            paddedGrid[newHeight - borderWidth + i, j] = paddedGrid[newHeight - borderWidth - 1, j];
        }
    }

    return std::make_pair(std::move(paddedData), paddedGrid);
}
void ImageProcessor::applyFilter(int kernelSize, std::string filterType) {
    if(!pixelData) {
        std::cerr << "[C++] Failed to process image." << std::endl;
    }
    // kernel size must be odd and a square => (2n+1) x (2n+1)
    bool create_sat{filterType == "sat"};
    int borderWidth = ((kernelSize - 1) / 2) + static_cast<int>(create_sat);
    int newWidth{width + 2 * (borderWidth)};
    int newHeight{height + 2 * (borderWidth)};

    // Height represents Number of Rows
    // Width rerpresents Number of Cols
    std::mdspan inputGrid(reinterpret_cast<Pixel*>(pixelData), height, width);

    auto [paddedData, paddedGrid] = createPadding(newWidth, newHeight, borderWidth, inputGrid);

    std::cout << "\nInput Pix[0,0]:\t" << (int)inputGrid[0, 0].r << " " << (int)inputGrid[0, 0].g
              << " " << (int)inputGrid[0, 0].b << "\n";

    // For iterating through the cells of input grid
    auto traverse = [&](auto operation) {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                operation(i, j);
            }
        }
    };

    if(filterType=="sat") {
        auto [satData, satGrid] = computeSAT(newWidth, newHeight, borderWidth, paddedGrid,
                                             ImageProcessor::SatMethod::SERIAL);
        std::cout << "\nRUNNING SAT BOX BLUR" << std::endl;
        traverse([&](int i, int j) { satBoxBlur(inputGrid, satGrid, i, j); });
    } else if(filterType=="naive") {
        std::cout << "\nRUNNING NAIVE BOX BLUR" << std::endl;
        traverse([&](int i, int j) { naiveBoxBlur(inputGrid, paddedGrid, i, j); });
    } else if(filterType=="factorybb"){
        std::cout << "\nRUNNNIG FACTORY BOX BLUR" << std::endl;
        auto kernel = KernelFactory::createBoxBlur(kernelSize);
        traverse([&](int i, int j){e_naiveBoxBlur(inputGrid, paddedGrid, i, j, kernel);});
    }
    std::cout << "\nInput Pix[0,0]:\t" << (int)inputGrid[0, 0].r << " " << (int)inputGrid[0, 0].g
              << " " << (int)inputGrid[0, 0].b << "\n";
}

int ImageProcessor::getWidth() const { return width; }
int ImageProcessor::getHeight() const { return height; }
uintptr_t ImageProcessor::getPixelDataPtr() const { return reinterpret_cast<uintptr_t>(pixelData); }
