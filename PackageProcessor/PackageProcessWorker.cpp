//
// Created by liuze on 2021/2/2.
//

#include "PackageProcessWorker.h"
#include "PackageProcessTask.h"
#include <iostream>
#include <thread>
#include <random>
#include <chrono>

void PackageProcessWorker::work(SimpleTask *task) {
    auto packageTask = dynamic_cast<PackageProcessTask*>(task);
    std::cout << packageTask->call << std::endl;
    free(packageTask);
}
