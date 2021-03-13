//
// Created by liuze on 2020/11/11.
//

#ifndef UDPSPEEDER_RSHELPER_H
#define UDPSPEEDER_RSHELPER_H
#include <iostream>
#define gwSize 256

class RSHelper {
private:
    static int num2field[gwSize];
    static int field2num[gwSize];
    int generatorPolynomial[gwSize + 1];
    int generatorPolynomialTemp[gwSize + 1];
    int currentRSCodeLength;
    void generateGeneratorPolynomial(int polynomialLength);

public:
    void attachRSCode(int *originMessage, int messageLength, int rsCodeLength);
    static bool getOriginMessage(int *message, int messageLength, int rsCodeLength);


    void generateRSPacket(unsigned char **packets, int originalLength, int rsLength, size_t packetSize);

    bool
    getOriginMessage1(unsigned char **packets, int originalLength, int rsLength, int offset, int batchLength);

    bool GetOriginMessageFromPackets(unsigned char **packets, int originalLength, int rsLength, size_t packetSize);
};

#endif //UDPSPEEDER_RSHELPER_H
