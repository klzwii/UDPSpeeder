//
// Created by liuze on 2021/4/1.
//

#include "RDT.h"
#include <sys/time.h>
#include <crc32c/crc32c.h>
#include <thread>
#include <algorithm>
#include <linux/ip.h>

#ifdef server
int RDT::serverFD = 0;
#endif

uint16_t RDT::calcCheckSum(uint16_t *data, size_t len, const uint16_t *fakeHead) {
    uint32_t cksum = 0;
    if (fakeHead != nullptr) {
        for (int i = 0; i < 6; i ++) {
            cksum += *(fakeHead + i);
        }
    }
    while(len > 1)
    {
        cksum += *(data++);
        len -= 2;
    }
    if (len == 1) {
        cksum += *(uint8_t*)data;
    }
    while (cksum >> 16) {
        uint16_t t = cksum >> 16;
        cksum &= 0x0000ffff;
        cksum += t;
    }
    return cksum;
}

void RDT::reCalcChecksum(uint16_t *payLoad, size_t len) {
    auto *iph = (iphdr*)payLoad;
    uint16_t headLen = iph->ihl * 4;
    uint8_t fakeHead[12];
    if (iph->protocol != IPPROTO_ICMP) {
        *(uint32_t*)fakeHead = iph->saddr;
        *(uint32_t*)(fakeHead + 4) = iph->daddr;
        *(uint8_t*)(fakeHead + 8) = 0;
        *(uint8_t*)(fakeHead + 9) = iph->protocol;
        *(uint16_t*)(fakeHead + 10) = htobe16((uint16_t)(len - headLen));
        if (iph->protocol == IPPROTO_TCP) {
            *(payLoad + headLen/2 + 8) = 0;
            *(payLoad + headLen/2 + 8) = ~calcCheckSum(payLoad + headLen/2, len - headLen, (uint16_t*)fakeHead);
        }
        if (iph->protocol == IPPROTO_UDP) {
            *(payLoad + headLen/2 + 3) = 0;
            *(payLoad + headLen/2 + 3) = ~calcCheckSum(payLoad + headLen/2, len - headLen, (uint16_t*)fakeHead);
        }
    }
    iph->check = 0;
    iph->check = ~(calcCheckSum(payLoad, headLen, nullptr));
}

// non thread safe
void RDT::SendBuffer() {
    uint8_t *p[20];
    ++SendWindowEnd;
    uint8_t *&currentBuffer = sendBuffer;
    memset(currentBuffer, 0, SEGMENT_LENGTH * (BATCH_LENGTH + RS_LENGTH));
    for (auto i = 0; i < BATCH_LENGTH; i++) {
        memcpy(currentBuffer + i * SEGMENT_LENGTH + HEADER_LENGTH, buffer + i * PACKET_SIZE, PACKET_SIZE);
        p[i] = currentBuffer + i * SEGMENT_LENGTH + HEADER_LENGTH;
    }
    for (auto i = BATCH_LENGTH; i < BATCH_LENGTH + RS_LENGTH; i++) {
        p[i] = currentBuffer + i * SEGMENT_LENGTH + HEADER_LENGTH;
    }
    if (RS_LENGTH != 0) {
        rdtEncode(p, offset);
    }
    for (int i = 0; i < RS_LENGTH + BATCH_LENGTH; i++) {
        auto head = header(currentBuffer + i * SEGMENT_LENGTH);
        head.SetPacketLength(offset);
        head.SetSendSeq(SendWindowEnd);
        head.SetSubSeq(i);
        head.SetCRC(crc32c::Crc32c(currentBuffer + i * SEGMENT_LENGTH + 4, SEGMENT_LENGTH - 4));
    }
    for (int i = 0; i < BATCH_LENGTH + RS_LENGTH; i++) {
        auto head = header(currentBuffer + i * SEGMENT_LENGTH);
#ifdef lossRate
        if (rand() % 10000 < lossRate * 10000) {
            continue;
        }
#endif
        size_t sendRet = 0;
#ifdef client
        sendRet = send(sendFD, currentBuffer + i * SEGMENT_LENGTH, SEGMENT_LENGTH, 0);
#endif
#ifdef server
        sendRet = sendto(serverFD, currentBuffer + i * SEGMENT_LENGTH, SEGMENT_LENGTH, 0, sendSockAddr, *sockLen);
#endif
        if (sendRet < 0) {
            perror("send");
        }
    }
}

