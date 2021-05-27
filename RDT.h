//
// Created by liuze on 2021/4/1.
//

#ifndef UDPCOMMON_RDT_H
#define UDPCOMMON_RDT_H
#include <cstdint>
#include <atomic>
#include <malloc.h>
#include <cstring>
#include "HeaderConst.h"
#include <thread>
#include "RS.h"
#include <random>
#include <netinet/in.h>
#include <unistd.h>

class RDT {
private:
    uint16_t RecvEnd;
    uint16_t RecvStart;
    uint32_t *ack;
    uint8_t *recvPackets;
    bool *finish;
    RS *rs;
#ifdef server
    static int serverFD; // for server only
    sockaddr *sendSockAddr;
    socklen_t *sockLen;
#endif
#ifdef debug
    bool *printed;
#endif
    uint8_t **RecvBuffers;
    uint16_t SendWindowEnd;
    uint8_t *sendBuffer;
    uint8_t rawBuffer[2000]{};
    uint16_t rawOffset = 0;

    void rdtEncode(uint8_t **encodeShards, uint16_t length);

    bool rdtDecode(uint8_t **decodeShards, const bool *validShards, uint16_t length);

#ifdef client
    int sendFD;
    int hijackFD;
    std::atomic_bool threadExit{false};
    static void RecvThread(RDT *rdt);
#endif
    uint8_t *buffer;
    timeval bufferStartTime{0, 0};
    size_t offset = 0;
    uint WINDOW_SIZE;
    uint BATCH_LENGTH;
    uint PACKET_SIZE;
    uint RS_LENGTH;
    uint SEGMENT_LENGTH;
    uint DATA_LENGTH;
    uint32_t BUFFER_THRESHOLD;
    in_addr_t fakeIP;
    int rawSocket;
    uint16_t *PacketLength;

    static void reCalcChecksum(uint16_t *payLoad, size_t len);

public:
    uint16_t uuid = 0;
#ifdef server
    volatile uint16_t downSpeed = 0;
    volatile uint16_t upSpeed = 0;

    static void init(int fd);

#endif

    ~RDT() {
        free(sendBuffer);
        for (int i = 0; i < WINDOW_SIZE; i++) {
            free(RecvBuffers[i]);
        }
        free(RecvBuffers);
        free(ack);
        free(finish);
        free(buffer);
        free(PacketLength);
#ifdef client
        threadExit.store(false);
        while (threadExit.load()) {
            std::this_thread::yield();
        }
        close(sendFD);
#endif
        close(rawSocket);
    }

#ifdef client
    RDT(uint WINDOW_SIZE, uint BATCH_LENGTH, uint PACKET_SIZE, uint RS_LENGTH, in_addr_t SendAddr, in_port_t SendPort,
        uint32_t RECOVER_THRESHOLD, uint32_t RESEND_THRESHOLD, uint32_t BUFFER_THRESHOLD, in_addr_t fakeIP, int hijackFD)
#endif
#ifdef server

    RDT(uint WINDOW_SIZE, uint BATCH_LENGTH, uint PACKET_SIZE, uint RS_LENGTH, sockaddr *sockADDR, socklen_t *sockLen,
        uint32_t BUFFER_THRESHOLD, in_addr_t fakeIP, uint16_t clientSeq, uint16_t serverSeq, uint16_t uuid)
#endif
    {
        if ((WINDOW_SIZE & -WINDOW_SIZE) != WINDOW_SIZE) {
            printf("WINDOW_SIZE should be integer power of 2\n");
            exit(-1);
        }
#ifdef server
        if (serverFD <= 0) {
            printf("should first initialize server socket fd\n");
            exit(-1);
        }
        sendSockAddr = sockADDR;
        this->sockLen = sockLen;
#endif
        SendWindowEnd = serverSeq;
        RecvStart = clientSeq;
        RecvEnd = clientSeq;
        this->WINDOW_SIZE = WINDOW_SIZE;
        this->BATCH_LENGTH = BATCH_LENGTH;
        this->PACKET_SIZE = PACKET_SIZE;
        this->BUFFER_THRESHOLD = BUFFER_THRESHOLD;
        DATA_LENGTH = PACKET_SIZE * BATCH_LENGTH;
        SEGMENT_LENGTH = PACKET_SIZE + HEADER_LENGTH;
        this->RS_LENGTH = RS_LENGTH;
        uint bufferSize = SEGMENT_LENGTH * (RS_LENGTH + BATCH_LENGTH);
        sendBuffer = reinterpret_cast<uint8_t *>(calloc(bufferSize, 1));
        RecvBuffers = reinterpret_cast<uint8_t **>(calloc(sizeof(void *), WINDOW_SIZE));
        for (int i = 0; i < WINDOW_SIZE; i++) {
            RecvBuffers[i] = reinterpret_cast<uint8_t *>(calloc(bufferSize, 1));
        }
        ack = reinterpret_cast<uint32_t *>(calloc(sizeof(uint32_t), WINDOW_SIZE));
        recvPackets = reinterpret_cast<uint8_t *>(calloc(sizeof(uint8_t), WINDOW_SIZE));
        SEGMENT_LENGTH = PACKET_SIZE + HEADER_LENGTH;
        rs = new RS(BATCH_LENGTH, RS_LENGTH);
        finish = reinterpret_cast<bool *>(calloc(WINDOW_SIZE, sizeof(bool)));
        buffer = reinterpret_cast<uint8_t *>(calloc(DATA_LENGTH, sizeof(uint8_t)));
        PacketLength = reinterpret_cast<uint16_t *>(calloc(WINDOW_SIZE, sizeof(uint16_t)));
        rawSocket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
        this->uuid = uuid;
        int val = 1;
        if (setsockopt(rawSocket, IPPROTO_IP, IP_HDRINCL, reinterpret_cast<const void *>(&val), sizeof(int))) {
            perror("setsockopt() error");
            exit(-1);
        }
#ifdef debug
        printed = reinterpret_cast<bool*>(calloc(sizeof(bool), WINDOW_SIZE));
#endif
        if (rawSocket < 0) {
            perror("open raw socket");
        }
        this->fakeIP = fakeIP;
#ifdef client
        this->hijackFD = hijackFD;
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 rand_num(seed);
        std::uniform_int_distribution<uint16_t> dist;
        uuid = dist(rand_num);
        struct sockaddr_in sockAddr{};
        sockAddr.sin_addr.s_addr = SendAddr;
        sockAddr.sin_family = AF_INET;
        sockAddr.sin_port = SendPort;
        sendFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sendFD < 0) {
            perror("open socket");
            exit(-1);
        }
        if (connect(sendFD, (sockaddr*)&sockAddr, sizeof(sockAddr)) < 0) {
            perror("connect");
            exit(-1);
        }
        std::thread(RecvThread, this).detach();
#endif
    };

    bool AddData(uint8_t *buffer, size_t length);

    void RecvBuffer(uint8_t *data);

    void BufferTimeOut();

    void SendBuffer();

    void SendRawBuffer();

    static uint16_t calcCheckSum(uint16_t *data, size_t len, const uint16_t *fakeHead);

};

#endif
