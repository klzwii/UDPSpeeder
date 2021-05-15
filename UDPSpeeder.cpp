#include <ctime>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include "HeaderConst.h"
#include "Connection.h"
#include <arpa/inet.h>
#include <thread>
#include <map>
#include <linux/if_tun.h>
#include "NAT.h"
#include <linux/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "RDT.h"

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

int set_stack_attribute(char *dev) {
    struct ifreq ifr{};
    struct sockaddr_in addr{};
    int sockfd, err = -1;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.62.1");
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

bool checkIPOut(struct iphdr *iph) {
    auto k = be32toh(iph->daddr);
    auto *t = (uint8_t *) &k;
    auto fir = *(t + 3), sec = *(t + 2);
    if (fir == 127 || fir == 10) {
        return false;
    }
    if (fir == 192 && sec == 168) {
        return false;
    }
    if (fir == 169 && sec == 254) {
        return false;
    }
    if (fir == 172 && sec >= 26 && sec <= 31) {
        return false;
    }
    if (fir >= 224) {
        return false;
    }
    return true;
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
            sockaddr tempAddr{};
            socklen_t tempLen;
            len = recvfrom(fd, buffer, 2000, 0, &tempAddr, &tempLen);
            if (len < 0) {
                perror("recv");
                continue;
            }
            auto head = header(buffer);
            auto *conn = Connection::getConn(head.UUID());
            if (head.IsACK() || head.IsSYN()) {
                int sendBack = Connection::startConn(buffer, &tempAddr, &tempLen);
                if (sendBack != 0) {
                    auto ret = sendto(fd, buffer, sendBack, 0, &tempAddr, tempLen);
                    if (ret < 0) {
                        perror("send back ret");
                    } else {
                        printf("send back bytes %d\n", sendBack);
                    }
                }
            } else if (conn != nullptr) {
                if (rdt == nullptr) {
                    rdt = conn->rdt;
                }
                conn->RecvBuffer(buffer);
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
    // getSubnetMask();
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
    set_stack_attribute(devName);
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    uint8_t buf[2000];
    fd_set oriFD;
    FD_ZERO(&oriFD);
    FD_SET(hijackFD, &oriFD);
    timeval tv{0, 1000};
    std::thread(readFromFD).detach();
    while (true) {
        auto fdCopy = oriFD;
        auto tvCopy = tv;
        auto selectedFD = select(hijackFD + 1, &fdCopy, nullptr, nullptr, &tvCopy);
        if (selectedFD == 0) {
            if (rdt != nullptr) {
                rdt->BufferTimeOut();
            }
        } else if (FD_ISSET(hijackFD, &fdCopy)) {
            auto r = read(hijackFD, buf, 2000);
            rdt->AddData(buf, r, true);
            rdt->BufferTimeOut();
        } else {
            perror("select");
        }
    }
}
