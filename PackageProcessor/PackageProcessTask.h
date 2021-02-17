//
// Created by liuze on 2021/2/2.
//

#ifndef UDPSPEEDER_PACKAGEPROCESSTASK_H
#define UDPSPEEDER_PACKAGEPROCESSTASK_H


#include "../SimpleWorkQueue/SimpleTask.h"

class PackageProcessTask : public SimpleTask {
public:
    int call;
};


#endif //UDPSPEEDER_PACKAGEPROCESSTASK_H
