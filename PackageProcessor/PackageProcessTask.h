//
// Created by liuze on 2021/2/2.
//

#ifndef UDPSPEEDER_PACKAGEPROCESSTASK_H
#define UDPSPEEDER_PACKAGEPROCESSTASK_H


#include "../SimpleWorkQueue/SimpleTask.h"
#include "../RSHelper.h"

class PackageProcessTask : public SimpleTask {
public:
    char* bytes{};
    int length;
};


#endif //UDPSPEEDER_PACKAGEPROCESSTASK_H