//non thread-safe
void RDT::RecvBuffer(uint8_t *data) {
    auto head = header(data);
    if (head.CRC() != crc32c::Crc32c(data + 4, SEGMENT_LENGTH - 4)) {
        printf("wrong crc \n");
        return;
    }
    uint16_t diff;
#ifdef debug
    printf("recv buffer %d subseq %d cur window start %d\n", head.SendSeq(), head.SubSeq(), RecvStart);
#endif
    uint16_t debugHead = head.SendSeq();
    diff = head.SendSeq() - RecvStart;
    if (diff > WINDOW_SIZE || diff == 0) {
        diff = head.SendSeq() - RecvEnd;
        if (diff > WINDOW_SIZE) {
            return;
        }
        uint16_t calcStart = head.SendSeq() - WINDOW_SIZE;
        for (uint16_t i = RecvStart;; i++) {
            uint16_t curPos = i % WINDOW_SIZE;
            finish[curPos] = false;
            RecvStart = i;
            ack[curPos] = 0;
            recvPackets[curPos] = 0;
            bzero(RecvBuffers[curPos], SEGMENT_LENGTH * (RS_LENGTH + BATCH_LENGTH));
            if (i == calcStart) {
                break;
            }
        }
        RecvStart = calcStart;
        for (uint16_t i = calcStart + 1; i != RecvEnd; i++) {
            uint16_t curPos = i % WINDOW_SIZE;
            if (finish[curPos]) {
                finish[curPos] = false;
                bzero(RecvBuffers[curPos], SEGMENT_LENGTH * (RS_LENGTH + BATCH_LENGTH));
#ifdef debug
                printed[curPos] = false;
#endif
                RecvStart = i;
                ack[curPos] = 0;
                recvPackets[curPos] = 0;
            } else {
                break;
            }
        }

    }
    diff = head.SendSeq() - RecvEnd;
    if (diff <= WINDOW_SIZE) {
        RecvEnd = head.SendSeq();
    }
    uint16_t pos = head.SendSeq() % WINDOW_SIZE;
    if (ack[pos] & (1 << head.SubSeq())) {
        return;
    }
    if (!ack[pos]) {
        PacketLength[pos] = head.PacketLength();
    }
    recvPackets[pos]++;
    ack[pos] |= (1 << head.SubSeq());
    uint8_t *&currentBuffer = RecvBuffers[pos];
    memcpy(currentBuffer + head.SubSeq() * PACKET_SIZE, data + HEADER_LENGTH, PACKET_SIZE);
    if (recvPackets[pos] == BATCH_LENGTH) {
        uint32_t ackBits = ack[pos];
        bool canDump = true;
        if (ackBits != ((1 << BATCH_LENGTH) - 1)) {
            int nowPackets = 0;
            bool validShards[RS_LENGTH + BATCH_LENGTH];
            bzero(validShards, RS_LENGTH + BATCH_LENGTH);
            int validPos = 0;
            while (ackBits) {
                if (1 & ackBits) {
                    ++nowPackets;
                    validShards[validPos] = true;
                }
                ++validPos;
                ackBits >>= 1;
            }
            uint8_t *p[RS_LENGTH + BATCH_LENGTH];
            for (int i = 0; i < BATCH_LENGTH + RS_LENGTH; i++) {
                p[i] = currentBuffer + PACKET_SIZE * i;
            }
            canDump = rdtDecode(p, validShards, PacketLength[pos]);
        }
        if (canDump) {
            uint16_t packetLength = PacketLength[pos];
            if (packetLength != 0) {
                uint16_t recvOffset = 4;
                uint16_t copyLength;
                while (recvOffset != packetLength) {
                    uint16_t dataLeft = packetLength - recvOffset;
                    if (dataLeft >= sizeof(struct iphdr)) {
                        auto *hdr = reinterpret_cast<struct iphdr *>(RecvBuffers[pos] + recvOffset);
                        copyLength = be16toh(hdr->tot_len);
                    }
                    copyLength = std::min((uint16_t) (packetLength - recvOffset), copyLength);
                    memcpy(rawBuffer + rawOffset, RecvBuffers[pos] + recvOffset, copyLength);
                    rawOffset += copyLength;
                    recvOffset += copyLength;
                    SendRawBuffer();
                }
            }
            finish[pos] = true;
#ifdef debug
            printf("recv seq %d\n", head.SendSeq());
#endif
        }
    }
}

