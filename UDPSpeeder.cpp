#include <ctime>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include "HeaderConst.h"
#include "Connection.h"
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstdio>
#include <fcntl.h>

struct sockaddr_in structsockaddrIn{}, sout{};
socklen_t sockLen;
int rawFD;
std::map<uint16_t, RDT *> rdtMap;
RDT *rdt = nullptr;

int fd;

int tun_alloc(char dev[IFNAMSIZ]) {
    struct ifreq ifr{};
    int fd, err;
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open");
        return -1;
    }
    bzero(&ifr, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (*dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }
    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        perror("ioctl TUNSETIFF");
        close(fd);
        return err;
    }
    strcpy(dev, ifr.ifr_name);
    std::cout << ifr.ifr_name << std::endl;
    return fd;
}

int set_stack_attribute(char *dev, in_addr_t ip) {
    struct ifreq ifr{};
    struct sockaddr_in addr{};
    int sockfd, err = -1;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ip;
    bzero(&ifr, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    bcopy(&addr, &ifr.ifr_addr, sizeof(addr));
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    if ((err = ioctl(sockfd, SIOCSIFADDR, (void *) &ifr)) < 0) {
        perror("ioctl SIOSIFADDR");
        goto done;
    }
    if ((err = ioctl(sockfd, SIOCGIFFLAGS, (void *) &ifr)) < 0) {
        perror("ioctl SIOCGIFADDR");
        goto done;
    }
    ifr.ifr_flags |= IFF_UP;
    if ((err = ioctl(sockfd, SIOCSIFFLAGS, (void *) &ifr)) < 0) {
        perror("ioctl SIOCSIFFLAGS");
        goto done;
    }
    inet_pton(AF_INET, "255.255.255.0", &addr.sin_addr);
    bcopy(&addr, &ifr.ifr_netmask, sizeof(addr));
    if ((err = ioctl(sockfd, SIOCSIFNETMASK, (void *) &ifr)) < 0) {
        perror("ioctl SIOCSIFNETMASK");
        goto done;
    }
    done:
    close(sockfd);
    return err;
}

void readFromFD() {
    uint8_t buffer[2000];
    ssize_t len;
    fd_set readFD;
    FD_ZERO(&readFD);
    FD_SET(fd, &readFD);
    timeval timeOut{0, 1000};
    while (true) {
        auto setCopy = readFD;
        auto tvCopy = timeOut;
        int selectedFD = select(fd + 1, &setCopy, nullptr, nullptr, &tvCopy);
        if (selectedFD == 0) {
            if (rdt == nullptr) {
                continue;
            }
        } else if (FD_ISSET(fd, &setCopy)) {
            sockaddr_in tempAddr{};
            socklen_t tempLen;
            len = recvfrom(fd, buffer, 2000, 0, (sockaddr *) &tempAddr, &tempLen);
            if (len < 0) {
                perror("recv");
                continue;
            }
            auto head = header(buffer);
            auto *conn = Connection::getConn(head.UUID());
            if (head.IsACK() || head.IsSYN()) {
                int sendBack = Connection::startConn(buffer, (sockaddr *) &tempAddr, &tempLen);
                if (sendBack > 0) {
                    auto ret = sendto(fd, buffer, sendBack, 0, (sockaddr *) &tempAddr, tempLen);
                    if (ret < 0) {
                        continue;
                    } else {
                        printf("send back bytes %d\n", sendBack);
                    }
                }
            } else if (conn != nullptr) {
                conn->RecvBuffer(buffer, tempAddr, tempLen);
            } else {
                printf("fatal invalid uuid\n");
                continue;
            }
        } else {
            perror("select");
        }
    }
}

int main() {
    //unlink("/tmp/UDPSpeederPipe");
    auto ret = mkfifo("/tmp/UDPSpeederPipe", 644);
    if (ret < 0) {
        perror("make fifo");
    }
    auto pipeFD = open("/tmp/UDPSpeederPipe", O_WRONLY | O_NONBLOCK);
    if (pipeFD < 0) {
        perror("open pipe");
        exit(-1);
    }
    auto *ipPool = IPPool::NewIPPool("192.168.62.1", 24);
    if (ipPool == nullptr) {
        printf("init ip pool error check input");
        exit(-1);
    }
    galois::initGF();
    rawFD = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (rawFD < 0) {
        perror("raw fd");
        exit(0);
    }
    fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) {
        perror("open socket");
        exit(0);
    }
    structsockaddrIn.sin_port = htons(1234);
    structsockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    structsockaddrIn.sin_family = AF_INET;
    auto c = bind(fd, (sockaddr *) &structsockaddrIn, sizeof(struct sockaddr_in));
    if (c < 0) {
        perror("bind");
        exit(0);
    }
    RDT::init(fd);
    char devName[IFNAMSIZ];
    bzero(devName, IFNAMSIZ);
    int hijackFD = tun_alloc(devName);
    set_stack_attribute(devName, ipPool->getIP());
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    uint8_t buf[2000];
    fd_set oriFD;
    FD_ZERO(&oriFD);
    FD_SET(hijackFD, &oriFD);
    timeval tv{0, 1000};
    Connection::init(ipPool, pipeFD);
    std::thread(readFromFD).detach();
    while (true) {
        auto fdCopy = oriFD;
        auto tvCopy = tv;
        auto selectedFD = select(hijackFD + 1, &fdCopy, nullptr, nullptr, &tvCopy);
        if (selectedFD == 0) {
            Connection::checkTimeOut();
        } else if (FD_ISSET(hijackFD, &fdCopy)) {
            auto r = read(hijackFD, buf, 2000);
            if (r > sizeof(iphdr)) {
                auto *hdr = (iphdr *) buf;
                auto *conn = Connection::GetConnectionByIP(hdr->daddr);
                conn->AddData(buf, r);
                Connection::checkTimeOut();
            }
        } else {
            perror("select");
        }
    }
}
