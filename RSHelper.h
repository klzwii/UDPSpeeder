//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>
#include <cstring>
#define gwSize 4096
#define codeLength 12

class RSHelper {
private:
    static const unsigned reserveHighBits = 0b11111111111111111111000000000000;
    static const unsigned eraseLowBits = 0b11111111111111110000000000001111;
    static int num2field[gwSize];
    static int field2num[gwSize];
    int tempPolynomial[gwSize];
    int generatorPolynomial[gwSize + 1];
    int generatorPolynomialTemp[gwSize + 1];
    int currentRSCodeLength;
    int polynomialValue[gwSize];
    int solveMatrix[gwSize][gwSize];
    int gaussAns[(gwSize >> 1) + 1];
    int wrongPos[gwSize >> 1];
    static void work(int k, int *workArray, int po);
    void generateGeneratorPolynomial(int polynomialLength);

public:
    bool getOriginMessage(unsigned char *message, int messageLength, int rsCodeLength);
    RSHelper();
    static inline void setPos(int pos, unsigned char *bytes, unsigned int value) {
        auto tempPos = (unsigned int*)(bytes + (pos >> 1) * 3 + (pos & 1));
        if(!(pos&1)) {
            *tempPos &= reserveHighBits;
            value &= ~reserveHighBits;
            *tempPos ^= value;
        } else {
            value &= ~reserveHighBits;
            value <<= 4;
            *tempPos &= eraseLowBits;
            *tempPos ^= value;
        }
    };
    static inline unsigned getPos(int pos, unsigned char *bytes) {
        unsigned temp = 0;
        memcpy(&temp, bytes + (pos >> 1) * 3 + (pos & 1) , 2);
        if(!(pos&1)) {
            temp &= 4095;
        } else {
            temp >>= 4;
        }
        return temp;
    };
    static inline void xorPos(int pos, unsigned char *bytes, unsigned int value) {
        auto tempPos = (unsigned int*)(bytes + (pos >> 1) * 3 + (pos & 1));
        if(!(pos&1)) {
            value &= ~reserveHighBits;
            *tempPos ^= value;
        } else {
            value &= ~reserveHighBits;
            value <<= 4;
            *tempPos ^= value;
        }
    };
    void attachRSCode(unsigned char *originMessage, int messageLength, int rsCodeLength);


};

#endif //UDPSPEEDER_RSHELPER_H
