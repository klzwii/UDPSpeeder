//
// Created by liuze on 2021/4/16.
//

#ifndef UDPSPEEDER_GF_H
#define UDPSPEEDER_GF_H

#include <atomic>
#include <cstdio>
#include <cstring>
#include <malloc.h>

namespace galois {
    typedef uint8_t gf;

    void initGF();

    gf mul(gf a, gf b);

    gf div(gf a, gf b);

    gf **cauchyMatrix(int m, int k);
}


#endif //UDPSPEEDER_GF_H
