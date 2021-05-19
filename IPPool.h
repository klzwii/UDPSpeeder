//
// Created by liuze on 2021/5/18.
//

#ifndef UDPSPEEDER_IPPOOL_H
#define UDPSPEEDER_IPPOOL_H

#include <arpa/inet.h>
#include <cstring>

class IPPool {
    in_addr_t IPPrefix;
    int prefix;
    uint32_t count;

    IPPool(in_addr_t IPPrefix, int prefix) {
        this->IPPrefix = IPPrefix;
        this->prefix = prefix;
        count = 0;
    }

public:
    in_addr_t getIP() {
        if (count > (1 << (32 - prefix)) - 2) {
            return INADDR_NONE;
        }
        ++count;
        return IPPrefix | htonl(count);
    }

    static IPPool *NewIPPool(const std::string &IP, int prefix) {
        if (prefix > 30 || prefix < 0) {
            return nullptr;
        }
        in_addr_t ip = inet_addr(IP.c_str());
        if (ip == INADDR_NONE) {
            return nullptr;
        }
        int tempPrefix = 32 - prefix;
        auto *k = (uint8_t *) (&ip);
        k += 3;
        for (int i = 3; i >= 0; i++) {
            int t = (1 << std::min(tempPrefix, 8)) - 1;
            *k &= ~t;
            tempPrefix -= 8;
            --k;
            if (tempPrefix <= 0) {
                break;
            }
        }
        return new IPPool(ip, prefix);
    }
};


#endif //UDPSPEEDER_IPPOOL_H
