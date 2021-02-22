#include "RSHelper.h"
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
#include <cstring>
#include <malloc.h>
#include "SimpleWorkQueue/SimpleWorker.h"
#include "SimpleWorkQueue/SimpleWorkQueue.h"
#include "PackageProcessor/PackageProcessWorker.h"
#define debug
#include "PackageProcessor/PackageProcessTask.h"
void test();
std::mt19937 g1(time(nullptr));
bool isStart = false;
const int threadPoolSize = 4;
const int taskPoolSize = 8;
int main() {
    auto worker = reinterpret_cast<SimpleWorker**>(malloc(threadPoolSize * sizeof(SimpleWorker*)));
    for (int i = 0; i < threadPoolSize; i ++) {
        worker[i] = new PackageProcessWorker();
    }
    auto tasks = reinterpret_cast<SimpleTask**>(malloc(taskPoolSize * sizeof(SimpleTask*)));
    auto queue = new SimpleWorkQueue::SimpleThreadsSafeQueue()
    for (int i = 0; i < taskPoolSize; i ++) {
        auto *message = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
        auto *messageCopy = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
        tasks[i] = new PackageProcessTask(message, messageCopy, 3000, 1000);
    }
    int cc = 4;
    while (cc--)
    std::thread(test).detach();
    std::this_thread::sleep_for(std::chrono::seconds(1000));
}
void test() {
    auto k = new RSHelper();
    auto *message = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
    auto *messageCopy = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
    int lun = 0;
    while (true) {
        int messageLength = 3000, rsCodeLength = 1000;
        //messageLength = gwSize - 1, rsCodeLength = 0;
        if (lun == 10) {
            lun = 0;
            std::cout << "time" << std::endl;
        }
        lun ++;
        memset(message, 0, sizeof(message));
        for (int i = rsCodeLength; i < messageLength + rsCodeLength; i++) {
            RSHelper::setPos(i, message, g1() % gwSize);
        }
        k->attachRSCode(message, messageLength, rsCodeLength);
        std::memcpy(messageCopy, message, 7000 * sizeof(unsigned char));
        int c = rsCodeLength >> 1;
        std::cout << c << std::endl;
        while (c) {
            int pos =g1() % gwSize;
            int changed = g1() % gwSize;
            RSHelper::setPos(pos, message, changed);
            c--;
        }
        auto startTime = std::clock();
        if (!k->getOriginMessage(message, messageLength, rsCodeLength)) {
            std::cout << "wrong";
            std::cout << rsCodeLength << " " << messageLength << std::endl;
            std::cout << std::endl;
            for (int i = messageLength + rsCodeLength - 1; i >= rsCodeLength; i --) {
                std::cout << RSHelper::getPos(i, message) << ",";
            }
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << RSHelper::getPos(i, message)  << ",";
            }
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << RSHelper::getPos(i, messageCopy) << ",";
            }
            std::cout << std::endl;
            break;
        }
        auto endTime = std::clock();
        std::cout << "The run time is: " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    }
    free(k);
}
