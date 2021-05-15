//
// Created by liuze on 2021/5/11.
//

#include <arpa/inet.h>
#include "Connection.h"

Connection *Connection::connectionArray[65536];

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
            curConn = new Connection();
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
                    curConn->rdt = new RDT(curConn->WindowSize, curConn->DataShards, curConn->PacketSize,
                                           curConn->FECShards,
                                           reinterpret_cast<sockaddr *>(curConn->sock), curConn->sockLen, 10,
                                           inet_addr("192.168.62.2"), curConn->ClientSeq, curConn->serverSeq);
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
                    if (*(uint16_t *) (buffer + HEADER_LENGTH) != curConn->serverSeq + 1) {
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

void Connection::RecvBuffer(uint8_t *buffer) {
    rdt->RecvBuffer(buffer);
}
