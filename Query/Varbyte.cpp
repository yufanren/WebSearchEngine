//
// Created by yufan on 11/1/21.
//

#include "Varbyte.h"
#include <fstream>
#include <cstring>
#include "Varbyte.h"

//Compress and decompress integers.

std::string toBinary(int n);

// write varbyte to pointer and return how many bytes it wrote.
unsigned int Varbyte::encodeVarbyte(uint64_t val) {

    unsigned int outSize = 0;
    while (val > 127) {
        buffer[outSize] = ((uint8_t)(val & 127)) | 128;
        val >>= 7;
        outSize++;
    }
    buffer[outSize++] = ((uint8_t)val) & 127;
    return outSize;
}

unsigned int Varbyte::encodeVarbyte(uint32_t val) {

    unsigned int outSize = 0;
    while (val > 127) {
        buffer[outSize] = ((uint8_t)(val & 127)) | 128;
        val >>= 7;
        outSize++;
    }
    buffer[outSize++] = ((uint8_t)val) & 127;
    return outSize;
}

// return the value of varbyte value at pointer and place how many
// byte were read inside int location in lastVarbyteSize.
uint32_t Varbyte::decodeVarbyte(const uint8_t* in) {
    unsigned int offset = 0;
    uint64_t result = 0;
    uint64_t temp;
    while (in[offset] > 127) {
        temp = in[offset] & 127;
        result |= temp << (7 * offset);
        offset++;
        if (offset > 5) {
            std::cerr << "Varbyte decoding error! 32bit int exceeded.\n";
            exit(1);
        }
    }
    temp = in[offset] & 127;
    result |= temp << (7 * offset);
    lastVarbyteSize = offset + 1;
    return result;
}

//Read the next varbyte in an ifstream.
uint32_t readVarbyte(std::ifstream &infile) {
    uint8_t byteBuffer;
    uint32_t result = 0;
    uint32_t temp;
    for (unsigned int i = 0; i < 5; i++) {
        infile.read((char*)&byteBuffer, 1);
        temp = byteBuffer;
        result |= (temp & 127) << (7 * i);
        if (byteBuffer <= 127)
            break;
    }
    return result;
}

//take a integer, encode it in varbyte and write to file.
unsigned int Varbyte::writeVarbyte(uint64_t val, FILE* fp) {
    unsigned int size = encodeVarbyte(val);
    unsigned int sizeWritten = sizeof(uint8_t) * fwrite(buffer, sizeof(uint8_t), size, fp);
    return sizeWritten;
}

unsigned int Varbyte::writeVarbyte(uint32_t val, FILE* fp) {
    unsigned int size = encodeVarbyte(val);
    unsigned int sizeWritten = sizeof(uint8_t) * fwrite(buffer, sizeof(uint8_t), size, fp);
    return sizeWritten;
}

unsigned int Varbyte::writeToBuffer(uint64_t val, char* dest) {
    unsigned int size = encodeVarbyte(val);
    memcpy(dest, buffer, size);
    return size;
}

unsigned int Varbyte::writeToBuffer(uint32_t val, char* dest) {
    unsigned int size = encodeVarbyte(val);
    memcpy(dest, buffer, size);
    return size;
}

uint8_t* Varbyte::getRawVarbyte() {
    return buffer;
}

unsigned int Varbyte::getLastReadSize() {
    return lastVarbyteSize;
}

Varbyte::Varbyte() {
    buffer = new uint8_t[9];
    lastVarbyteSize = 0;
}

Varbyte::~Varbyte() {
    delete buffer;
}