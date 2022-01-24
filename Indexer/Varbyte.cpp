//
// Created by yufan on 10/13/21.
//
#include <fstream>
#include <cstring>
#include "Varbyte.h"

std::string toBinary(int n);

// write varbyte to pointer and return how many bytes it wrote.
unsigned int Vbyte::encodeVbyte(uint64_t val) {

    unsigned int outSize = 0;
    while (val > 127) {
        buffer[outSize] = ((uint8_t)(val & 127)) | 128;
        val >>= 7;
        outSize++;
    }
    buffer[outSize++] = ((uint8_t)val) & 127;
    return outSize;
}

unsigned int Vbyte::encodeVbyte(uint32_t val) {

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
// byte were read inside int location.
uint64_t Vbyte::decodeVbyte(const uint8_t* in) {
    unsigned int offset = 0;
    uint64_t result = 0;
    uint64_t temp;
    while (in[offset] > 127) {
        temp = in[offset] & 127;
        result |= temp << (7 * offset);
        offset++;
        if (offset > 8) {
            std::cerr << "Varbyte decoding error! 64bit int exceeded.\n";
            exit(1);
        }
    }
    temp = in[offset] & 127;
    result |= temp << (7 * offset);
    lastVbyteSize = offset + 1;
    return result;
}

//take a integer, encode it in varbyte and write to file.
unsigned int Vbyte::writeVbyte(uint64_t val, FILE* fp) {
    unsigned int size = encodeVbyte(val);
    unsigned int sizeWritten = sizeof(uint8_t) * fwrite(buffer, sizeof(uint8_t), size, fp);
    return sizeWritten;
}

unsigned int Vbyte::writeVbyte(uint32_t val, FILE* fp) {
    unsigned int size = encodeVbyte(val);
    unsigned int sizeWritten = sizeof(uint8_t) * fwrite(buffer, sizeof(uint8_t), size, fp);
    return sizeWritten;
}

unsigned int Vbyte::writeToBuffer(uint64_t val, char* dest) {
    unsigned int size = encodeVbyte(val);
    memcpy(dest, buffer, size);
    return size;
}

unsigned int Vbyte::writeToBuffer(uint32_t val, char* dest) {
    unsigned int size = encodeVbyte(val);
    memcpy(dest, buffer, size);
    return size;
}

uint8_t* Vbyte::getRawVbyte() {
    return buffer;
}

unsigned int Vbyte::getLastReadSize() {
    return lastVbyteSize;
}

Vbyte::Vbyte() {
    buffer = new uint8_t[9];
    lastVbyteSize = 0;
}

Vbyte::~Vbyte() {
    delete buffer;
}