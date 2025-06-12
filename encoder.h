//
// Created by h4vg on 6/12/25.
//

#ifndef ENCODER_H
#define ENCODER_H

#include <string>
#include <vector>


class Encoder {
public:
    Encoder() = default;


    bool encode(const std::string& inputPath,
                const std::string& outputAPath,
                const std::string& outputBPath);
private:
    bool readInput(const std::string& path, std::vector<unsigned char>& buffer);
    void scramble(const std::vector<unsigned char> &buffer,
                  std::vector<bool> &streamA,
                  std::vector<bool> &streamB);
    bool writeStream(const std::vector<bool>& stream, const std::string& path);
};



#endif //ENCODER_H
