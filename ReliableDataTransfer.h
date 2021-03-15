//
// Created by liuze on 2021/3/15.
//

#ifndef UDPWINCLIENT_RELIABLEDATATRANSFER_H
#define UDPWINCLIENT_RELIABLEDATATRANSFER_H
#include <atomic>
#include <mutex>
#include <algorithm>
#define windowSize 10
#define PacketSize 1000

static std::atomic_uint32_t currentSeq;
static std::atomic_uint32_t ackSeq;
static uint8_t **buffers;
static std::once_flag t;
static std::atomic_uint8_t ack[windowSize];
static std::atomic_bool finish[windowSize];
FILE *file;

void init() {
    buffers = reinterpret_cast<uint8_t**>(malloc(sizeof(uint8_t **) * windowSize));
    for (int i = 0; i < windowSize; i ++) {
        buffers[i] = reinterpret_cast<uint8_t*>(malloc(10000));
        ack[i].store(0);
    }
}

void negotiateConfig() {
    currentSeq.store(0);
    ackSeq.store(0);
    std::call_once(t, init);
    file = fopen("/root/UDPSpeeder/test/123.pdf", "wb");
    if (ferror(file)) {
        perror("open file");
        exit(0);
    }
    //todo negotiate between server and client
}

void close() {
    fflush(file);
    fclose(file);
}

void setData(uint32_t seq, uint8_t subSeq, uint8_t realSeq, uint8_t* buffer, uint16_t packetLength) {
    auto curSec = ackSeq.load();
    if ((seq - curSec) >= windowSize) {
        return;
    }
    const auto pos = seq % windowSize;
    memcpy(buffers[pos] + subSeq *  PacketSize, buffer + HEADER_LENGTH, PacketSize);
    auto befAck = ack[pos].load();
    uint8_t aftAck = befAck | (1<<subSeq);
    while (!ack[pos].compare_exchange_strong(befAck, aftAck)) {
        befAck = ack[pos].load();
        aftAck = befAck | (1<<subSeq);
        std::cout << aftAck << std::endl;
    }
    std::cout << seq << " " << (int)subSeq << std::endl;
    uint8_t realBits = (1 << realSeq) - 1;
    if ((realBits & aftAck) == realBits) {
        finish[pos].store(true);
        curSec = ackSeq.load();
        if (curSec == seq) {
            for (uint8_t i = 0; i < windowSize; i ++) {
                auto tempPos = (pos + i) % windowSize;
                if (finish[tempPos].load()) {
                    uint32_t befSeq = curSec + i;
                    if (!ackSeq.compare_exchange_strong(befSeq, curSec + i + 1)) {
                        break;
                    }
                    fwrite(buffers[tempPos], 1, packetLength, file);
                    finish[tempPos].store(false);
                    ack[tempPos].store(0);
                } else {
                    break;
                }
            }
        }
    }
}

#endif //UDPWINCLIENT_RELIABLEDATATRANSFER_H
