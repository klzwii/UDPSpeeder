//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>
#define gwSize 4096

class RSHelper {
private:
    static int num2field[4096];
    static int field2num[4096];
    static int generatorPolynomial[4096];
    static int generatorPolynomialTemp[4096];
    static int currentRSCodeLength;
    static void generateGeneratorPolynomial(int polynomialLength);

public:
    void attachRSCode(int *originMessage, int messageLength, int rsCodeLength);

    bool getOriginMessage(int *message, int messageLength, int rsCodeLength);
};

#endif //UDPSPEEDER_RSHELPER_H
