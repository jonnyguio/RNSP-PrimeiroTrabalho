// #include "mnist/mnist_reader.hpp"
#include <wann/WiSARD.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>

template<typename TimeT = std::chrono::milliseconds>
struct measure
{
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F&& func, Args&&... args)
    {
        auto start = std::chrono::steady_clock::now();
        std::forward<decltype(func)>(func)(std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast< TimeT> 
                            (std::chrono::steady_clock::now() - start);
        return duration.count();
    }
};

int numThreads = 4;
int numBitsAddrs = 16;
int threshold = 98;
bool bleaching = true;

using namespace wann;

void executePartial(std::vector<std::vector<int>> array, std::vector<std::vector<std::string>> *result, WiSARD *w) {
    std::vector<std::string> res;
    std::cout << "Starting prediction..." << std::endl;
    res = w->predict(array);
    std::cout << "Finished predict, adding to list" << std::endl;
    result->push_back(res);
    std::cout << "Exiting..." << std::endl;
}

void reverseInt(int *input) {
    unsigned char c1, c2, c3, c4;

    c1 = (*input) & 255;
    c2 = ((*input) >> 8) & 255;
    c3 = ((*input) >> 16) & 255;
    c4 = ((*input) >> 24) & 255;
    
    *input = ((int) c1 << 24) + ((int) c2 << 16) + ((int) c3 << 8) + ((int) c4); 
}

void convertMNISTBinary(std::vector<std::vector<int>> input, std::vector<std::vector<int>> &output) {
    // We are converting the input data, that contain values from 0 to 255 to binary.
    // Therefore, we need to multiply each line of vector to 64
    // basically, multiplying the columns and the rows by 8 
    int aux, count;
    output.resize(input.size(), std::vector<int>(input[0].size() * 64, 0));
    
    for (int i = 0; i < input.size(); i++) {
        for (int j = 0; j < input[0].size(); j++) {
            aux = input[i][j];
            count = 0;
            while (aux > 0) {
                output[i][j + count] = aux % 2;
                aux /= 2;
                count++;
            }
            for (int k = 0; k <= count / 2; k++) {
                aux = output[i][j + k];
                output[i][j + k] = output[i][j + count - k]; 
                output[i][j + count - k] = aux;
            }
        }
    }
}

void readLabelsMNIST(const char *filename, std::vector<std::string> &v) {
    unsigned char temp;
    int magicNumber = 0, numOfLabels = 0;
    std::ifstream file(filename, std::ios::binary);

    if (file.is_open()) {
        file.read((char *) &magicNumber, sizeof(magicNumber));
        reverseInt(&magicNumber);
        file.read((char *) &numOfLabels, sizeof(numOfLabels));
        reverseInt(&numOfLabels);
        v.resize(numOfLabels);
        std::cout << "Number of labels: " << numOfLabels << std::endl;
        for (int i = 0; i < numOfLabels; i++) {
            temp = 0;
            file.read((char *) &temp, sizeof(temp));
            // std::cout << temp << std::endl;
            v[i] = std::to_string((int) temp);
        }
    }
    else {
        std::cout << "File not found: " << filename << std::endl;
    }
}

void readImagesMNIST(const char *filename, std::vector<std::vector<int>> &array) {
    unsigned char temp;
    int magicNumber = 0, numOfImages = 0, rows = 0, columns = 0; 
    std::ifstream file(filename, std::ios::binary);

    if (file.is_open()) {
        file.read((char *) &magicNumber, sizeof(magicNumber));
        reverseInt(&magicNumber);
        file.read((char *) &numOfImages, sizeof(numOfImages));
        reverseInt(&numOfImages);
        file.read((char *) &rows, sizeof(rows));
        reverseInt(&rows);
        file.read((char *) &columns, sizeof(columns));
        reverseInt(&columns);
        array.resize(numOfImages, std::vector<int>(rows * columns));
        std::cout << "Number of images, rows and columns: " << numOfImages << ", " << rows << ", " << columns << std::endl;  
        for (int i = 0; i < numOfImages; i++) {
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < columns; c++) {
                    temp = 0;
                    file.read((char *) &temp, sizeof(temp));
                    array[i][(rows * r) + c] = (int) temp;
                }
            }
        }
    }
    else {
        std::cout << "File not found: " << filename << std::endl;
    }
}

