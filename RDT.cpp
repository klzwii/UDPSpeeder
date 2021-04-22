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


long long operator - (const timeval &a, const timeval &b) {
    long long t = (a.tv_sec - b.tv_sec) * 1000 + (a.tv_usec - b.tv_usec) / 1000;
    if (t < 0) {
        t += 60 * 60 * 24 * 1000;
    }
    return t;
}

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
    uint16_t curSeq = SendWindowEnd;
    while ((uint16_t) (curSeq - SendWindowStart.load()) >= WINDOW_SIZE) {
        std::this_thread::yield();
    }
    uint8_t *p[20];
    ++SendWindowEnd;
    uint8_t *&currentBuffer = sendBuffers[SendWindowEnd % WINDOW_SIZE];
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
        head.SetRealSeq(BATCH_LENGTH);
        head.SetSendSeq(SendWindowEnd);
        head.SetRSSeq(RS_LENGTH);
        head.SetSubSeq(i);
        head.SetACK();
        head.SetHeadStart(firstHead);
        head.SetCRC(crc32c::Crc32c(currentBuffer + i * SEGMENT_LENGTH + 4, SEGMENT_LENGTH - 4));
    }
    sendBufferBySeq(SendWindowEnd);
}

void RDT::sendBufferBySeq(uint16_t seq) {
#ifdef debug
    printf("send buffer %d\n", seq);
#endif
    uint8_t *&currentBuffer = sendBuffers[seq % WINDOW_SIZE];
    for (int i = 0; i < BATCH_LENGTH + RS_LENGTH; i++) {
        auto head = header(currentBuffer + i * SEGMENT_LENGTH);
        if (head.AckSeq() != RecvStart) {
            head.SetAckSeq(RecvStart);
            head.SetCRC(crc32c::Crc32c(currentBuffer + i * SEGMENT_LENGTH + 4, SEGMENT_LENGTH - 4));
        }
#ifdef lossRate
        if (rand() % 10000 < lossRate * 10000) {
            continue;
        }
#endif
        int sendRet = 0;
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
    timeval temp{};
    gettimeofday(&temp, nullptr);
    sendTime[seq % WINDOW_SIZE] = temp;
    lastSendTime = temp;
}

//non thread-safe
void RDT::RecvBuffer(uint8_t *data) {
    auto head = header(data);
    if(head.CRC() != crc32c::Crc32c(data + 4, SEGMENT_LENGTH - 4)) {
        return;
    }
    uint16_t diff = head.SendSeq() - RecvStart;
    if (diff > WINDOW_SIZE || diff == 0) {
        return;
    }
    diff = head.SendSeq() - RecvEnd;
    if (diff <= WINDOW_SIZE) {
        RecvEnd = head.SendSeq();
    }
    uint16_t pos = head.SendSeq() % WINDOW_SIZE;
    if (ack[pos]&(1 << head.SubSeq())) {
        return;
    }
    if (!ack[pos]) {
        PacketLength[pos] = head.PacketLength();
        FirstHeader[pos] = head.HeadStart();
    }
    ack[pos] |= (1 << head.SubSeq());
    gettimeofday(&recvTime[pos], nullptr);
    if (head.IsACK()) {
        diff = head.AckSeq() - SendWindowStart;
        if (diff != 0 && diff <= WINDOW_SIZE) {
            SendWindowStart.store(head.AckSeq());
        }
    }
    uint8_t* &currentBuffer = RecvBuffers[pos];
    memcpy(currentBuffer + head.SubSeq() * PACKET_SIZE, data + HEADER_LENGTH, PACKET_SIZE);
}

#ifdef client
void RDT::RecvThread(RDT* rdt) {
    fd_set fdSet{};
    timeval timeOut{0, 20000};
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
            rdt->DumpData();
            rdt->TimeOut();
        } else if (selectedFd == 0) {
            rdt->TimeOut();
            rdt->DumpData();
        } else {
            perror("select");
        }
    }
}
#endif

void RDT::DumpDataBySeq(uint16_t curSeq, RDT* rdt, int id) {
    uint16_t pos = curSeq % rdt->WINDOW_SIZE;
    if (rdt->finish[pos]) {
        return;
    }
    const int &RS_LENGTH = rdt->RS_LENGTH;
    const int &BATCH_LENGTH = rdt->BATCH_LENGTH;
    const int &PACKET_SIZE = rdt->PACKET_SIZE;
    uint8_t ackBits = rdt->ack[pos];
    if (rdt->recvTime[pos].tv_usec == 0 && rdt->recvTime[pos].tv_sec == 0) {
        return;
    }
    uint8_t realBits = (1 << BATCH_LENGTH) - 1;
    bool canDump = false;
    if ((realBits & ackBits) == realBits) {
        canDump = true;
    } else {
        timeval curTime{};
        gettimeofday(&curTime, nullptr);
        auto diffTime = curTime - rdt->recvTime[pos];
        if (diffTime > rdt->RECOVER_THRESHOLD) {
            int nowPackets = 0;
            bool validShards[RS_LENGTH + BATCH_LENGTH];
            memset(validShards, 0, sizeof(validShards));
            int validPos = 0;
            while (ackBits) {
                if (1 & ackBits) {
                    ++nowPackets;
                    validShards[validPos] = true;
                }
                ++validPos;
                ackBits >>= 1;
            }
#ifdef debug
            printf("try recover valid packets %d\n", nowPackets);
#endif
            if (nowPackets >= BATCH_LENGTH) {
                uint8_t *p[RS_LENGTH + BATCH_LENGTH];
                uint8_t *&currentBuffer = rdt->RecvBuffers[pos];
                for (int i = 0; i < BATCH_LENGTH + RS_LENGTH; i++) {
                    p[i] = currentBuffer + PACKET_SIZE * i;
                }
                canDump = rdt->rdtDecode(p, validShards, rdt->PacketLength[pos]);
#ifdef debug
                if (!canDump) {
                    printf("recover message fail seq:%d realLength:%d\n arrivePackets:%d ", curSeq, BATCH_LENGTH, nowPackets);
                } else {
                    printf("recover succeed\n");
                }
#endif
            }
        }
    }
    if (canDump) {
        rdt->finish[pos] = true;
    }
}

