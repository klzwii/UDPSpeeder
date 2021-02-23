#include "RSHelper.h"
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
#include <cstring>
#include <mm_malloc.h>
#include "SimpleWorkQueue/SimpleWorker.h"
#include "SimpleWorkQueue/SimpleWorkQueue.h"
#include "PackageProcessor/PackageProcessWorker.h"
#include "PackageProcessor/PackageProcessTask.h"
#include<sys/types.h>
#include<sys/stat.h>
#include <unistd.h>
#include<fcntl.h>
#include <cmath>
#include <iostream>
#include<cstdio>
#include<cstdlib>
#include <fcntl.h>
#include <cstdlib>
#include<cstring>
#include<cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<netinet/in.h>

void test();
std::mt19937 g1(time(nullptr));
bool isStart = false;
const int threadPoolSize = 4;
const int taskPoolSize = 8;
int main() {
//    auto worker = reinterpret_cast<SimpleWorker**>(malloc(threadPoolSize * sizeof(SimpleWorker*)));
//    auto threadSafeQueue = new SimpleWorkQueue::SimpleThreadsSafeQueue();
//    for (int i = 0; i < threadPoolSize; i ++) {
//        worker[i] = new PackageProcessWorker();
//    }
//    auto tasks = reinterpret_cast<SimpleTask**>(malloc(taskPoolSize * sizeof(SimpleTask*)));
//    for (int i = 0; i < taskPoolSize; i ++) {
//        auto *message = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
//        auto *messageCopy = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
//        tasks[i] = new PackageProcessTask(message, messageCopy, 3000, 1000);
//    }
    int    listenFD, connFD = 0;
    struct sockaddr_in serverADDR{};
    char    buff[4096];
    int     n;
    if( (listenFD = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        perror("open socket");
        exit(0);
    }
    memset(&serverADDR, 0, sizeof(serverADDR));
    serverADDR.sin_family = AF_INET;
    serverADDR.sin_addr.s_addr = htonl(INADDR_ANY);
    serverADDR.sin_port = htons(6666);
    if(bind(listenFD, (struct sockaddr*)&serverADDR, sizeof(serverADDR)) == -1){
        perror("bind socket");
        exit(0);
    }
    if (listen(listenFD, 10) == -1) {
        perror("listen");
        exit(0);
    }
    struct timeval tv{3, 0};
    auto buffer = reinterpret_cast<unsigned char*>(malloc(sizeof(unsigned char) * 2000));
    fd_set FD;
    FD_ZERO(&FD);
    FD_SET(listenFD, &FD);
    int maxFD = listenFD + 1;
    while (true) {
        auto selectFD = FD;
        auto selectTV = tv;
        bool err = false;
        switch (select(maxFD, &selectFD, nullptr, nullptr, &selectTV)) {
            case -1:
                err = true;
                perror("select");
                break;
            case 0:
                //todo select timeout send package
                std::cout << "select time out" << std::endl;
                break;
            default:
                for (int i = 0; i < maxFD; i ++) {
                    if (!FD_ISSET(i, &selectFD)) {
                        continue;
                    }
                    if (i == listenFD) {
                        struct sockaddr_in clientADDR{};
                        memset(&clientADDR, 0, sizeof(clientADDR));
                        socklen_t sock_size = sizeof(struct sockaddr_in);
                        connFD = accept(listenFD,(struct sockaddr *)&clientADDR, &sock_size);
                        if (connFD < 0) {
                            perror("accept connect");
                            continue;
                        }
                        FD_SET(connFD, &FD);
                        maxFD = std::max(maxFD, connFD + 1);
                        printf("有新客户端连接 %s:%d\n",inet_ntoa(clientADDR.sin_addr),ntohs(clientADDR.sin_port));
                    } else {
                        int nread;
                        ioctl(i, FIONREAD, &nread);//取得数据量交给nread
                        /*客户数据请求完毕，关闭套接字，从集合中清除相应描述符 */
                        if (nread == 0) {
                            close(i);
                            FD_CLR(i, &FD); //去掉关闭的fd
                            printf("removing client on fd %d\n", i);
                        } else {
                            int recvSize;
                            while ((recvSize = recv(i, buff, 10, 0)) == 10) {
                                printf("从客户端读取%d字节数据\n", recvSize);
                            }
                            printf("从客户端读取%d字节数据\n", recvSize);
                        }
                    }
                }
        }
        if (err) {
            break;
        }
    }
//    int cc = 4;
//    while (cc--)
//    std::thread(test).detach();
//    std::this_thread::sleep_for(std::chrono::seconds(1000));
}
void test() {
    auto k = new RSHelper();
    auto *message = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
    auto *messageCopy = reinterpret_cast<unsigned char*>(std::malloc(7000 * sizeof(unsigned char)));
    int lun = 0;
    while (true) {
        int messageLength = 3000, rsCodeLength = 1000;
        //messageLength = gwSize - 1, rsCodeLength = 0;
        if (lun == 10) {
            lun = 0;
            std::cout << "time" << std::endl;
        }
        lun ++;
        memset(message, 0, sizeof(message));
        for (int i = rsCodeLength; i < messageLength + rsCodeLength; i++) {
            RSHelper::setPos(i, message, g1() % gwSize);
        }
        k->attachRSCode(message, messageLength, rsCodeLength);
        std::memcpy(messageCopy, message, 7000 * sizeof(unsigned char));
        int c = rsCodeLength >> 1;
        std::cout << c << std::endl;
        while (c) {
            int pos =g1() % gwSize;
            int changed = g1() % gwSize;
            RSHelper::setPos(pos, message, changed);
            c--;
        }
        auto startTime = std::clock();
        if (!k->getOriginMessage(message, messageLength, rsCodeLength)) {
            std::cout << "wrong";
            std::cout << rsCodeLength << " " << messageLength << std::endl;
            std::cout << std::endl;
            for (int i = messageLength + rsCodeLength - 1; i >= rsCodeLength; i --) {
                std::cout << RSHelper::getPos(i, message) << ",";
            }
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << RSHelper::getPos(i, message)  << ",";
            }
            std::cout << std::endl;
            for (int i = 0; i < messageLength + rsCodeLength; i++) {
                std::cout << RSHelper::getPos(i, messageCopy) << ",";
            }
            std::cout << std::endl;
            break;
        }
        auto endTime = std::clock();
        std::cout << "The run time is: " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    }
    free(k);
}
