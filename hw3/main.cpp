#include <iostream>
#include <bitset>
#include <hash_map>
using namespace std;

class key{
}

int main()
{
    // Cache line: 64 Byte
    // L1 size: 64 KB, 2 way
    // L1 addr: 6 + 10 -1 = 15 bit
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
                int addrA = k & ((1 << log1024) - 1);
                addrA = addrA << 3; // 3 bit is the offset (a double)
                int addrAmsb = i & ((1 << log256) - 1);
                addrA = (addrAmsb << (10 + 3)) + addrA;
                int baseA = (1 << 30);   // 0100_0000_0000_0000_0000_0000_0000_0000
                addrA = addrA + baseA;
                //cout << (addrAmsb << (10 + 3)) << endl;
                //cout << addrA << endl;
                //cout << std::bitset<32>((addrAmsb << (10 + 3))) <<endl;
                //cout << std::bitset<32>(addrA) <<endl;

                b = B[k][j];
                int addrB = j & ((1 << log512) - 1);
                addrB = addrB << 3; // 3 bit is the offset (a double)
                int addrBmsb = k & ((1 << log1024) - 1);
                addrB = (addrBmsb << (9 + 3)) + addrB;
                int baseB = (1 << 31); // 1000_0000_0000_0000_0000_0000_0000_0000
                addrB = addrB + baseB;
                //cout << std::bitset<32>(addrB) <<endl;

                c = C[i][j];
                int addrC = j & ((1 << log512) - 1);
                addrC = addrC << 3; // 3 bit is the offset (a double)
                int addrCmsb = i & ((1 << log256) - 1);
                addrC = (addrCmsb << (9 + 3)) + addrC;
                int baseC = (3 << 30);   // 1100_0000_0000_0000_0000_0000_0000_0000
                addrC = addrC + baseC;
                cout << std::bitset<32>(addrC) <<endl;

                c += a * b;

                C[i][j] = c;
                }
          }
    }
    cout << "Hello world!" << endl;
    return 0;
}
