//
// Created by liuze on 2021/5/11.
//

#include <arpa/inet.h>
#include "Connection.h"
#include "LRULinkedList.h"
#include "util.h"

std::map<uint16_t, Connection *>Connection::connectionArray;
std::map<in_addr_t, uint16_t>Connection::IP2UUID;
IPPool *Connection::ipPool = nullptr;
LRULinkedList *Connection::lru = new LRULinkedList();
std::mutex Connection::mtx;
std::vector<RDT *> Connection::rtdVec;
int Connection::pipeFD;

bool operator==(const sockaddr_in &a, const sockaddr_in &b) {
    return a.sin_addr.s_addr == b.sin_addr.s_addr && a.sin_port == b.sin_port && a.sin_family == b.sin_family;
}

int Connection::startConn(uint8_t *buffer, sockaddr *sockAddr, const socklen_t *sockAddrLen) {
    auto head = header(buffer);
    auto uuid = head.UUID();
    auto *tempSock = (sockaddr_in *) sockAddr;
    Connection *curConn = connectionArray[uuid];
    if (curConn == nullptr || *tempSock == *(sockaddr_in *) sockAddr) {
        if (curConn == nullptr) {
            curConn = new Connection(uuid);
            connectionArray[uuid] = curConn;
        }
        *(curConn->sock) = *(sockaddr_in *) sockAddr;
        *(curConn->sockLen) = *(sockAddrLen);
        switch (curConn->CurrentState) {
            case CLIENT_STATE_INITIAL:
                if (head.PacketLength() != 6) {
                    break;
                }
                curConn->DataShards = *(uint8_t *) (buffer + HEADER_LENGTH);
                curConn->FECShards = *(uint8_t *) (buffer + HEADER_LENGTH + 1);
                curConn->WindowSize = *(uint16_t *) (buffer + HEADER_LENGTH + 2);
                curConn->PacketSize = *(uint16_t *) (buffer + HEADER_LENGTH + 4);
                curConn->serverSeq = rand();
                curConn->ClientSeq = head.SendSeq();
                *(uint16_t *) (buffer + HEADER_LENGTH) = head.SendSeq() + 1;
                head.Clear();
                head.SetSendSeq(curConn->serverSeq);
                head.SetACK();
                head.SetSYN();
                head.SetPacketLength(2);
                head.SetCRC(crc32c::Crc32c(buffer + 4, HEADER_LENGTH - 2));
                curConn->CurrentState = CLIENT_STATE_WAIT_AGREE;
                return HEADER_LENGTH + 2;
            case CLIENT_STATE_WAIT_AGREE:
                if (head.IsSYN()) {
                    if (head.PacketLength() != 6) {
                        break;
                    }
                    curConn->ClientSeq = head.SendSeq();
                    curConn->DataShards = *(uint8_t *) (buffer + HEADER_LENGTH);
                    curConn->FECShards = *(uint8_t *) (buffer + HEADER_LENGTH + 1);
                    curConn->WindowSize = *(uint16_t *) (buffer + HEADER_LENGTH + 2);
                    curConn->PacketSize = *(uint16_t *) (buffer + HEADER_LENGTH + 4);
                    printf("window size is %d\n", *(uint16_t *) (buffer + HEADER_LENGTH + 2));
                    *(uint16_t *) (buffer + HEADER_LENGTH) = head.SendSeq() + 1;
                    head.Clear();
                    head.SetACK();
                    head.SetSYN();
                    head.SetSendSeq(curConn->serverSeq);
                    head.SetPacketLength(2);
                    head.SetCRC(crc32c::Crc32c(buffer + 4, HEADER_LENGTH - 2));
                    return HEADER_LENGTH + 2;
                } else if (head.IsACK()) {
                    if (head.PacketLength() != 0) {
                        break;
                    }
                    uint16_t serverSeqACK = curConn->serverSeq + 1;
                    if (head.SendSeq() != serverSeqACK) {
                        break;
                    }
                    curConn->CurrentState = CLIENT_STATE_ESTABLISHED;
                    in_addr_t assignedIP = ipPool->getIP();
                    IP2UUID[assignedIP] = uuid;
                    curConn->rdt = new RDT(curConn->WindowSize, curConn->DataShards, curConn->PacketSize,
                                           curConn->FECShards,
                                           reinterpret_cast<sockaddr *>(curConn->sock), curConn->sockLen, 10,
                                           assignedIP, curConn->ClientSeq, curConn->serverSeq, uuid);
                    mtx.lock();
                    rtdVec.push_back(curConn->rdt);
                    mtx.unlock();
                    lru->AddNewNode(uuid, curConn);
                    printf("established WINDOW_SIZE %d PACKETSIZE %d DATASHARDS %d FECSHARDS %d UUID %d\n",
                           curConn->WindowSize, curConn->PacketSize, curConn->DataShards, curConn->FECShards, uuid);
                    head.Clear();
                    head.SetACK();
                    head.SetCRC(crc32c::Crc32c(buffer + 4, HEADER_LENGTH - 4));
                    return HEADER_LENGTH;
                }
                break;
            case CLIENT_STATE_ESTABLISHED:
                if (head.PacketLength() != 0) {
                    break;
                }
                uint16_t serverSeqACK = curConn->serverSeq + 1;
                if (head.IsACK()) {
                    if (*(uint16_t *) (buffer + HEADER_LENGTH) != serverSeqACK) {
                        break;
                    }
                    head.Clear();
                    head.SetACK();
                    head.SetCRC(crc32c::Crc32c(buffer + 4, HEADER_LENGTH - 4));
                    return HEADER_LENGTH;
                }
                break;
        }
    }
    head.Clear();
    head.SetRST();
    head.SetCRC(crc32c::Crc32c(buffer + 4, HEADER_LENGTH - 4));
    return HEADER_LENGTH;
}

