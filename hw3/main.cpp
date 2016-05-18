#include <iostream>
#include <bitset>
using namespace std;

int main()
{
    // L1 size: 64 KB, 2 way
    // L1 index: 6 + 10 -1 = 15 bit
    double A[256][1024];
    // [256]: 8 bit, [1024]: 10 bit, double(offset is 8 byte): 3 bit, total: 8+10+3 = 21 bit
    int log8 = 3;
    int log256 = 8;
    int log512 = 9;
    int log1024 = 10;
    double B[1024][512];
    // [1024]: 10 bit, [512]: 9bit, double(offset is 8 byte): 3 bit, total: 10 + 9 +3 = 22 bit
    double C[256][512];
    // [256]: 8 bit, [512]: 9 bit, double(offset is 8 byte): 3 bit, total: 8 + 9 + 3 = 20 bit

    double a,b,c;
    int i,j,k;

    for(i = 0; i < 128; i+=2) {
          for(j = 0; j < 128; j++) {
                for(k = 0; k < 128; k+=4) {

                a = A[i][k];
                int indexA = k & ((1 << log1024) - 1);
                indexA = indexA << 3; // 3 bit is the offset (a double)
                int indexAmsb = i & ((1 << log256) - 1);
                indexA = (indexAmsb << (10 + 3)) + indexA;
                //cout << (indexAmsb << (10 + 3)) << endl;
                //cout << indexA << endl;
                //cout << std::bitset<32>((indexAmsb << (10 + 3))) <<endl;
                //cout << std::bitset<32>(indexA) <<endl;

                b = B[k][j];
                int indexB = j & ((1 << log512) - 1);
                indexB = indexB << 3; // 3 bit is the offset (a double)
                int indexBmsb = k & ((1 << log1024) - 1);
                indexB = (indexBmsb << (9 + 3)) + indexB;
                //cout << std::bitset<32>(indexB) <<endl;

                c = C[i][j];
                int indexC = j & ((1 << log512) - 1);
                indexC = indexC << 3; // 3 bit is the offset (a double)
                int indexCmsb = i & ((1 << log256) - 1);
                indexC = (indexCmsb << (9 + 3)) + indexC;
                cout << std::bitset<32>(indexC) <<endl;

                c += a * b;

                C[i][j] = c;
                }
          }
    }
    cout << "Hello world!" << endl;
    return 0;
}
