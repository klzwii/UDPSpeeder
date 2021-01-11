//
// Created by liuze on 2020/11/11.
//

#include "RSHelper.h"

//G(2^8) log2num
int RSHelper::num2field[gwSize];
//G(2^8) num2log
int RSHelper::field2num[gwSize];

/*
 * 生成reed-solomon纠错算法的生成多项式 保存在generatorPolynomial中
 * @param polynomialLength 表示所需生成的多项式的最高次数项次数
 */
const static int primitivePolynomial[14][14] = {
        {},
        {},
        {1, 1, 1},
        {1, 0, 1, 1},
        {1, 1, 0, 0, 1},
        {1, 0, 1, 0, 0, 1},
        {1, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 1},
        {1, 0, 1, 1, 1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
        {1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1},
        {1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1}
};
void RSHelper::generateGeneratorPolynomial(int polynomialLength) {
    memset(generatorPolynomial, 0, sizeof(generatorPolynomial));
    generatorPolynomial[0] = field2num[1];
    generatorPolynomial[1] = field2num[1];
    for (int i = 2; i <= polynomialLength; i++) {
        memset(generatorPolynomialTemp, 0, sizeof(generatorPolynomialTemp));
        generatorPolynomialTemp[0] = generatorPolynomial[0] + (i - 1);
        generatorPolynomialTemp[0] %= (gwSize - 1);
        for (int j = 1; j < i; j++) {
            generatorPolynomialTemp[i - j] = generatorPolynomial[i - j - 1];
        }
        for (int j = 1; j < i; j++) {
            int temp = generatorPolynomial[j] + i - 1;
            temp %= (gwSize - 1);
            temp = num2field[temp];
            temp ^= num2field[generatorPolynomialTemp[j]];
            generatorPolynomialTemp[j] = field2num[temp];
        }
        std::swap(generatorPolynomial, generatorPolynomialTemp);
    }
//    for (int i = polynomialLength; i >= 0; i --) {
//        std::cout << num2field[generatorPolynomial[i]] << ",";
//    }
//    std::cout << std::endl;
}

/*
 * 在原始信息的头部附加对应的rs纠错码 如信息长度为3纠错码长度为2则传入数组应为BBXXX(其中B为空字符,X为原始信息)
 * 在函数运行结束后原数组变为SSXXX(S为rs纠错码)
 * @param originMessage 原始信息所保存的数组指针 需要在前端预留出填充纠错码的位置
 * @param messageLength 原始的信息长度 需要注意messageLength + rsCodeLength需要严格小于等于255
 * @param rsCodeLength 纠错码的长度 需要注意messageLength + rsCodeLength需要严格小于等于255
 */
void RSHelper::attachRSCode(int *originMessage, int messageLength, int rsCodeLength) {
    int tempPolynomial[gwSize];
    memset(tempPolynomial, 0, sizeof(tempPolynomial));
    if (rsCodeLength != currentRSCodeLength) {
        currentRSCodeLength = rsCodeLength;
        generateGeneratorPolynomial(rsCodeLength);
    }
    for (int i = 0; i < messageLength; i++) {
        tempPolynomial[i + rsCodeLength] = originMessage[i + rsCodeLength];
    }
    for (int i = messageLength + rsCodeLength; i >= rsCodeLength; i--) {
        if (tempPolynomial[i]) {
            int addFactor = field2num[tempPolynomial[i]];
            for (int j = 0; j <= rsCodeLength; j++) {
                int tempFactor = generatorPolynomial[rsCodeLength - j];
                tempFactor = (tempFactor + addFactor) % (gwSize - 1);
                tempFactor = num2field[tempFactor];
                tempPolynomial[i - j] ^= tempFactor;
            }
        }
    }
    for (int i = 0; i < rsCodeLength; i++) {
        originMessage[i] = tempPolynomial[i];
    }
}

/*
 * 将通过attachRSCode函数产生的带有纠错码并且出错数目少于rsCodeLength>>1的信息还原成原本的信息
 * @param message 需要进行还原的带有纠错码的信息
 * @param messageLength 原始的信息长度 需要注意messageLength + rsCodeLength需要严格小于等于255
 * @param rsCodeLength 纠错码的长度 需要注意messageLength + rsCodeLength需要严格小于等于255
 * @return bool true则为信息已被恢复 false则代表信息错误数过多 无法恢复
 */
bool RSHelper::getOriginMessage(int *message, int messageLength, int rsCodeLength) {

    int polynomialValue[gwSize];
    thread_local int solveMatrix[gwSize][gwSize];
    int isCorrect = 0;
    for (int i = 0; i < rsCodeLength; i++) {
        int tempAns = 0;
        for (int j = rsCodeLength + messageLength - 1; j >= 0; j--) {
            if (!message[j]) {
                continue;
            }
            int tempAlpha = (j * i) + field2num[message[j]];
            tempAlpha %= (gwSize - 1);
            tempAns ^= num2field[tempAlpha];
        }
        polynomialValue[i] = tempAns;
        isCorrect |= tempAns;
    }
    if (!isCorrect) {
        return true;
    }
    int maxWrongPos = rsCodeLength >> 1;  //初始假设错误有 rsCodeLength>>1 个，检验矩阵是否满秩
    int matrixRank = 0;
    while (true) {
        for (int i = 0; i < maxWrongPos; i++) {
            for (int j = 0; j < maxWrongPos; j++) {
                solveMatrix[i][j] = polynomialValue[i + j];
            }
        }
        // 开始将矩阵转换为行阶梯形式
        for (int i = 0; i < maxWrongPos; i++) {
            bool done = false;
            bool finished = true;
            for (int j = i; j < maxWrongPos; j++) {
                for (int k = i; k < maxWrongPos; k++) {
                    if (solveMatrix[k][j]) {
                        finished = false;
                        matrixRank++;
                        std::swap(solveMatrix[k], solveMatrix[i]);
                        int tempAlpha = field2num[solveMatrix[i][j]];
                        for (int kk = j; kk < maxWrongPos; kk++) {
                            if (!solveMatrix[i][kk]) {
                                continue;
                            }
                            int calcAlpha = field2num[solveMatrix[i][kk]] - tempAlpha;
                            calcAlpha += (gwSize - 1);
                            calcAlpha %= (gwSize - 1);
                            solveMatrix[i][kk] = num2field[calcAlpha];
                        }
                        for (int kc = k + 1; kc < maxWrongPos; kc++) {
                            int tempAlpha = field2num[solveMatrix[kc][j]];
                            if (solveMatrix[kc][j]) {
                                for (int kk = j; kk < maxWrongPos; kk++) {
                                    if (!solveMatrix[i][kk]) {
                                        continue;
                                    }
                                    int calcAlpha = field2num[solveMatrix[i][kk]] + tempAlpha;
                                    calcAlpha %= (gwSize - 1);
                                    solveMatrix[kc][kk] ^= num2field[calcAlpha];
                                }
                            }
                        }
                        done = true;

                        break;
                    }
                }
                if (done) {
                    break;
                }
            }
            if (finished) {
                break;
            }
        }
        if (matrixRank == maxWrongPos) { //当计算出来的矩阵秩和假设的错误数相同时，代表当前矩阵满秩同时错误数与矩阵秩相同
            break;                       //(当错误过多时，矩阵秩为rsCodeLength>>1，且无法恢复错误）
        } else {
            maxWrongPos = matrixRank; //当前矩阵若不是满秩，其错误数最小是当前矩阵秩个，因此在下一次迭代可以直接将矩阵的最大秩设置为当前秩
            matrixRank = 0;
        }
    }
    for (int i = 0; i < matrixRank; i++) {
        for (int j = 0; j <= matrixRank; j++) {
            solveMatrix[i][j] = polynomialValue[i + j];
        }
    }
    // 高斯消元法进行定位多项式系数求解
    for (int i = 0; i < matrixRank; i++) {
        if (!solveMatrix[i][i]) {
            for (int j = i + 1; j < matrixRank; j++) {
                if (solveMatrix[j][i]) {
                    std::swap(solveMatrix[i], solveMatrix[j]);
                }
            }
        }
        int tempAlpha = field2num[solveMatrix[i][i]];
        for (int j = i; j <= matrixRank; j++) {
            if (solveMatrix[i][j] == 0)
                continue;
            int calcAlpha = field2num[solveMatrix[i][j]] - tempAlpha;
            calcAlpha += (gwSize - 1);
            calcAlpha %= (gwSize - 1);
            solveMatrix[i][j] = num2field[calcAlpha];
        }
        for (int j = i + 1; j < matrixRank; j++) {
            if (!solveMatrix[j][i]) {
                continue;
            }
            int tempAlpha = field2num[solveMatrix[j][i]];
            for (int k = i; k <= matrixRank; k++) {
                int tempFactor = 0;
                if (solveMatrix[i][k]) {
                    int calcAlpha = tempAlpha + field2num[solveMatrix[i][k]];
                    calcAlpha %= (gwSize - 1);
                    tempFactor = num2field[calcAlpha];
                }
                solveMatrix[j][k] ^= tempFactor;
            }
        }
    }
    for (int i = matrixRank - 1; i >= 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            if (!solveMatrix[j][i]) {
                continue;
            }
            if (!solveMatrix[i][matrixRank]) {
                solveMatrix[j][i] = 0;
                continue;
            }
            int tempAlpha = field2num[solveMatrix[j][i]];
            solveMatrix[j][i] = 0;
            int calcAlpha = field2num[solveMatrix[i][matrixRank]] + tempAlpha;
            calcAlpha %= (gwSize - 1);
            solveMatrix[j][matrixRank] ^= num2field[calcAlpha];
        }
    }
    int gaussAns[(gwSize >> 1) + 1];
    for (int i = 0; i < matrixRank; i++) {
        gaussAns[i] = solveMatrix[i][matrixRank];
    }
    // 高斯消元法结束 开始通过求得定位多项式进行错误位置求解
    int sumWrongPos = 0;
    int wrongPos[gwSize >> 1];
    for (int i = 0; i < rsCodeLength + messageLength; i++) {
        int calcAns = 1;
        for (int j = 0; j < matrixRank; j++) {
            if (!gaussAns[matrixRank - j - 1]) {
                continue;
            }
            int tempFactor = field2num[gaussAns[matrixRank - j - 1]];
            tempFactor -= i * (j + 1);
            tempFactor %= (gwSize - 1);
            tempFactor += (gwSize - 1);
            tempFactor %= (gwSize - 1);
            calcAns ^= num2field[tempFactor];
        }
        if (!calcAns) {
            wrongPos[sumWrongPos++] = i;
        }
    }

    //如果求出的错误位置和矩阵的秩不相同 代表错误过多无法消除(或者有bug TAT)
    if (sumWrongPos != matrixRank) {
        return false;
    }

    for (int i = 0; i < sumWrongPos; i++) {
        for (int j = 0; j < sumWrongPos; j++) {
            //std::cout << wrongPos[j] << " " << i << " " << j << std::endl;
            solveMatrix[i][j] = num2field[(wrongPos[j] * i) % (gwSize - 1)];
        }
       solveMatrix[i][sumWrongPos] = polynomialValue[i];
    }

    // 高斯消元法进行错误恢复
    for (int i = 0; i < matrixRank; i++) {
        if (!solveMatrix[i][i]) {
            for (int j = i + 1; j < matrixRank; j++) {
                if (solveMatrix[j][i]) {
                    std::swap(solveMatrix[i], solveMatrix[j]);
                }
            }
        }
        int tempAlpha = field2num[solveMatrix[i][i]];
        for (int j = i; j <= matrixRank; j++) {
            if (solveMatrix[i][j] == 0)
                continue;
            int calcAlpha = field2num[solveMatrix[i][j]] - tempAlpha;
            calcAlpha += (gwSize - 1);
            calcAlpha %= (gwSize - 1);
            solveMatrix[i][j] = num2field[calcAlpha];
        }
        for (int j = i + 1; j < matrixRank; j++) {
            if (!solveMatrix[j][i]) {
                continue;
            }
            int tempAlpha = field2num[solveMatrix[j][i]];
            for (int k = i; k <= matrixRank; k++) {
                int tempFactor = 0;
                if (solveMatrix[i][k]) {
                    int calcAlpha = tempAlpha + field2num[solveMatrix[i][k]];
                    calcAlpha %= (gwSize - 1);
                    tempFactor = num2field[calcAlpha];
                }
                solveMatrix[j][k] ^= tempFactor;
            }
        }
    }
    for (int i = matrixRank - 1; i >= 0; i--) {
        for (int j = i - 1; j >= 0; j--) {
            if (!solveMatrix[j][i]) {
                continue;
            }
            if (!solveMatrix[i][matrixRank]) {
                solveMatrix[j][i] = 0;
                continue;
            }
            int tempAlpha = field2num[solveMatrix[j][i]];
            solveMatrix[j][i] = 0;
            int calcAlpha = field2num[solveMatrix[i][matrixRank]] + tempAlpha;
            calcAlpha %= (gwSize - 1);
            solveMatrix[j][matrixRank] ^= num2field[calcAlpha];
        }
    }

    for (int i = 0; i < matrixRank; i++) {
        gaussAns[i] = solveMatrix[i][matrixRank];
    }
    // 高斯消元法结束 将传入消息进行错误恢复
    for (int i = 0; i < sumWrongPos; i++) {
        message[wrongPos[i]] ^= gaussAns[i];
    }
    // 验证消息是否正确恢复
    for (int i = 0; i < rsCodeLength; i++) {
        int tempAns = 0;
        for (int j = rsCodeLength + messageLength - 1; j >= 0; j--) {
            if (!message[j]) {
                continue;
            }
            int tempAlpha = (j * i) + field2num[message[j]];
            tempAlpha %= (gwSize - 1);
            tempAns ^= num2field[tempAlpha];
        }
        polynomialValue[i] = tempAns;
    }
    for (int i = 0; i < rsCodeLength; i++) {
        if (polynomialValue[i]) {
            return false;
        }
    }
    return true;
}

