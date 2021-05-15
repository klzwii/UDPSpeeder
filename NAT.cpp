//
// Created by liuze on 2021/5/13.
//

#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include "NAT.h"

uint16_t NAT::calcCheckSum(uint16_t *data, size_t len, const uint16_t *fakeHead) {
    uint32_t cksum = 0;
    if (fakeHead != nullptr) {
        for (int i = 0; i < 6; i++) {
            cksum += *(fakeHead + i);
        }
    }
    while (len > 1) {
        cksum += *(data++);
        len -= 2;
    }
    if (len == 1) {
        cksum += *(uint8_t *) data;
    }
    while (cksum >> 16) {
        uint16_t t = cksum >> 16;
        cksum &= 0x0000ffff;
        cksum += t;
    }
    return cksum;
}

void NAT::reCalcChecksum(uint16_t *payLoad, size_t len) {
    auto *iph = (iphdr *) payLoad;
    uint16_t headLen = iph->ihl * 4;
    uint8_t fakeHead[12];
    if (iph->protocol != IPPROTO_ICMP) {
        *(uint32_t *) fakeHead = iph->saddr;
        *(uint32_t *) (fakeHead + 4) = iph->daddr;
        *(uint8_t *) (fakeHead + 8) = 0;
        *(uint8_t *) (fakeHead + 9) = iph->protocol;
        *(uint16_t *) (fakeHead + 10) = htobe16((uint16_t) (len - headLen));
        if (iph->protocol == IPPROTO_TCP) {
            *(payLoad + headLen / 2 + 8) = 0;
            *(payLoad + headLen / 2 + 8) = ~calcCheckSum(payLoad + headLen / 2, len - headLen, (uint16_t *) fakeHead);
        }
        if (iph->protocol == IPPROTO_UDP) {
            *(payLoad + headLen / 2 + 3) = 0;
            *(payLoad + headLen / 2 + 3) = ~calcCheckSum(payLoad + headLen / 2, len - headLen, (uint16_t *) fakeHead);
        }
    }
    iph->check = 0;
    iph->check = ~(calcCheckSum(payLoad, headLen, nullptr));
}

uint16_t NAT::GetUUID(uint16_t port, int Type) {
    switch (Type) {
        case IPPROTO_UDP:
            return UDPPort[port];
        case IPPROTO_TCP:
            return TCPPort[port];
        case IPPROTO_ICMP:
            return ICMPId[port];
        default:
            return 0;
    }
}

uint16_t NAT::GetPort(uint16_t uuid, uint16_t port, int type) {
    uint16_t *array;
    uint16_t *oriArray;
    switch (type) {
        case IPPROTO_UDP:
            array = UDPPort;
            oriArray = OriUDPPort;
            break;
        case IPPROTO_TCP:
            array = TCPPort;
            oriArray = OriTCPPort;
            break;
        case IPPROTO_ICMP:
            array = ICMPId;
            oriArray = OriICMPId;
            break;
        default:
            return 0;
    }
    int i;
    if (array[port] == uuid) {
        return port;
    }
    if (!array[port]) {
        array[port] = uuid;
        oriArray[port] = port;
        return port;
    }
    for (i = 1024; i < 65535; i++) {
        if (array[i] || !i) {
            array[i] = uuid;
            oriArray[i] = port;
            break;
        }
    }
    return i;
}

bool NAT::TransferOutputPackets(uint8_t *buffer, uint16_t length, uint16_t uuid) {
    if (length < 20) {
        return false;
    }
    auto *head = (iphdr *) buffer;
    uint8_t IPHeaderOffset = 4 * head->ihl;
    head->saddr = fakeIP;
    if (head->protocol == IPPROTO_UDP) {
        if (length < sizeof(udphdr) + IPHeaderOffset) {
            return false;
        }
        auto *hdr = (udphdr *) (buffer + IPHeaderOffset);
        hdr->source = GetPort(uuid, hdr->source, IPPROTO_UDP);
    } else if (head->protocol == IPPROTO_UDP) {
        if (length < sizeof(tcphdr) + IPHeaderOffset) {
            return false;
        }
        auto *hdr = (tcphdr *) (buffer + IPHeaderOffset);
        hdr->source = GetPort(uuid, hdr->source, IPPROTO_TCP);
    } else if (head->protocol == IPPROTO_ICMP) {
        if (length < sizeof(tcphdr) + IPHeaderOffset) {
            return false;
        }
        auto *hdr = (icmphdr *) (buffer + IPHeaderOffset);
        if (hdr->type != ICMP_ECHO && hdr->type != ICMP_ECHOREPLY) {
            return true;
        }
        hdr->un.echo.id = GetPort(uuid, hdr->un.echo.id, IPPROTO_ICMP);
    } else {
        return false;
    }
    reCalcChecksum((uint16_t *) buffer, length);
    return true;
}

uint16_t NAT::TransferInputPackets(uint *buffer, uint16_t length) {
    if (length < 20) {
        return 0;
    }
    auto *head = (iphdr *) buffer;
    uint8_t IPHeaderOffset = 4 * head->ihl;
    head->saddr = fakeIP;
    if (head->protocol == IPPROTO_UDP) {
        if (length < sizeof(udphdr) + IPHeaderOffset) {
            return 0;
        }
        auto *hdr = (udphdr *) (buffer + IPHeaderOffset);
        uint16_t uuid = UDPPort[hdr->dest];
        hdr->source = OriUDPPort[hdr->dest];
        return uuid;
    } else if (head->protocol == IPPROTO_UDP) {
        if (length < sizeof(tcphdr) + IPHeaderOffset) {
            return 0;
        }
        auto *hdr = (tcphdr *) (buffer + IPHeaderOffset);
        uint16_t uuid = TCPPort[hdr->dest];
        hdr->source = OriTCPPort[hdr->dest];
        return uuid;
    } else if (head->protocol == IPPROTO_ICMP) {
        if (length < sizeof(tcphdr) + IPHeaderOffset) {
            return 0;
        }
        auto *hdr = (icmphdr *) (buffer + IPHeaderOffset);
        if (hdr->type != ICMP_ECHO && hdr->type != ICMP_ECHOREPLY) {
            return 0;
        }
        uint16_t uuid = ICMPId[hdr->un.echo.id];
        hdr->un.echo.id = OriICMPId[hdr->un.echo.id];
        return uuid;
    }
    return 0;
}
