//
// Created by liuze on 2021/6/6.
//

#ifndef UDPSPEEDER_UTIL_H
#define UDPSPEEDER_UTIL_H

#include <sys/time.h>

class util {
public:
    static long long timeSub(const timeval &a, const timeval &b) {
        long long t = (a.tv_sec - b.tv_sec) * 1000 + (a.tv_usec - b.tv_usec) / 1000;
        if (t < 0) {
            t += 60 * 60 * 24 * 1000;
        }
        return t;
    }
};


#endif //UDPSPEEDER_UTIL_H
