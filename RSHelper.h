//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>
#define gwSize 4096

class RSHelper {
private:
    static int num2field[gwSize];
    static int field2num[gwSize];
    int generatorPolynomial[gwSize + 1];
    int generatorPolynomialTemp[gwSize + 1];
    int currentRSCodeLength;


public:
    void attachRSCode(int *originMessage, int messageLength, int rsCodeLength);
    void generateGeneratorPolynomial(int polynomialLength);
    static bool getOriginMessage(int *message, int messageLength, int rsCodeLength);
    static void init();
    RSHelper();
    int testFunc(int a, int b);
};

#endif //UDPSPEEDER_RSHELPER_H
