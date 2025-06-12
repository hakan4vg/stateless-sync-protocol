//
// Created by h4vg on 6/12/25.
//

#include "encoder.h"
#include <fstream>
#include <iostream>

bool Encoder::encode(const std::string& inputPath,
                     const std::string& outputAPath,
                     const std::string& outputBPath) {

    std::vector<unsigned char> buffer;
    if (!readInput(inputPath, buffer)) {
        std::cerr << "Failed to read input file" << std::endl;
        return false;
    }
    std::vector<bool> streamA, streamB;
    scramble(buffer, streamA, streamB);

    if (!writeStream(streamA, outputAPath) || !writeStream(streamB, outputBPath)) {
        std::cerr << "Failed to write output file" << std::endl;
        return false;
    }
    return true;
}

bool Encoder::readInput(const std::string& inputPath, std::vector<unsigned char>& buffer) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file) return false;
    buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    return true;
}

void Encoder::scramble(const std::vector<unsigned char>& buffer, std::vector<bool>& streamA, std::vector<bool>& streamB) {
    //TODO: implement layers here for different logics

    //Pre-allocate memory for the a and b vectors
    //Buffer size * 8 / 2 (two vectors hence divide by 2)
    streamA.reserve(buffer.size() * 8 / 2);
    streamB.reserve(buffer.size() * 8 / 2);

    for (unsigned char byte : buffer) {
        for (int bit = 7; bit >= 0; --bit) {
            bool value = (byte >> bit) & 1; // Shift the bits of byte to right by bit position, & 1 to mask other bits other than LSB
            if (streamA.size() <= streamB.size()) {
                streamA.push_back(value);
            }
            else
                streamB.push_back(value);
        }
    }
}

bool Encoder::writeStream(const std::vector<bool>& stream, const std::string& output) {
    std::ofstream out(output, std::ios::binary);
    if (!out) return false;

    unsigned char byte = 0;
    int count = 0;
    for (bool bit : stream) {
        byte = (byte << 1) | static_cast<unsigned char>(bit);
        if (++count == 8) {
            out.put(static_cast<char>(byte));
            byte = 0;
            count = 0;
        }
    }

    // Pad with zeros
    if (count) {
        byte <<= (8 - count);
        out.put(static_cast<char>(byte));
    }
    return true;
}


int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: encoder <in> <outA> <outB>" << std::endl;
        return 1;
    }
    Encoder enc;
    return enc.encode(argv[1], argv[2], argv[3]) ? 0 : 2;
}