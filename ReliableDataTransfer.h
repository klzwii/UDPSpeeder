//
// Created by liuze on 2021/3/15.
//

#ifndef UDPWINCLIENT_RELIABLEDATATRANSFER_H
#define UDPWINCLIENT_RELIABLEDATATRANSFER_H
#include <atomic>
#include <mutex>
#include <algorithm>
#include <thread>
#include <sys/time.h>
#define WINDOW_SIZE 128
#define BATCH_LENGTH 5
#define PACKET_SIZE 1000
#define RS_LENGTH 4
#define TRY_RECOVER_THRESHOLD 2
#define THREAD_NUM 8
#define RESEND_THRESHOLD 700
#define PACKET_LOSS 10

static std::atomic_uint16_t recvEnd;
static std::atomic_uint16_t recvStart;
static std::atomic_uint16_t SendWindowEnd;
static std::atomic_bool finished;
static std::atomic_uint16_t SendWindowStart;
static uint8_t **buffers;
static uint8_t **sendBuffers;
static std::once_flag t;
static std::atomic_uint16_t ack[WINDOW_SIZE];
static std::atomic_uint8_t realLength[WINDOW_SIZE];
static std::atomic_uint16_t packetLengths[WINDOW_SIZE];
static bool finish[WINDOW_SIZE];
static std::atomic_uint16_t wg;
static timeval recvTime[WINDOW_SIZE];
static RSHelper* helpers[WINDOW_SIZE];
static int UDPSock;
static std::atomic_bool closed;
static struct sockaddr_in sSendAddr{};
static timeval sendTime[WINDOW_SIZE];
FILE *file;

void init() {
    for (int i = 0; i < WINDOW_SIZE; i ++) {
        helpers[i] = new RSHelper();
    }
    memset(recvTime, 0, sizeof(recvTime));
    buffers = reinterpret_cast<uint8_t**>(malloc(sizeof(uint8_t **) * WINDOW_SIZE));
    for (int i = 0; i < WINDOW_SIZE; i ++) {
        buffers[i] = reinterpret_cast<uint8_t*>(malloc(10000));
        ack[i].store(0);
    }
    sendBuffers = reinterpret_cast<uint8_t**>(malloc(sizeof(uint8_t **) * WINDOW_SIZE));
    for (int i = 0; i < WINDOW_SIZE; i ++) {
        sendBuffers[i] = reinterpret_cast<uint8_t*>(malloc(10000));
        ack[i].store(0);
    }
}

void negotiateConfig() {
    recvEnd.store(-1);
    recvStart.store(-1);
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

void setData(uint16_t seq, uint8_t subSeq, uint8_t realSeq, uint8_t* buffer, uint16_t packetLength) {
    uint16_t curSec = recvStart.load();
    if ((seq - curSec) > WINDOW_SIZE) {
        return;
    }
    const auto pos = seq % WINDOW_SIZE;
    if (finish[pos] || (1<<subSeq) & ack[pos].load()) {
        return;
    }
    memcpy(buffers[pos] + subSeq *  PACKET_SIZE, buffer + HEADER_LENGTH, PACKET_SIZE);
    auto befAck = ack[pos].load();
    if (befAck == 0) {
        realLength[pos].store(realSeq);
        packetLengths[pos].store(packetLength);
        gettimeofday(&recvTime[pos], nullptr);
    }
    uint8_t aftAck = befAck | (1<<subSeq);
    while (!ack[pos].compare_exchange_strong(befAck, aftAck)) {
        befAck = ack[pos].load();
        aftAck = befAck | (1<<subSeq);
    }
    static uint16_t lastSeq = recvEnd.load();
    while ((uint16_t)(seq - lastSeq) <= WINDOW_SIZE && !recvEnd.compare_exchange_strong(lastSeq, seq)) {
        lastSeq = recvEnd.load();
    }
    std::cout << "last seq" << recvEnd.load() << std::endl;
}

void DumpDataBySeq(uint16_t curSeq) {
    ++curSeq;
    std::cout << "curSeq: " << curSeq << std::endl;
    uint16_t pos = curSeq % WINDOW_SIZE;
    if (finish[pos]) {
        return;
    }
    uint8_t ackBits = ack[pos].load();
    auto k = helpers[pos];
    if (recvTime[pos].tv_usec == 0 && recvTime[pos].tv_sec == 0) {
        return;
    }
    uint8_t realBits = (1 << realLength[pos]) - 1;
    bool canDump = false;
    if ((realBits & ackBits) == realBits) {
        canDump = true;
    } else {
        timeval curTime{};
        gettimeofday(&curTime, nullptr);
        auto diffTime = (curTime.tv_sec - recvTime[pos].tv_sec) * 1000 + (curTime.tv_usec - recvTime[pos].tv_usec) / 1000;
        if (diffTime > TRY_RECOVER_THRESHOLD) {
            int nowPackets = 0;
            while (ackBits) {
                nowPackets += (1 & ackBits);
                ackBits >>= 1;
            }
            if ((RS_LENGTH >> 1) >= (RS_LENGTH + realLength[pos] - nowPackets)) {
                uint8_t *p[RS_LENGTH + BATCH_LENGTH];
                for (int i = 0; i < realLength[pos]; i ++) {
                    p[i + RS_LENGTH] = buffers[pos] + i * PACKET_SIZE;
                }
                for (int i = 0; i < RS_LENGTH; i ++) {
                    p[i] = buffers[pos] + (realLength[pos] + i) * PACKET_SIZE;
                }
                canDump = k->GetOriginMessageFromPackets(p, realLength[pos], RS_LENGTH, PACKET_SIZE);
                if (!canDump) {
                    printf("recover message fail seq:%d realLength:%d\n arrivePackets:%d ", curSeq, (int)realLength[pos], nowPackets);
                    auto c = ack[pos].load();
                    while (c) {
                        std::cout << (int)(c&1);
                        c >>=1;
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    if (canDump) {
        finish[pos] = true;
    }
}

void DumpDataThread(uint16_t seq) {
    DumpDataBySeq(seq);
    uint16_t curWG = wg.load();
    while (!wg.compare_exchange_strong(curWG, curWG - 1)) {
        curWG = wg.load();
    }
}

bool DumpData() {
    uint16_t start = recvStart.load();
    wg.store(0);
    uint16_t end = recvEnd.load();
    if ((uint16_t) (end - start) > THREAD_NUM) {
        end = start + THREAD_NUM;
    }
    for (uint16_t i = start; (uint16_t)(end - start) > (uint16_t)(i - start) ; ++i) {
        uint16_t curWG = wg.load();
        while (!wg.compare_exchange_strong(curWG, curWG + 1)) {
            curWG = wg.load();
        }
        std::thread(DumpDataThread, i).detach();
    }
    while (wg.load() != 0) {
        std::this_thread::yield();
    }
    bool flg = false;
    for (uint16_t i = recvStart.load() + 1; (uint16_t)(end - start) >= (uint16_t)(i - start); i ++) {
        auto pos = i % WINDOW_SIZE;
        fflush(stdout);
        if (finish[pos]) {
            fwrite(buffers[pos], 1, packetLengths[pos], file);
            recvStart.store(i);
            recvTime[pos] = timeval{0, 0};
            finish[pos] = false;
            ack[pos].store(0);
            flg = true;
        } else {
            break;
        }
    }
    return flg;
}

#endif //UDPWINCLIENT_RELIABLEDATATRANSFER_H
