//
// Created by liuze on 2021/3/14.
//

#ifndef UDPWINCLIENT_HEADERCONST_H
#define UDPWINCLIENT_HEADERCONST_H
#define HEADER_LENGTH 12
struct header {
private:
    static const uint8_t FIN = 0b1;
    static const uint8_t ACK = 0b10;
    static const uint8_t SYN = 0b100;
    static const uint8_t RST = 0b1000;
//    uint32_t crc;  // 0 - 3
//    uint16_t sendSeq; // 4 - 5
//    uint16_t packetLength; // 6 - 7
//    uint8_t subSeq; // 8
//    uint8_t symbol; // 9
//    uint16_t uuid; // 10 - 11
    uint8_t *addr;
public:
    explicit header(uint8_t *bytes) {
        this->addr = bytes;
    }
    void Clear() {
        memset(addr, 0, HEADER_LENGTH);
    }
    void SetCRC(uint32_t crc) {
        *reinterpret_cast<uint32_t*>(addr) = crc;
    }
    void SetSendSeq(uint16_t sendSeq) {
        *reinterpret_cast<uint16_t*>(addr + 4) = sendSeq;
    }
    void SetPacketLength(uint16_t packetLength) {
        *reinterpret_cast<uint16_t *>(addr + 6) = packetLength;
    }

    void SetSubSeq(uint8_t subSeq) {
        *reinterpret_cast<uint8_t *>(addr + 8) = subSeq;
    }

    void SetFIN() {
        *reinterpret_cast<uint8_t *>(addr + 9) |= FIN;
    }

    void SetACK() {
        *reinterpret_cast<uint8_t *>(addr + 9) |= ACK;
    }

    void SetSYN() {
        *reinterpret_cast<uint8_t *>(addr + 9) |= SYN;
    }

    void SetRST() {
        *reinterpret_cast<uint8_t *>(addr + 9) |= RST;
    }

    void SetUUID(uint16_t uuid) {
        *reinterpret_cast<uint16_t *>(addr + 10) = uuid;
    }

    uint32_t CRC() {
        return *reinterpret_cast<uint32_t *>(addr);
    }

    uint16_t SendSeq() {
        return *reinterpret_cast<uint16_t *>(addr + 4);
    }

    uint16_t PacketLength() {
        return *reinterpret_cast<uint16_t *>(addr + 6);
    }

    uint8_t SubSeq() {
        return *reinterpret_cast<uint8_t *>(addr + 8);
    }

    bool IsFin() {
        return *reinterpret_cast<uint8_t *>(addr + 9) & FIN;
    }

    bool IsACK() {
        return *reinterpret_cast<uint8_t *>(addr + 9) & ACK;
    }

    bool IsSYN() {
        return *reinterpret_cast<uint8_t *>(addr + 9) & SYN;
    }

    bool IsRST() {
        return *reinterpret_cast<uint8_t *>(addr + 9) & RST;
    }

    uint16_t UUID() {
        return *reinterpret_cast<uint16_t *>(addr + 10);
    }
};

#endif //UDPWINCLIENT_HEADERCONST_H
