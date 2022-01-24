//
// Created by yufan on 11/1/21.
//

#ifndef QUERY_VARBYTE_H
#define QUERY_VARBYTE_H

#include <cstdint>
#include <iostream>

class Varbyte {
private:
    uint8_t* buffer;
    unsigned int lastVarbyteSize;
public:
    unsigned int encodeVarbyte(uint64_t val);
    unsigned int encodeVarbyte(uint32_t val);
    uint32_t decodeVarbyte(const uint8_t* in);
    uint8_t* getRawVarbyte();
    unsigned int getLastReadSize();
    unsigned int writeVarbyte(uint64_t val, FILE* fp);
    unsigned int writeVarbyte(uint32_t val, FILE* fp);
    unsigned int writeToBuffer(uint64_t val, char* dest);
    unsigned int writeToBuffer(uint32_t val, char* dest);
    Varbyte();
    ~Varbyte();
};

uint32_t readVarbyte(std::ifstream &infile);

#endif //QUERY_VARBYTE_H
