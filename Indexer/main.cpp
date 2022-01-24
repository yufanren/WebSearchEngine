#include "FileLoader.h"
#include "IndexGenerator.h"
#include "Varbyte.h"
#include <ctime>

int main(int argc, const char * argv[]) {

    time_t start = time(nullptr);

//    processSource(FILE_MODE_BINARY);

//    MergeTempIndex();

    GenerateInvertedIndex(FILE_MODE_BINARY);

    time_t total = difftime(time(nullptr), start);
    long min = total / 60;
    long sec = total % 60;
    std::cout <<"Running time: " << min << " minutes " << sec << " seconds.\n";
    return 0;


    //varbyte test
//    Vbyte varbyte = Vbyte();
//    unsigned long long test = 3464460;
//
//    auto writtenSize = varbyte.encodeVbyte(test);
//    uint64_t result = varbyte.decodeVbyte(varbyte.getRawVbyte());
//
//    std::cout << "Number is " << test << std::endl;
//    std::cout << "Size written is " << writtenSize << ", final number is " << result;

    return 0;
}



