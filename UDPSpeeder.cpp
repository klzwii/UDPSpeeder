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
    timeval tv{0, 10000};
    FD_ZERO(&FDSet);
    FD_SET(listenFD, &FDSet);
    uint8_t finishCount = 0;
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
            }
        } else if(FD_ISSET(listenFD, &fdSetCopy)) {
            length = recvfrom(listenFD, buffer, 10000, 0, reinterpret_cast<sockaddr *>(&clientADDR), &clientADDRLen);
            if (length < 0) {
                perror("recv");
                continue;
            }
            sendHead.SetAckSeq(head.SendSeq());
            if (!head.IsFin() && crc32c::Crc32c(buffer + 4, length - 4) != head.CRC()) {
                continue;
            }
            uint32_t sendSeq = head.SendSeq();
            if (head.IsFin() || finished) {
                if (!finished && !ackSeq.compare_exchange_strong(sendSeq, sendSeq + 1)) {
                    continue;
                }
                std::cout << "finished" << head.SendSeq() << std::endl;
                sendHead.SetFIN(true);
                finishCount = 0;
                finished = true;
            } else {
                setData(sendSeq, head.SubSeq(), head.RealSeq(), buffer, head.PacketLength());
            }
            sendHead.SetAckSeq(ackSeq.load());
            sendto(listenFD, sendBuffer, HEADER_LENGTH, 0,
                   reinterpret_cast<const sockaddr *>(&clientADDR), clientADDRLen);
        }
    }
    close();
}