void parallel() {

    int count = 0;
    std::vector<std::vector<int>> trainingImages, testImages;
    std::vector<std::string> trainingLabels, testLabels, result;
    std::vector<std::vector<int>>::const_iterator first, last;
    std::vector<std::thread *> prediction;
    std::vector<std::vector<std::string>> resultPartial;

    const char *trainLabelsFilename = "./mnist/train-labels-idx1-ubyte", *trainImagesFilename = "./mnist/train-images-idx3-ubyte", *testLabelsFilename = "./mnist/t10k-labels-idx1-ubyte", *testImagesFilename = "./mnist/t10k-images-idx3-ubyte";

    std::thread tTrainLabels(readLabelsMNIST, std::ref(trainLabelsFilename), std::ref(trainingLabels));
    std::thread tTrainImages(readImagesMNIST, std::ref(trainImagesFilename), std::ref(trainingImages));
    std::thread tTestLabels(readLabelsMNIST, std::ref(testLabelsFilename), std::ref(testLabels));
    std::thread tTestImages(readImagesMNIST, std::ref(testImagesFilename), std::ref(testImages));

    tTrainLabels.join();
    tTrainImages.join();
    tTestLabels.join();
    tTestImages.join();

    std::cout << "Preparing it to WiSARD..." << std::endl;
    for (int i = 0; i < trainingImages.size(); i++) {
        for (int j = 0; j < trainingImages[0].size(); j++) {
            if (trainingImages[i][j] < threshold) {
                trainingImages[i][j] = 0;
            }
            else {
                trainingImages[i][j] = 1;
            }
        }
    }
    
    WiSARD *w[numThreads];
    for (int i = 0; i < numThreads; i++) {
        w[i] = new WiSARD(numBitsAddrs, bleaching);
    }

    // std::cout << trainingImages[0].size() << " " << trainingImages.size() << " " << trainingLabels.size() << std::endl;

    std::cout << "Training..." << std::endl;
    for (int i = 0; i < numThreads; i++) {
        w[i]->fit(trainingImages, trainingLabels);
    }

    std::cout << "Prediction..." << std::endl;

    for (int i = 0; i < numThreads; i++) {
        first = testImages.begin() + testImages.size() / numThreads * i;
        last = testImages.begin() + testImages.size() / numThreads * (i + 1);
        std::vector<std::vector<int>> partial(first, last);

        prediction.push_back(new std::thread(executePartial, partial, &resultPartial, w[i]));
    }

    for (int i = 0; i < numThreads; i++) {
        prediction[i]->join();
    }

    // result = w->predict(testImages);

    std::cout << "Finished, counting right answers (" << resultPartial.size() << ", " << resultPartial[0].size() << ", " << resultPartial[1].size() << ", " << resultPartial[2].size() << ", " << resultPartial[3].size() << ")" << std::endl;

    for (int i = 0; i < resultPartial.size(); i++) {
        for (int j = 0; j < resultPartial[i].size(); j++) {
            if (testLabels[i * resultPartial[0].size() + j] == resultPartial[i][j]) {
                count++;
                // std::cout << "Image " << i << "is " << result[i] << std::endl;
            }
        }
    }

    std::cout << "Rights: " << count << std::endl;

    delete w[4];
    // std::cout << "Imprimindo labels (" << trainingLabels.size() << ")" << std::endl;
    // for (int i = 0; i < trainingLabels.size(); i++) {
    //     std::cout << trainingLabels[i] << std::endl;
    // }
}

void sequential() {
    int count = 0;
    std::vector<std::vector<int>> trainingImages, testImages, trainingImagesExtended, testImagesExtended;
    std::vector<std::string> trainingLabels, testLabels, result;

    readLabelsMNIST("./mnist/train-labels-idx1-ubyte", trainingLabels);
    readImagesMNIST("./mnist/train-images-idx3-ubyte", trainingImages);
    readLabelsMNIST("./mnist/t10k-labels-idx1-ubyte", testLabels);
    readImagesMNIST("./mnist/t10k-images-idx3-ubyte", testImages);

    std::cout << "Preparing it to WiSARD..." << std::endl;
    // for (int i = 0; i < trainingImages.size(); i++) {
    //     for (int j = 0; j < trainingImages[0].size(); j++) {
    //         if (trainingImages[i][j] < threshold) {
    //             trainingImages[i][j] = 0;
    //         }
    //         else {
    //             trainingImages[i][j] = 1;
    //         }
    //     }
    // }
    convertMNISTBinary(trainingImages, trainingImagesExtended);
    convertMNISTBinary(testImages, testImagesExtended);

    WiSARD *w = new WiSARD(trainingImagesExtended[0].size(), numBitsAddrs, bleaching);

    // std::cout << trainingImages[0].size() << " " << trainingImages.size() << " " << trainingLabels.size() << std::endl;

    std::cout << "Training..." << std::endl;
    w->fit(trainingImagesExtended, trainingLabels);

    std::cout << "Prediction..." << std::endl;
    result = w->predict(testImagesExtended);

    for (int i = 0; i < testLabels.size(); i++) {
        if (testLabels[i] == result[i]) {
            count++;
            // std::cout << "Image " << i << "is " << result[i] << std::endl;
        }
    }
    std::cout << "Rights: " << count << std::endl;

    delete w;
    // std::cout << "Imprimindo labels (" << trainingLabels.size() << ")" << std::endl;
    // for (int i = 0; i < trainingLabels.size(); i++) {
    //     std::cout << trainingLabels[i] << std::endl;
    // }
}

int main(int argc, char *argv[]) {

    if (argc > 1) {
        if (argc > 2) {
            numBitsAddrs = atoi(argv[2]);
            if (argc > 3) {
                if (argv[3][0] == '0') {
                    bleaching = false;
                }
                else {
                    if (argv[3][0] != '1') {
                        std::cout << "Wrong argument. Use 0 for false and 1 for true" << std::endl;
                        return 0;
                    }
                }
                if (argc > 4) {
                    numThreads = atoi(argv[4]);
                }
            }
        }
        if (argv[1][0] == '1') {
            std::cout << measure<std::chrono::milliseconds>::execution(parallel) / 1000 << " seconds." << std::endl;
            // parallel()
        }
        else {
            if (argv[1][0] == '0') {
               std::cout << measure<std::chrono::milliseconds>::execution(sequential) / 1000 << " seconds." << std::endl;
            }
            else {
                std::cout << "Wrong argument. Use 0 for sequential or 1 for parallel execution" << std::endl;
                return 0;
            }
            // sequential();
        }
    }
    else {
        std::cout << "usage: " << argv[0] << " <0 = sequential, 1 = parallel> [<numBitsAddrs> <bleaching, 0 = false, 1 = true> <numThreads>]" << std::endl;
    }
    return 0;
}