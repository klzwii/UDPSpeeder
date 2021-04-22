//
// Created by liuze on 2021/2/2.
//

#ifndef UDPSPEEDER_PACKAGEPROCESSWORKER_H
#define UDPSPEEDER_PACKAGEPROCESSWORKER_H


#include "../SimpleWorkQueue/SimpleWorker.h"
#include "../SimpleWorkQueue/SimpleWorkQueue.h"

class PackageProcessWorker : public SimpleWorker {
public:
    void work(SimpleTask* task) override;
    PackageProcessWorker();
private:
    SimpleWorkQueue::SimpleThreadsSafeQueue *queue;
};


#endif //UDPSPEEDER_PACKAGEPROCESSWORKER_H
