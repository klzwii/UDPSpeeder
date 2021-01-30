//
// Created by liuze on 2021/1/24.
//

#include "SimpleWorkQueue.h"
#include <atomic>

const SimpleTask* SimpleWorkQueue::finishTask = new SimpleTask();

SimpleWorkQueue::SimpleWorkQueue(int workerSize, SimpleWorker **simpleWorkers) {
    finished = false;
    workers = reinterpret_cast<SimpleWorker **>(workerSize * sizeof(SimpleWorker*));
    memcpy(workers, simpleWorkers, workerSize * sizeof(SimpleWorker*));
    this->workerSize = workerSize;
}

void SimpleWorkQueue::simpleWorkFunc(SimpleWorker *simpleWorker, SimpleThreadSafeQueue* queue) {
    while (true) {
        SimpleTask *task = queue->offer();
        if (task == finishTask) {
            break;
        }
        simpleWorker->work(task);
    }
}

void SimpleWorkQueue::submitTask(SimpleTask *task) {
    if (finished) {
        return;
    }
    submitTask(task);
}

void SimpleWorkQueue::finish() {
    finished.store(true);
    for (int i = 0; i < workerSize; i ++) {
        queue.push(finishTask);
    }
}

