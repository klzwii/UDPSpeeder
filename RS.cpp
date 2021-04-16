//
// Created by liuze on 2021/4/12.
//

#include "RS.h"
#include <cstdio>
#include <cstring>

void RS::encode(uint8_t **shards, int ShardLength) {
    for (int i = DataShards; i < DataShards + FECShards; i++) {
        for (int j = 0; j < ShardLength; j++) {
            for (int k = 0; k < DataShards; k++) {
                shards[i][j] ^= galois::mul(encodeMatrix[i][k], shards[k][j]);
            }
        }
    }
}

void RS::decode(uint8_t **shards, int ShardLength, const bool validShards[]) {
    galois::gf originMat[DataShards][DataShards];
    galois::gf invMat[DataShards][DataShards];
    memset(originMat, 0, sizeof(originMat));
    memset(invMat, 0, sizeof(invMat));
    int j = DataShards;
    for (int i = 0; i < DataShards; i++) {
        invMat[i][i] = 1;
        if (!validShards[i]) {
            bool valid = false;
            for (; j < DataShards + FECShards; j++) {
                if (validShards[j]) {
                    memcpy(shards[i], shards[j], ShardLength * sizeof(galois::gf));
                    memcpy(originMat[i], encodeMatrix[j], DataShards * sizeof(galois::gf));
                    valid = true;
                    j++;
                    break;
                }
            }
            if (!valid) {
                printf("don't have enough shards to recover data");
                return;
            }
        } else {
            originMat[i][i] = 1;
        }
    }
#ifdef debug
    galois::gf originMatCopy[DataShards][DataShards];
    memcpy(originMatCopy, originMat, sizeof(originMat));
#endif
    // first step transform into upper triangle matrix
    for (int i = 0; i < DataShards; i++) {
        if (validShards[i]) {
            continue;
        }
        for (j = 0; j < i; j++) {
            if (originMat[i][j]) {
                galois::gf factor = originMat[i][j];
                for (int k = 0; k < DataShards; k++) {
                    originMat[i][k] ^= galois::mul(originMat[j][k], factor);
                    invMat[i][k] ^= galois::mul(invMat[j][k], factor);
                }
            }
        }
        galois::gf factor = originMat[i][i];
        for (j = 0; j < DataShards; j++) {
            originMat[i][j] = galois::div(originMat[i][j], factor);
            invMat[i][j] = galois::div(invMat[i][j], factor);
        }
    }
    for (int i = DataShards - 1; i >= 1; i--) {
        for (j = i - 1; j >= 0; j--) {
            if (validShards[j]) {
                continue;
            }
            galois::gf factor = originMat[j][i];
            for (int k = 0; k < DataShards; k++) {
                originMat[j][k] ^= galois::mul(factor, originMat[i][k]);
                invMat[j][k] ^= galois::mul(factor, invMat[i][k]);
            }
        }
    }
    int curFec = DataShards;
    for (int i = 0; i < DataShards; i++) {
        if (validShards[i]) {
            continue;
        }
        memset(shards[curFec], 0, sizeof(galois::gf) * ShardLength);
        for (j = 0; j < ShardLength; j++) {
            for (int k = 0; k < DataShards; k++) {
                shards[curFec][j] ^= galois::mul(invMat[i][k], shards[k][j]);
            }
        }
        ++curFec;
    }
    curFec = DataShards;
    for (int i = 0; i < DataShards; i++) {
        if (validShards[i]) {
            continue;
        }
        memcpy(shards[i], shards[curFec], sizeof(galois::gf) * ShardLength);
        ++curFec;
    }
#ifdef debug
    for (int i = 0; i < DataShards; i ++) {
        for (j = 0; j < DataShards; j ++) {
            printf("%d ", (int)invMat[i][j]);
        }
        printf("\n");
    }
    for (int i = 0; i < DataShards; i ++) {
        for (j = 0; j < DataShards; j ++) {
            printf("%d ", (int)originMatCopy[i][j]);
        }
        printf("\n");
    }
    galois::gf testMat[DataShards][DataShards];
    memset(testMat, 0, sizeof(testMat));
    for (int i = 0; i < DataShards; i ++) {
        for (j = 0; j < DataShards; j ++) {
            for (int k = 0; k < DataShards; k++ ) {
                testMat[i][j] ^= galois::mul(originMatCopy[i][k], invMat[k][j]);
            }
        }
    }
    for (int i = 0; i < DataShards; i ++) {
        for (j = 0; j < DataShards; j ++) {
            printf("%d ", (int)testMat[i][j]);
        }
        printf("\n");
    }
#endif
}

