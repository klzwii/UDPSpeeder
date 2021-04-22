//
// Created by liuze on 2021/4/16.
//

#ifndef UDPCOMMON_RS_H
#define UDPCOMMON_RS_H

#include "GF.h"

class RS {

    galois::gf **encodeMatrix;
    int DataShards, FECShards;

public:
    void encode(uint8_t **shards, unsigned ShardLength);

    void decode(uint8_t **shards, unsigned ShardLength, const bool validShards[]);

    RS(int DataShards, int FECShards) {
        this->DataShards = DataShards;
        this->FECShards = FECShards;
        encodeMatrix = galois::cauchyMatrix(DataShards + FECShards, DataShards);
    }
};


#endif //UDPCOMMON_RS_H
