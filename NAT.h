//
// Created by liuze on 2021/5/13.
//

#ifndef UDPSPEEDER_NAT_H
#define UDPSPEEDER_NAT_H

#include <arpa/inet.h>
#include <cstring>
#include <hiredis/hiredis.h>

class NAT {
private:
    uint16_t UDPPort[65536];
    uint16_t OriUDPPort[65536];
    uint16_t TCPPort[65536];
    uint16_t OriTCPPort[65536];
    uint16_t ICMPId[65536];
    uint16_t OriICMPId[65536];
    in_addr_t fakeIP;
public:
    NAT(in_addr_t fakeIP) {
        this->fakeIP = fakeIP;
        bzero(UDPPort, sizeof(UDPPort));
        bzero(TCPPort, sizeof(TCPPort));
        bzero(ICMPId, sizeof(ICMPId));
        bzero(OriICMPId, sizeof(OriICMPId));
        bzero(OriTCPPort, sizeof(OriTCPPort));
        bzero(OriUDPPort, sizeof(OriUDPPort));
    }

    uint16_t GetUUID(uint16_t port, int Type);

    uint16_t GetPort(uint16_t uuid, uint16_t port, int Type);

    bool TransferOutputPackets(uint8_t *buffer, uint16_t length, uint16_t uuid);

    uint16_t TransferInputPackets(uint *buffer, uint16_t length);

    uint16_t calcCheckSum(uint16_t *data, size_t len, const uint16_t *fakeHead);

    void reCalcChecksum(uint16_t *payLoad, size_t len);
};


#endif //UDPSPEEDER_NAT_H
