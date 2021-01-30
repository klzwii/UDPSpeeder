//
// Created by liuze on 2021/1/24.
//

#ifndef UDPSPEEDER_SIMPLEWORKQUEUE_H
#define UDPSPEEDER_SIMPLEWORKQUEUE_H


#include "SimpleWorker.h"
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

class SimpleWorkQueue {
private:
    class SimpleThreadSafeQueue {
    private:
        std::queue<SimpleTask*>que;
        std::mutex mMutex;
        std::condition_variable mCondVar;
        std::unique_lock<std::mutex>* uniqueLock;
    public:
        SimpleThreadSafeQueue() {
            uniqueLock = new std::unique_lock<std::mutex>(mMutex, std::defer_lock);
        }
        SimpleTask* offer() {
            uniqueLock->lock();
            while(que.empty()) {
                mCondVar.wait(*uniqueLock);
            }
            auto ret = que.front();
            que.pop();
            uniqueLock->release();
            return ret;
        }
        void push(const SimpleTask* task) {
            uniqueLock->lock();
            que.push(const_cast<SimpleTask *>(task));
            if (que.size() == 1) {
                mCondVar.notify_all();
            }
            uniqueLock->unlock();
        }
        ~ SimpleThreadSafeQueue() {
            delete(uniqueLock);
        }
    };
    SimpleWorker **workers{};
    unsigned int workerSize;
    const static SimpleTask *finishTask;
    std::atomic<bool>finished{};
    static void simpleWorkFunc(SimpleWorker *simpleWorker, SimpleThreadSafeQueue* queue);
    SimpleThreadSafeQueue queue;
public:
    SimpleWorkQueue(int workerSize, SimpleWorker** simpleWorkers);
    void submitTask(SimpleTask *task);
    void finish();
};


#endif //UDPSPEEDER_SIMPLEWORKQUEUE_H