int RSHelper::testFunc(int a, int b) {
    return num2field[(field2num[a] + field2num[b]) % (gwSize - 1)];
}

RSHelper::RSHelper() {
    currentRSCodeLength = - 1;
    memset(generatorPolynomial, 0, sizeof(generatorPolynomial));
    memset(generatorPolynomialTemp, 0, sizeof(generatorPolynomialTemp));
}

void RSHelper::work(int k, int* workArray, int po) {
    memset(workArray, 0 ,sizeof(int) * gwSize);
    workArray[k] = 1;
    int i = k;
    for (; i >= po; i--) {
        if (!workArray[i]) {
            continue;
        }
        for (int j = po; j >= 0; j --) {
            workArray[i + j - po] ^= primitivePolynomial[po][j];
        }
    }
    int sum = 0, addVal = 1;
    for (i = 0; i < po; i++) {
        sum += workArray[i] * addVal;
        addVal <<= 1;
    }
    RSHelper::num2field[k] = sum;
    field2num[sum] = k;
}

void RSHelper::init() {
    static_assert((gwSize & (-gwSize)) == gwSize, "not 2 pow");
    static_assert(gwSize <= 8192, "too big");
    static_assert(gwSize >= 4, "too small");
    int k = gwSize, po = 0;
    while (k) {
        ++po;
        k = k >> 1;
    }
    --po;
    int workArray[gwSize];
    for (int i = 0; i < gwSize; i++) {
        work(i, workArray, po);
    }
}


