#include <ctime>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/ip.h>
#include <ifaddrs.h>
#include "HeaderConst.h"
#include <arpa/inet.h>
#include <thread>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <libnetfilter_queue/libnetfilter_queue_ipv4.h>
#include <linux/netfilter.h>
#include <map>
#include "RDT.h"

struct sockaddr_in structsockaddrIn{}, sout{};
socklen_t sockLen;
int rawFD;
std::map<uint16_t, RDT*>rdtMap;
RDT* rdt = nullptr;

void printIP(uint32_t netIP) {
    auto *k = (uint8_t*)&netIP;
    for (int i = 0; i <= 3; i ++) {
        printf("%d.", (int)*(k+i));
    }
    printf("\n");
}

int fd;

bool checkIPOut(struct iphdr* iph) {
    auto k = be32toh(iph->daddr);
    auto *t = (uint8_t*)&k;
    auto fir = *(t+3), sec = *(t+2);
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

void reCalcChecksum(uint16_t *buffer, size_t len) {
    auto *iph = (iphdr*)buffer;
    iph->check = 0;
    uint32_t cksum = 0;
    while(len)
    {
        cksum += *(buffer++);
        len -= 2;
    }
    while (cksum >> 16) {
        uint16_t t = cksum >> 16;
        cksum &= 0x0000ffff;
        cksum += t;
    }
    iph->check = ~cksum;
}

int cb(struct nfq_q_handle *gh, struct nfgenmsg *nfmsg, struct nfq_data *nfad, void *data) {
    struct iphdr *iph;
    struct nfqnl_msg_packet_hdr *ph;
    ph = nfq_get_msg_packet_hdr(nfad);
    unsigned char* payload;
    if (ph) {
        uint32_t id = ntohl(ph->packet_id);
        int mark = nfq_get_nfmark(nfad);
        if (mark == 100) {
            nfq_set_verdict(gh, id, NF_ACCEPT, 0, nullptr);
        }
        // todo check if dest ip exist in IP pool
        int r = nfq_get_payload(nfad, &payload);
        if (r >= sizeof(struct iphdr)) {
            iph = reinterpret_cast<struct iphdr*>(payload);
            std::string s;
            switch (iph->protocol) {
                case IPPROTO_ICMP:
                    s = "icmp";
                    break;
                case IPPROTO_UDP:
                    s = "udp";
                    break;
            }
            printf("get %s packets, length %d\n", s.c_str(), (int)be16toh(iph->tot_len));
            printIP(iph->saddr);
            printIP(iph->daddr);
            rdt->AddData(payload, r);
            rdt->BufferTimeOut();
            return nfq_set_verdict(gh, id, NF_ACCEPT, 0, nullptr);
        }
    }
    return 0;
}


void readFromFD() {
    uint8_t buffer[2000];
    int len;
    fd_set readFD;
    FD_ZERO(&readFD);
    FD_SET(fd, &readFD);
    timeval timeOut{0, 20000};
    while (true) {
        auto setCopy = readFD;
        auto tvCopy = timeOut;
        int selectedFD = select(fd + 1, &readFD, nullptr, nullptr, &tvCopy);
        if (selectedFD == 0) {
            if (rdt == nullptr) {
                continue;
            }
            rdt->TimeOut();
            rdt->DumpData();
            // todo timeout
        } else if (FD_ISSET(fd, &setCopy)) {
            len = recvfrom(fd, buffer, 2000, 0, (sockaddr*)&sout, &sockLen);
            if (rdt == nullptr) {
                rdt = new RDT(128, 5, 400, 2, &sout, 10, 2000, 50);
            }
            if (len < 0) {
                perror("recv from");
            }
            rdt->RecvBuffer(buffer);
            rdt->DumpData();
            rdt->TimeOut();
        } else {
            perror("select");
        }
    }
    while ((len = recvfrom(fd, buffer, 2000, 0, (sockaddr*)&sout, &sockLen)) > 0) {
//        auto iph = (iphdr*)buffer;
//        iph->saddr = inet_addr("192.168.23.1");
//        printf("111\n");
//        printIP(iph->saddr);
//        printIP(iph->daddr);
//        printf("111\n");
//        reCalcChecksum((uint16_t*)buffer, len);
//        struct sockaddr_in out{};
//        out.sin_addr.s_addr = iph->daddr;
//        out.sin_family = AF_INET;
        auto head = header(buffer);
        if (rdt == nullptr) {
            rdt = new RDT(128, 5, 400, 2, &sout, 10, 2000, 50);
        }
        rdt->RecvBuffer(buffer);
        //auto k = sendto(rawFD, buffer, len, 0, (sockaddr*)&out, sizeof(sockaddr_in));
        //printf("%ld\n", k);
    }
    if (len < 0) {
        perror("recv");
    }
}

int main() {
    // getSubnetMask();
    int one = 1;
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
    auto c = bind(fd, (sockaddr *)&structsockaddrIn, sizeof(struct sockaddr_in));
    if (c < 0) {
        perror("bind");
        exit(0);
    }
    RDT::init(fd);
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int r;
    char buf[10240];
    //auto f = popen("iptables -A OUTPUT -j NFQUEUE 1234", "r");
    h = nfq_open();
    if (h == NULL) {
        perror("nfq_open error");
        exit(0);
    }
    if (nfq_unbind_pf(h, AF_INET) != 0) {
        perror("nfq_unbind_pf error");
        exit(0);
    }
    if (nfq_bind_pf(h, AF_INET) != 0) {
        perror("nfq_bind_pf error");
        exit(0);
    }
    qh = nfq_create_queue(h, 1234, &cb, NULL);
    if (qh == NULL) {
        perror("nfq_create_queue error");
        exit(0);
    }

    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) != 0) {
        perror("nfq_set_mod error");
        exit(0);
    }
    int queueFD = nfq_fd(h);
    fd_set oriFD;
    FD_ZERO(&oriFD);
    FD_SET(queueFD, &oriFD);
    timeval tv{1, 0};
    std::thread(readFromFD).detach();
    while(true) {
        auto fdCopy = oriFD;
        auto tvCopy = tv;
        auto selectedFD = select(queueFD + 1, &fdCopy, nullptr, nullptr, &tvCopy);
        if (selectedFD == 0) {
            //todo check all clients send buffer time out
            if (rdt != nullptr)
            rdt->BufferTimeOut();
        } else if (FD_ISSET(queueFD, &fdCopy)) {
            r = recv(queueFD, buf, sizeof(buf), MSG_DONTWAIT);
            if (r == 0) {
                printf("recv return 0. exit");
                break;
            } else if (r < 0) {
                perror("recv error");
                break;
            } else {
                nfq_handle_packet(h, buf, r);
            }
        } else {
            perror("select");
        }
    }
    return 0;
}
