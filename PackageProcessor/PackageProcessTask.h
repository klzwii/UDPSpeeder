//
// Created by liuze on 2021/2/2.
//

#ifndef UDPSPEEDER_PACKAGEPROCESSTASK_H
#define UDPSPEEDER_PACKAGEPROCESSTASK_H


#include "../SimpleWorkQueue/SimpleTask.h"
#include "../RSHelper.h"

class PackageProcessTask : public SimpleTask {
public:
    unsigned char * bytes{};
    int messageLength, rsCodeLength;
    bool isAttach;
    PackageProcessTask(unsigned char* message, int messageLength, int rsCodeLength, bool isAttach) {
        this->bytes = message;
        this->messageLength = messageLength;
        this->rsCodeLength = rsCodeLength;
        this->isAttach = isAttach;
    };
    ~PackageProcessTask() override {
        free(bytes);
    }
};


#endif //UDPSPEEDER_PACKAGEPROCESSTASK_H
