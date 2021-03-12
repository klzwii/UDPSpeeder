#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <sys/types.h>
#include <linux/if_tun.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include "SimpleWorkQueue/SimpleWorkQueue.h"
#include "PackageProcessor/PackageProcessWorker.h"
#include "PackageProcessor/PackageProcessTask.h"

int main(){
    auto k = new RSHelper();
    int originalLength = 5, rsLength = 2;
    auto packetSize = 1000;
    auto packets =  reinterpret_cast<unsigned char **>(malloc(originalLength + rsLength));
    for (int i = 0; i < originalLength + rsLength; i ++) {
        packets[i] = reinterpret_cast<unsigned char *>(malloc(packetSize * sizeof(unsigned char)));
    }
    k->generateRSPacket(packets, originalLength, rsLength, packetSize);
    std::cout << "11111";
    if (k->GetOriginMessageFromPackets(packets, originalLength, rsLength, packetSize)) {
        std::cout << "succeed";
    } else {
        std::cout << "fail";
    }
    std::cout << "11111";
}