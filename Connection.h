//
// Created by liuze on 2021/5/11.
//

#ifndef UDPWINCLIENT_CONNECTION_H
#define UDPWINCLIENT_CONNECTION_H

#include "RDT.h"
#include <atomic>
#include "crc32c/crc32c.h"
#include "IPPool.h"
#include "LRULinkedList.h"
#include <map>
#include "LinkedNodeCallBack.h"
#include <mutex>
#include <vector>
#include <sstream>

class Connection : public LinkedNodeCallBack {
private:
    RDT *rdt;
    sockaddr_in *sock;
    socklen_t *sockLen;
    static const uint8_t CLIENT_STATE_INITIAL = 1;
    static const uint8_t CLIENT_STATE_WAIT_AGREE = 2;
    static const uint8_t CLIENT_STATE_ESTABLISHED = 3;
    uint8_t CurrentState;
    uint16_t ClientSeq;
    uint16_t serverSeq;
    uint8_t DataShards;
    uint16_t WindowSize;
    uint16_t PacketSize;
    uint8_t FECShards;
    uint16_t uuid;
    static int pipeFD;
    static IPPool *ipPool;
    static std::map<in_addr_t, uint16_t> IP2UUID;
    static std::map<uint16_t, Connection *> connectionArray;
    static LRULinkedList *lru;
    Connection(uint16_t uuid) {
        sock = new sockaddr_in;
        sockLen = new socklen_t;
        CurrentState = CLIENT_STATE_INITIAL;
        this->uuid = uuid;
    }

    ~Connection() {
        delete sock;
        delete sockLen;
    }

    static std::mutex mtx;

    static std::vector<RDT *> rtdVec;

    static void speedWatcher() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::stringstream ss;
            ss << "[";
            bool c = false;
            mtx.lock();
            if (rtdVec.empty()) {
                mtx.unlock();
                continue;
            }
            for (auto &i : rtdVec) {
                if (c) {
                    ss << ",";
                }
                ss << "{ \"uuid\":" << i->uuid << ", \"up_speed\":" << i->upSpeed << ", \"down_speed\":" << i->downSpeed
                   << "}";
                c = true;
                i->upSpeed = 0;
                i->downSpeed = 0;
            }
            mtx.unlock();
            ss << "]";
            auto s = ss.str();
            write(pipeFD, s.c_str(), s.size());
        }
    }

public:
    static void init(IPPool *pool, int fd) {
        Connection::ipPool = pool;
        std::thread(speedWatcher).detach();
        pipeFD = fd;
    }

    static Connection *getConn(uint16_t uuid);

    static int startConn(uint8_t *buffer, sockaddr *sockAddr, const socklen_t *sockAddrLen);

    void RecvBuffer(uint8_t *buffer, const sockaddr_in &addr, const socklen_t &socklen);

    void AddData(uint8_t *buffer, uint16_t length);

    static Connection *GetConnectionByIP(in_addr_t ip);

    void callBack() override;

    static void checkTimeOut();
};


#endif //UDPWINCLIENT_CONNECTION_H
