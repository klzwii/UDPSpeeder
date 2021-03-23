#include "RSHelper.h"
#include <ctime>
#include <iostream>
#include <random>
#include <cstring>
#include <unistd.h>
#include<cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include "crc32c/crc32c.h"
#include "HeaderConst.h"
#include "ReliableDataTransfer.h"

int main() {
    int listenFD;
    struct sockaddr_in serverADDR{}, clientADDR{};
    socklen_t clientADDRLen = sizeof(clientADDR);
    if ((listenFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("open socket");
        exit(0);
    }
    uint8_t buffer[2000];
    memset(&serverADDR, 0, sizeof(serverADDR));
    serverADDR.sin_family = AF_INET;
    serverADDR.sin_addr.s_addr = htonl(INADDR_ANY);
    serverADDR.sin_port = htons(1234);
    ssize_t length;
    if (bind(listenFD, reinterpret_cast<sockaddr *>(&serverADDR), sizeof(serverADDR)) < 0) {
        perror("bind socket error");
        exit(0);
    }
    negotiateConfig();
    int success = 0, fail = 0;
    std::cout << "server started" << std::endl;
    uint8_t sendBuffer[HEADER_LENGTH];
    auto sendHead = header(sendBuffer);
    auto head = header(buffer);
    bool finished = false;
    fd_set FDSet;
    timeval tv{0, 5000};
    FD_ZERO(&FDSet);
    FD_SET(listenFD, &FDSet);
    uint8_t finishCount = 0;
    int process = 0;
    while (true) {
        auto fdSetCopy = FDSet;
        auto tvCopy = tv;
        auto selectFd = select(listenFD + 1, &fdSetCopy, nullptr, nullptr, &tvCopy);
        if (selectFd < 0) {
            perror("socket");
            exit(0);
        } else if (selectFd == 0) {
            if (finished) {
                std::cout << "finished" << std::endl;
                ++finishCount;
                if (finishCount == 100) {
                    break;
                }
                continue;
            } else {
                if (DumpData()) {
                    sendHead.Clear();
                    sendHead.SetAckSeq(recvStart.load());
                    sendHead.SetACK();
                    sendto(listenFD, sendBuffer, HEADER_LENGTH, 0,
                           reinterpret_cast<const sockaddr *>(&clientADDR), clientADDRLen);
                }
            }
        } else if(FD_ISSET(listenFD, &fdSetCopy)) {
            sendHead.Clear();
            length = recvfrom(listenFD, buffer, 10000, 0, reinterpret_cast<sockaddr *>(&clientADDR), &clientADDRLen);
            if (length < 0) {
                perror("recv");
                continue;
            }
            if (!head.IsFin() && crc32c::Crc32c(buffer + 4, length - 4) != head.CRC()) {
                std::cout << "crc check failure" << std::endl;
                continue;
            }
            uint16_t sendSeq = head.SendSeq();
            if (head.IsFin() || finished) {
                std::cout << sendSeq << std::endl;
                uint16_t needAck = sendSeq - 1;
                if (!finished && !recvStart.compare_exchange_strong(needAck, needAck + 1)) {
                    continue;
                }
                std::cout << "finished" << head.SendSeq() << std::endl;
                sendHead.SetFIN();
                finishCount = 0;
                finished = true;
            } else {
                setData(sendSeq, head.SubSeq(), head.RealSeq(), buffer, head.PacketLength());
                if (process == 10) {
                    DumpData();
                }
                ++process;
            }
            sendHead.SetAckSeq(recvStart.load());
            sendHead.SetACK();
            sendto(listenFD, sendBuffer, HEADER_LENGTH, 0,
                   reinterpret_cast<const sockaddr *>(&clientADDR), clientADDRLen);
        }
        //std::cout << "try dump" << std::endl;
    }
    close();
}