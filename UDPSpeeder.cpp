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
    int workerSize = 4;
    auto workerArray = reinterpret_cast<SimpleWorker**>(malloc(sizeof(SimpleWorker*) * workerSize));
    for (int i = 0; i < workerSize; i ++) {
        workerArray[i] = new PackageProcessWorker();
//        auto k = new PackageProcessTask();
//        k->call = i;
//        workerArray[i]->work(k);
    }
    std::cout << "test worker success" << std::endl;
    auto threadPool = new SimpleWorkQueue(workerSize, workerArray);
    std::cout << "init thread pool success" << std::endl;
    for (int i = 0; i < 100; i ++) {
        auto k = new PackageProcessTask();
        k->call = i;
        threadPool->submitTask(k);
    }
    threadPool->finish();
    while (true) {

    }
}