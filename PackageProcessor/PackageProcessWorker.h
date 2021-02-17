//
// Created by liuze on 2021/2/2.
//

#ifndef UDPSPEEDER_PACKAGEPROCESSWORKER_H
#define UDPSPEEDER_PACKAGEPROCESSWORKER_H


#include "../SimpleWorkQueue/SimpleWorker.h"

class PackageProcessWorker : public SimpleWorker {
    void work(SimpleTask* task) override;
};


#endif //UDPSPEEDER_PACKAGEPROCESSWORKER_H
