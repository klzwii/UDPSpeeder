#include "RSHelper.h"
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
void test();
std::mt19937 g1(time(nullptr));
bool isStart = false;
int main() {
    RSHelper::init();
    for (int i = 0; i < 8; i ++) {
        std::thread(test).detach();
    }
    isStart = true;
    while (true) {

    }
}
void test() {
   // freopen("test.txt", "w", stdout);
    RSHelper k;
    int message[gwSize];
    int messageCopy[gwSize];
    int lun = 0;
    while (true) {
        int messageLength = g1() % gwSize, rsCodeLength = gwSize - 1 - messageLength;
        //messageLength = gwSize - 1, rsCodeLength = 0;
        if (lun == 10) {
            lun = 0;
            std::cout << "time" << std::endl;
        }
        lun ++;
        memset(message, 0, sizeof(message));
        for (int i = rsCodeLength; i < messageLength + rsCodeLength; i++) {
            message[i] = g1() % gwSize;
        }
        k.attachRSCode(message, messageLength, rsCodeLength);
        std::memcpy(messageCopy, message, sizeof(message));
        int c = rsCodeLength >> 1;
        std::cout << c << std::endl;
        while (c) {
            int pos =g1() % gwSize;
            int changed = g1() % gwSize;
            message[pos] = changed;
            c--;
        }

        if (!RSHelper::getOriginMessage(message, messageLength, rsCodeLength)) {
            std::cout << "wrong";
            std::cout << rsCodeLength << " " << messageLength << std::endl;
            std::cout << std::endl;
            for (int i = messageLength + rsCodeLength - 1; i >= rsCodeLength; i --) {
                std::cout << message[i] << ",";
            }
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
