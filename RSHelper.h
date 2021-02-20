//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>
#define gwSize 4096
#define codeLength 12

class RSHelper {
private:
    static int num2field[gwSize];
    static int field2num[gwSize];
    int generatorPolynomial[gwSize + 1];
    int generatorPolynomialTemp[gwSize + 1];
    int currentRSCodeLength;
    static void work(int k, int *workArray, int po);
    void generateGeneratorPolynomial(int polynomialLength);

public:
    static bool getOriginMessage(unsigned char *message, int messageLength, int rsCodeLength);
    RSHelper();
    static void setPos(int pos, unsigned char *bytes, unsigned int value);
    static unsigned getPos(int pos, unsigned char *bytes);

    void attachRSCode(unsigned char *originMessage, int messageLength, int rsCodeLength);
};

#endif //UDPSPEEDER_RSHELPER_H
