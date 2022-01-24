//
// Created by yufan on 10/13/21.
//

#ifndef RINDEXER_VARBYTE_H
#define RINDEXER_VARBYTE_H

#include <cstdint>
#include <iostream>

class Vbyte {
private:
    uint8_t* buffer;
    unsigned int lastVbyteSize;
public:
    unsigned int encodeVbyte(uint64_t val);
    unsigned int encodeVbyte(uint32_t val);
    uint64_t decodeVbyte(const uint8_t* in);
    uint8_t* getRawVbyte();
    unsigned int getLastReadSize();
    unsigned int writeVbyte(uint64_t val, FILE* fp);
    unsigned int writeVbyte(uint32_t val, FILE* fp);
    unsigned int writeToBuffer(uint64_t val, char* dest);
    unsigned int writeToBuffer(uint32_t val, char* dest);
    Vbyte();
    ~Vbyte();
};





#endif //RINDEXER_VARBYTE_H
