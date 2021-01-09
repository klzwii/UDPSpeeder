#include "RSHelper.h"
#include <ctime>
#include <iostream>
#include <random>
int main() {
    std::srand(std::time(NULL));
    RSHelper k;
    int message[256];
    int messageCopy[256];
    int messageLength = 128, rsCodeLength = 127;
    int lun = 0;
    while (true) {
        if (lun == 1000) {
            lun = 0;
            std::cout << "time" << std::endl;
        }
        lun ++;
        memset(message, 0, sizeof(message));
        for (int i = rsCodeLength; i < messageLength + rsCodeLength; i++) {
            message[i] = std::rand() % 256;
        }
        k.attachRSCode(message, messageLength, rsCodeLength);
        std::memcpy(messageCopy, message, sizeof(message));
        int c = rand() % 64;
        while (c) {
            int pos = rand() % 64;
            int changed = rand() % 256;
            message[pos] = changed;
            c--;
        }

        if (!k.getOriginMessage(message, messageLength, rsCodeLength)) {
            std::cout << "wrong";
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << message[i] << ",";
            }
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << messageCopy[i] << ",";
            }
            std::cout << std::endl;

            break;
        }
    }
}