Connection *Connection::getConn(uint16_t uuid) {
    return connectionArray[uuid];
}

Connection *Connection::GetConnectionByIP(in_addr_t ip) {
    auto tempUUID = IP2UUID[ip];
    return tempUUID == 0 ? nullptr : connectionArray[tempUUID];
}

void Connection::AddData(uint8_t *buffer, uint16_t length) {
    if (rdt->AddData(buffer, length)) {
        lru->moveToTail(uuid);
    }
}

void Connection::callBack() {
    rdt->BufferTimeOut();
}

void Connection::checkTimeOut() {
    lru->checkTimeOut();
}

void Connection::RecvBuffer(uint8_t *buffer, const sockaddr_in &addr, const socklen_t &socklen) {
    rdt->RecvBuffer(buffer);
    *sock = addr;
    *sockLen = socklen;
}

void Connection::speedWatcher() {
    timeval curTime{};
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
        auto iter = rtdVec.begin();
        while (iter != rtdVec.end()) {
            auto &i = *iter;
            timeval lastTime{i->lastActiveTime.tv_sec, i->lastActiveTime.tv_usec};
            gettimeofday(&curTime, nullptr);
            if (util::timeSub(curTime, lastTime) > 60000) {
                printf("%lld\n", util::timeSub(curTime, lastTime));
                Connection::IP2UUID[i->fakeIP] = 0;
                Connection::connectionArray[i->uuid] = nullptr;
                delete i;
                rtdVec.erase(iter);
            } else {
                if (c) {
                    ss << ",";
                }
                ss << "{ \"uuid\":" << i->uuid << ", \"up_speed\":" << i->upSpeed << ", \"down_speed\":" << i->downSpeed
                   << "}";
                c = true;
                i->upSpeed = 0;
                i->downSpeed = 0;
                iter++;
            }
        }
        mtx.unlock();
        ss << "]";
        auto s = ss.str();
        write(pipeFD, s.c_str(), s.size());
    }
}