#ifdef client
void RDT::RecvThread(RDT* rdt) {
    fd_set fdSet{};
    timeval timeOut{0, 1000};
    FD_ZERO(&fdSet);
    auto sendFD = rdt->sendFD;
    FD_SET(sendFD, &fdSet);
    uint8_t buffer[rdt->SEGMENT_LENGTH];
    while (true) {
        auto setCopy = fdSet;
        auto tvCopy = timeOut;
        if (rdt->threadExit) {
            rdt->threadExit.store(false);
            return;
        }
        int selectedFd = select(sendFD + 1, &setCopy, nullptr, nullptr, &tvCopy);
        if (FD_ISSET(sendFD, &setCopy)) {
            auto recvRet = recvfrom(sendFD, buffer, rdt->SEGMENT_LENGTH, 0, nullptr, nullptr);
            if (recvRet < 0) {
                perror("receive from socket");
            }
            rdt->RecvBuffer(buffer);
        } else if (selectedFd != 0) {
            perror("select");
        }
    }
}
#endif

void RDT::SendRawBuffer() {
    if (rawOffset < sizeof(struct iphdr)) {
        return;
    }
    auto *iphdr = reinterpret_cast<struct iphdr*>(rawBuffer);
    uint16_t sendLength = be16toh(iphdr->tot_len);
    if (sendLength != rawOffset) {
        return;
    }
    sockaddr_in tempSock{};
#ifdef client
    iphdr->daddr = fakeIP;
#endif
#ifdef server
    iphdr->saddr = fakeIP;
#endif
    tempSock.sin_family = AF_INET;
    tempSock.sin_addr.s_addr = iphdr->daddr;
    if (iphdr->protocol == IPPROTO_TCP || iphdr->protocol == IPPROTO_UDP) {
        tempSock.sin_port = *(uint16_t *) (rawBuffer + 4 * (iphdr->ihl) + 2);
    }
    reCalcChecksum((uint16_t *) rawBuffer, sendLength);
#ifdef server
    auto sendRet = sendto(rawSocket, rawBuffer, sendLength, 0, (sockaddr *) &tempSock, sizeof(sockaddr_in));
#endif
#ifdef client
    auto sendRet = write(hijackFD, rawBuffer, sendLength);
#endif
    if (sendRet < 0) {
        perror("send raw");
    }
    rawOffset = 0;
}

bool RDT::AddData(uint8_t *data, size_t length) {
    bool ret = false;
    if (offset == 0) {
        offset = 4;
        ret = true;
    }
    if (DATA_LENGTH - offset < length) {
        SendBuffer();
        offset = 0;
        AddData(data, length);
        return true;
    }
    memcpy(this->buffer + offset, data, length);
    offset += length;
    return ret;
}


// non thread-safe should be called in send thread;
void RDT::BufferTimeOut() {
    if (!offset) {
        return;
    }
    SendBuffer();
    offset = 0;
}

bool RDT::rdtDecode(uint8_t *decodeShards[], const bool *validShards, uint16_t length) {
    uint8_t *ShardsPointerCopy[20];
    memcpy(ShardsPointerCopy, decodeShards, sizeof(ShardsPointerCopy));
    rs->decode(ShardsPointerCopy, PACKET_SIZE, validShards);
    uint32_t checkSum = *(uint32_t *) decodeShards[0];
    uint32_t calcCheckSum = 0;
    *(uint32_t *) decodeShards[0] = 0;
    for (int i = 0; i < BATCH_LENGTH; i++) {
        calcCheckSum ^= crc32c::Crc32c(decodeShards[i], PACKET_SIZE);
    }
    if (calcCheckSum != checkSum) {
        printf("fatal error ");
        for (int i = 0; i < BATCH_LENGTH + RS_LENGTH; i++) {
            printf("%d ", validShards[i]);
        }
        printf(" %d %d\n", checkSum, calcCheckSum);
    }
    return calcCheckSum == checkSum;
}

void RDT::rdtEncode(uint8_t **encodeShards, uint16_t length) {
    *(uint32_t *) encodeShards[0] = 0;
    uint32_t checkSum = 0;
    for (int i = 0; i < BATCH_LENGTH; i++) {
        checkSum ^= crc32c::Crc32c(encodeShards[i], PACKET_SIZE);
    }
    *(uint32_t *) encodeShards[0] = checkSum;
    rs->encode(encodeShards, PACKET_SIZE);
}

#ifdef server

void RDT::init(int fd) {
    serverFD = fd;
}

#endif

