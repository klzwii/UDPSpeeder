#include <cstring>
#include <cstdlib>
#include <iostream>
#include "PackageProcessor/PackageProcessWorker.h"
#include "crc32c/crc32c.h"

int main(){
    auto k = new RSHelper();
    srand(time(nullptr));
    int originalLength = 5, rsLength = 2;
    auto packetSize = 1000;
    auto packets =  reinterpret_cast<unsigned char **>(malloc((originalLength + rsLength) * sizeof(unsigned char *)));
    for (int i = 0; i < originalLength + rsLength; i ++) {
        packets[i] = reinterpret_cast<unsigned char *>(malloc(packetSize * sizeof(unsigned char)));
        for (int j = 0; j < packetSize; j ++) {
            (packets[i][j]) = rand();
        }
    }
    uint8_t testPacket[packetSize];
    memcpy(testPacket, packets[2], packetSize);
    std::cout << memcmp(testPacket, packets[2], packetSize) << std::endl;
    k->GenerateRSPacket(packets, originalLength, rsLength, packetSize);
    for (int i = 0; i < rsLength + originalLength; i ++) {
        std::cout << crc32c::Crc32c(packets[i], packetSize) << " ";
    }
    memset(packets[2], 0, packetSize);
    std::cout << std::endl;
    for (int i = 0; i < rsLength + originalLength; i ++) {
        std::cout << crc32c::Crc32c(packets[i], packetSize) << " ";
    }
    std::cout << std::endl;
    if (k->GetOriginMessageFromPackets(packets, originalLength, rsLength, packetSize)) {
        std::cout << "succeed";
    } else {
        std::cout << "fail";
    }
    std::cout << std::endl;
    for (int i = 0; i < rsLength + originalLength; i ++) {
        std::cout << crc32c::Crc32c(packets[i], packetSize) << " ";
    }
    std::cout << std::endl << memcmp(testPacket, packets[2], packetSize);
}