//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>

class RSHelper {
private:
    static unsigned short num2field[256];
    static unsigned short field2num[256];
    static int generatorPolynomial[256];
    static int generatorPolynomialTemp[256];
    static int currentRSCodeLength;
    static void generateGeneratorPolynomial(unsigned short polynomialLength);

public:
    void attachRSCode(int *originMessage, unsigned short messageLength,
                      unsigned short rsCodeLength);
    bool getOriginMessage(int *message, unsigned short messageLength,
                          unsigned short rsCodeLength);
};

#endif //UDPSPEEDER_RSHELPER_H
