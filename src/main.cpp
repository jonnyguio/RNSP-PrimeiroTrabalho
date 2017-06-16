// #include "mnist/mnist_reader.hpp"
#include <wann/WiSARD.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
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

const char 
    *trainLabelsFilename = "./mnist/train-labels-idx1-ubyte", 
    *trainImagesFilename = "./mnist/train-images-idx3-ubyte", 
    *testLabelsFilename = "./mnist/t10k-labels-idx1-ubyte",
    *testImagesFilename = "./mnist/t10k-images-idx3-ubyte";
int numThreads = 4;
int numBitsAddrs = 16;
int threshold = 98;
bool bleaching = true;
bool extended = false;
std::mutex resultMutex;

using namespace wann;

void executePartial(int threadID, std::vector<std::vector<int>> array, std::vector< std::vector<std::string> > *result, WiSARD *w) {
    std::vector<std::string> res;
    std::cout << "Starting prediction..." << std::endl;
    res = w->predict(array);
    std::cout << "Finished predict, adding to list" << std::endl;
    resultMutex.lock();
    (*result)[threadID] = res;
    resultMutex.unlock();
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
    // basically, multiply each line by 8, because we're expanding each number to 8 digits
    int aux, count;
    output.resize(input.size(), std::vector<int>(input[0].size() * 8, 0));
    
    for (int i = 0; i < input.size(); i++) {
        for (int j = 0; j < input[0].size(); j++) {
            aux = input[i][j];
            count = 0;
            while (aux > 0) {
                output[i][j * 8 + count] = aux % 2;
                aux /= 2;
                count++;
            }
            std::reverse(output[i].begin() + j * 8, output[i].begin() + j * 8 + count);
            // for (int k = 0; k <= count / 2; k++) {
            //     aux = output[i][j * 8 + k];
            //     output[i][j * 8 + k] = output[i][j * 8  + count - k]; 
            //     output[i][j * 8  + count - k] = aux;
            // }
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

void prepareToWisard(std::vector< std::vector < int > > &images, std::vector< std::vector < int > > &imagesExtended) {
    if (!extended) {
        for (int i = 0; i < images.size(); i++) {
            for (int j = 0; j < images[0].size(); j++) {
                if (images[i][j] < threshold) {
                    images[i][j] = 0;
                }
                else {
                    images[i][j] = 1;
                }
            }
        }
    }
    else {
        convertMNISTBinary(images, imagesExtended);
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
    std::vector<std::vector<int>> trainingImages, testImages, trainingImagesExtended, testImagesExtended;
    std::vector<std::string> trainingLabels, testLabels, result;
    std::vector<std::vector<int>>::const_iterator first, last;
    std::vector<std::thread *> prediction;
    std::vector< std::vector< std::string > > resultPartial(numThreads);

    readLabelsMNIST(trainLabelsFilename, trainingLabels);
    readImagesMNIST(trainImagesFilename, trainingImages);
    readLabelsMNIST(testLabelsFilename, testLabels);
    readImagesMNIST(testImagesFilename, testImages);

    // std::thread tTrainLabels(readLabelsMNIST, std::ref(trainLabelsFilename), std::ref(trainingLabels));
    // std::thread tTrainImages(readImagesMNIST, std::ref(trainImagesFilename), std::ref(trainingImages));
    // std::thread tTestLabels(readLabelsMNIST, std::ref(testLabelsFilename), std::ref(testLabels));
    // std::thread tTestImages(readImagesMNIST, std::ref(testImagesFilename), std::ref(testImages));

    // tTrainLabels.join();
    // tTrainImages.join();
    // tTestLabels.join();
    // tTestImages.join();

    std::cout << "Preparing it to WiSARD..." << std::endl;
    
    prepareToWisard(trainingImages, trainingImagesExtended);
    prepareToWisard(testImages, testImagesExtended);

    WiSARD *w = new WiSARD((extended) ? trainingImagesExtended[0].size() : trainingImages[0].size(), numBitsAddrs, bleaching);

    std::cout << "Training..." << std::endl;
    w->fit((extended) ? trainingImagesExtended : trainingImages, trainingLabels);

    std::cout << "Prediction..." << std::endl;

    for (int i = 0; i < numThreads; i++) {
        if (!extended) {
            first = testImages.begin() + testImages.size() / numThreads * i;
            last = testImages.begin() + testImages.size() / numThreads * (i + 1);
        }
        else {
            first = testImagesExtended.begin() + testImagesExtended.size() / numThreads * i;
            last = testImagesExtended.begin() + testImagesExtended.size() / numThreads * (i + 1);
        }
        std::vector<std::vector<int>> partial(first, last);

        prediction.push_back(new std::thread(executePartial, i, partial, &resultPartial, w));
    }

    for (int i = 0; i < numThreads; i++) {
        prediction[i]->join();
    }

    std::cout << "Finished, counting right answers (" << numThreads << "):" << std::endl; 
    std::cout << "\t";
    for (int i = 0; i < numThreads; i++) {
        std::cout << resultPartial[i].size() << " ";
    }
    std::cout << std::endl;

    for (int i = 0; i < numThreads; i++) {
        for (int j = 0; j < resultPartial[0].size(); j++) {
            if (testLabels[i * resultPartial[0].size() + j] == resultPartial[i][j]) {
                count++;
            }
        }
    }

    std::cout << "Rights: " << count << std::endl;

    delete w;
}

void sequential() {
    int count = 0;
    std::vector<std::vector<int>> trainingImages, testImages, trainingImagesExtended, testImagesExtended;
    std::vector<std::string> trainingLabels, testLabels, result;

    readLabelsMNIST(trainLabelsFilename, trainingLabels);
    readImagesMNIST(trainImagesFilename, trainingImages);
    readLabelsMNIST(testLabelsFilename, testLabels);
    readImagesMNIST(testImagesFilename, testImages);

    std::cout << "Preparing it to WiSARD..." << std::endl;

    prepareToWisard(trainingImages, trainingImagesExtended);
    prepareToWisard(testImages, testImagesExtended);
    
    WiSARD *w = new WiSARD((extended) ? trainingImagesExtended[0].size() : trainingImages[0].size(), numBitsAddrs, bleaching);

    std::cout << "Training..." << std::endl;
    w->fit((extended) ? trainingImagesExtended : trainingImages, trainingLabels);

    std::cout << "Prediction..." << std::endl;
    result = w->predict((extended) ? testImagesExtended : testImages);

    std::cout << "Rights: ";
    for (int i = 0; i < testLabels.size(); i++) {
        if (testLabels[i] == result[i]) {
            count++;
        }
    }
    std::cout << count << std::endl;

    delete w;
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
                    extended = (argv[4][0] == '0') ? false : true;
                    if (argc > 5 ) {
                        numThreads = atoi(argv[5]);
                    }
                }
            }
        }
        if (argv[1][0] == '1') {
            std::cout << measure<std::chrono::milliseconds>::execution(parallel) << " miliseconds." << std::endl;
        }
        else {
            if (argv[1][0] == '0') {
               std::cout << measure<std::chrono::milliseconds>::execution(sequential) << " miliseconds." << std::endl;
            }
            else {
                std::cout << "Wrong argument. Use 0 for sequential or 1 for parallel execution" << std::endl;
                return 0;
            }
        }
    }
    else {
        std::cout << "usage: " << argv[0] << " <0 = sequential, 1 = parallel> [<numBitsAddrs> <bleaching, 0 = false, 1 = true> <extend images> <numThreads>]" << std::endl;
    }
    return 0;
}