void RDT::DumpDataThread(uint16_t seq, RDT* rdt, int id) {
    DumpDataBySeq(seq, rdt, id);
    int curWG = rdt->wg.load();
    while (!rdt->wg.compare_exchange_strong(curWG, curWG - 1)) {
        curWG =  rdt->wg.load();
    }
}

bool RDT::DumpData() {
    uint16_t start = RecvStart;
    wg.store(0);
    uint16_t end = RecvEnd;
    int id = 0;
    bool flg = false;
    for (uint16_t i = start + 1; end - i <= WINDOW_SIZE; ++i) {
        if (finish[i % WINDOW_SIZE]) {
            continue;
        }
        if (id >= THREAD_NUM) {
            break;
        }
        int curWG = wg.load();
        while (!wg.compare_exchange_strong(curWG, curWG + 1)) {
            curWG = wg.load();
        }
        std::thread(DumpDataThread, i, this, id).detach();
    }
    while (wg.load() != 0) {
        std::this_thread::yield();
    }
    for (uint16_t i = start + 1; end - i <= WINDOW_SIZE; ++i) {
        auto pos = i % WINDOW_SIZE;
        if (finish[pos]) {
            flg = true;
            uint16_t packetLength = PacketLength[pos];
            if (packetLength != 0) {
                uint16_t recvOffset = 4;
                uint16_t copyLength = FirstHeader[pos] - 4;
                memcpy(rawBuffer + rawOffset, RecvBuffers[pos] + recvOffset, copyLength);
                rawOffset += copyLength;
                recvOffset += copyLength;
                SendRawBuffer();
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
            RecvStart = i;
            recvTime[pos] = timeval{0, 0};
            finish[pos] = false;
            ack[pos] = 0;
        } else {
            break;
        }
    }
    return flg;
}

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
    if (iphdr->protocol == IPPROTO_TCP) {
        tempSock.sin_port = *(uint16_t *)(rawBuffer + 4 * (iphdr->ihl) + 2);
    }
    reCalcChecksum((uint16_t*)rawBuffer, sendLength);
    auto sendRet = sendto(rawSocket, rawBuffer, sendLength, 0, (sockaddr*)&tempSock, sizeof(sockaddr_in));
    if (sendRet < 0) {
        perror("send raw");
    }
    rawOffset = 0;
}

void RDT::AddData(uint8_t *data, size_t length, bool initial) {
    if (offset == 0) {
        offset += 4;
        gettimeofday(&bufferStartTime, nullptr);
    }
    if (DATA_LENGTH - offset < length) {
        SendBuffer();
        offset = 0;
        firstHead = DATA_LENGTH;
        AddData(data, length, true);
        return;
    }
    memcpy(this->buffer + offset, data, length);
    if (initial) {
        if (firstHead > offset) {
            firstHead = offset;
        }
    }
    offset += length;
}


// non thread-safe should be called in send thread;
void RDT::BufferTimeOut() {
    if (!offset) {
        return;
    }
    timeval curTime{};
    gettimeofday(&curTime, nullptr);
    if (curTime - bufferStartTime > BUFFER_THRESHOLD) {
        if (firstHead > offset) {
            firstHead = offset;
        }
        SendBuffer();
        offset = 0;
        firstHead = DATA_LENGTH;
    }
}


// must not modify SendWindowStart during this func, should be called in recv thread
void RDT::TimeOut() {
    timeval curTime{};
    gettimeofday(&curTime, nullptr);
    for (uint16_t i = SendWindowStart + 1; SendWindowEnd - i <= WINDOW_SIZE; i++) {
        if (curTime - sendTime[i % WINDOW_SIZE] > RESEND_THRESHOLD) {
            sendBufferBySeq(i);
        } else {
            break;
        }
    }
}

bool RDT::rdtDecode(uint8_t **decodeShards, const bool *validShards, uint16_t length) {
    rs->decode(decodeShards, PACKET_SIZE, validShards);
    uint32_t checkSum = *(uint32_t *) decodeShards[0];
    uint32_t calcCheckSum = 0;
    *(uint32_t *) decodeShards[0] = 0;
    for (int i = 0; i < BATCH_LENGTH; i++) {
        calcCheckSum ^= crc32c::Crc32c(decodeShards[i], PACKET_SIZE);
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

void RDT::HeartBeat() {
    if (SendWindowEnd != SendWindowStart.load()) {
        timeval curTime{};
        gettimeofday(&curTime, nullptr);
        if (curTime - lastSendTime > 50) {
#ifdef debug
            printf("heart beat\n");
#endif
            lastSendTime = curTime;
            auto head = header(heartBuffer);
            head.SetACK();
            head.SetAckSeq(RecvStart);
            head.SetCRC(crc32c::Crc32c(heartBuffer, HEADER_LENGTH - 4));
        }
    }
}

#ifdef server

void RDT::init(int fd) {
    serverFD = fd;
}

#endif

