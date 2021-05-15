//
// Created by liuze on 2021/5/11.
//

#ifndef UDPWINCLIENT_CONNECTION_H
#define UDPWINCLIENT_CONNECTION_H

#include "RDT.h"
#include <atomic>
#include "crc32c/crc32c.h"

class Connection {
public:
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
    static Connection *connectionArray[65536];

    static void init() {
        bzero(connectionArray, sizeof(connectionArray));
    }

    Connection() {
        sock = new sockaddr_in;
        sockLen = new socklen_t;
        CurrentState = CLIENT_STATE_INITIAL;
    }

    ~Connection() {
        delete sock;
        delete sockLen;
    }

public:
    static Connection *getConn(uint16_t uuid);

    static int startConn(uint8_t *buffer, sockaddr *sockAddr, const socklen_t *sockAddrLen);

    void RecvBuffer(uint8_t *buffer);
};


#endif //UDPWINCLIENT_CONNECTION_H
