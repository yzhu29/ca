#include <iostream>
#include <bitset>
#include <unordered_map>

using namespace std;

//#define PRINT
class Data{
public:
    int tag;
    bool lru;
    Data() {};
    Data (int tag, bool lru):tag(tag), lru(lru){

    }
};

unordered_multimap<int, Data> L1;
unordered_multimap<int, Data> L2;


    int hit_count1 = 0;
    int miss_count1 = 0;
    int access_count1 = 0;
    int evict_count1 = 0;

    int hit_count2 = 0;
    int miss_count2 = 0;
    int access_count2 = 0;
    int evict_count2 = 0;

    int valid_cacheline1 = 0;
    int valid_cacheline2 = 0;

    char array_set;

void print_stats1(char as, bool hit, int index, int tag, int count, Data d1, Data d2) {
#ifdef PRINT
    cout << as << endl;
    if (hit) {
        cout << "Hit." << endl;
    }
    else{
        cout << "Miss." <<endl;
    }
    cout << "index: " << index << " tag: " << tag << endl;
    if (count <= 0){
        cout << "insert new data to L1" <<endl;
    }
    else if (count == 1) {
        cout << "index: " << index << " tag: " << d1.tag << " lru: " <<d1.lru << endl;
        cout << "(One matching data)" <<endl;
    }
    else if (count == 2) {
        cout << "index: " << index << " tag: " << d1.tag << " lru: " <<d1.lru << endl;
        cout << "index: " << index << " tag: " << d2.tag << " lru: " <<d2.lru << endl;
        if (hit) {
            cout << "(Two matching data)" <<endl;
        }
        else {
            cout << "(Two matching data; one eviction)" <<endl;
        }
    }
    else {
        cout << "Containing more than 2 cachelines" <<endl;
        exit(1);
    }
#endif // PRINT
}

void print_stats1(char as, bool hit, int index, int tag, int count, Data d1) {
#ifdef PRINT
    cout << as << endl;
    if (hit) {
        cout << "Hit." << endl;
    }
    else{
        cout << "Miss." <<endl;
    }
    cout << "index: " << index << " tag: " << tag << endl;
    if (count <= 0){
        cout << "insert new data to L1" <<endl;
    }
    else if (count == 1) {
        cout << "index: " << index << " tag: " << d1.tag<< " lru: " <<d1.lru << endl;
        cout << "(One matching data)" <<endl;
    }
    else {
        cout << "Containing more than 2 cachelines" <<endl;
        exit(1);
    }
#endif // PRINT
}

void print_stats1(char as, bool hit, int index, int tag, int count) {
#ifdef PRINT

    cout << as << endl;
    if (hit) {
        cout << "Hit." << endl;
    }
    else{
        cout << "Miss." <<endl;
    }
    cout << "index: " << index << " tag: " << tag << endl;
    if (count <= 0){
        cout << "insert new data to L1" <<endl;
    }
    else {
        cout << "Containing more than 2 cachelines" <<endl;
        exit(1);
    }
#endif // PRINT
}

bool cache_access1 (char as, int index, int tag, Data d) {
    bool hit = 1;
    pair<int, Data> mypair(index, d);
                int count = L1.count(index);
                // Miss
                if (count <= 0) {
                    miss_count1++;
                    hit = 0;
                    print_stats1(as, hit, index, tag, count);
                    L1.insert(mypair);
                    valid_cacheline1++;
                }
                else if (count == 1) {
                    auto range = L1.equal_range(index);
                    auto it = range.first;
                    Data d1 = it->second;
                    // Hit
                    if (d1.tag == tag) {
                        it->second.lru = 0;
                        //cout << "Hit" << endl;
                        hit_count1++;
                        print_stats1(as, hit, index, tag, count, d1);
                    }

                    // Miss
                    else {
                        //cout << "Miss" << endl;
                        miss_count1++;
                        hit = 0;
                        print_stats1(as, hit, index, tag, count, d1);
                        it->second.lru = 1;
                        L1.insert(mypair);
                        valid_cacheline1++;
                    }
                }
                else if (count == 2) {
                    auto range = L1.equal_range(index);
                    auto it = range.first;
                    auto it_1 = range.first;
                    auto it_2 = ++it;
                    Data d1 = it_1->second;
                    Data d2 = it_2->second;
                    // Hit
                    if (d1.tag == tag) {
                        print_stats1(as, hit, index, tag, count, d1, d2);
                        it_1->second.lru = 0;
                        it_2->second.lru = 1;
                        //cout << "Hit" << endl;
                        hit_count1++;
                    }
                    else if (d2.tag == tag) {
                        print_stats1(as, hit, index, tag, count, d1, d2);
                        it_1->second.lru = 1;
                        it_2->second.lru = 0;
                        //cout << "Hit" << endl;
                        hit_count1++;
                    }
                    else if (d1.lru == 1) {
                        //cout << "Miss" << endl;
                        print_stats1(as, hit, index, tag, count, d1, d2);
                        miss_count1++;
                        evict_count1++;
                        hit = 0;

                        it_2->second.lru = 1;
                        L1.erase(it_1);
                        L1.insert(mypair);
                    }
                    else {
                        //cout << "Miss" << endl;
                        miss_count1++;
                        evict_count1++;
                        hit = 0;
                        print_stats1(as, hit, index, tag, count, d1, d2);
                        it_1->second.lru = 1;
                        L1.erase(it_2);
                        L1.insert(mypair);
                    }

                }
                else {
                    cout << "Containing more than 2 cachelines" <<endl;
                    exit(1);
                }
    return hit;
}

void cache_access2 (int index, int tag, Data d) {
    bool hit = 1;
    pair<int, Data> mypair(index, d);
                int count = L2.count(index);
                // Miss
                if (count <= 0) {
                    L2.insert(mypair);
                    miss_count2++;
                    hit = 0;
                    valid_cacheline2++;
                }
                else if (count == 1) {
                    auto range = L2.equal_range(index);
                    auto it = range.first;
                    Data d1 = it->second;
                    // Hit
                    if (d1.tag == tag) {
                        it->second.lru = 0;
                        //cout << "Hit" << endl;
                        hit_count2++;
                    }

                    // Miss
                    else {
                        it->second.lru = 1;
                        //cout << "Miss" << endl;
                        miss_count2++;
                        hit = 0;
                        L2.insert(mypair);
                        valid_cacheline2++;
                    }
                }
                else if (count == 2) {
                    auto range = L2.equal_range(index);
                    auto it = range.first;
                    auto it_1 = range.first;
                    auto it_2 = ++it;
                    Data d1 = it_1->second;
                    Data d2 = it_2->second;
                    // Hit
                    if (d1.tag == tag) {
                        it_1->second.lru = 0;
                        it_2->second.lru = 1;
                        //cout << "Hit" << endl;
                        hit_count2++;
                    }
                    else if (d2.tag == tag) {
                        it_1->second.lru = 1;
                        it_2->second.lru = 0;
                        //cout << "Hit" << endl;
                        hit_count2++;
                    }
                    else if (d1.lru == 1) {
                        it_2->second.lru = 1;
                        //cout << "Miss" << endl;
                        miss_count2++;
                        evict_count2++;
                        hit = 0;
                        L2.erase(it_1);
                        L2.insert(mypair);
                    }
                    else {
                        it_1->second.lru = 1;
                        //cout << "Miss" << endl;
                        miss_count2++;
                        evict_count2++;
                        hit = 0;
                        L2.erase(it_2);
                        L2.insert(mypair);
                    }

                }
                else {
                    cout << "Containing more than 2 cachelines" <<endl;
                    exit(1);
                }
}

int main()
{
    // Cache line: 64 Byte, block offset=6 bit
    // L1 size: 64 KB, 2 way
    // L1 index: 1024 cache lines and 2 way = 10 - 1 = 9 bit
    // L2 size: 512 KB, 2 way
    // L2 index: 8192 cache lines and 2 way = 13 - 1 = 12 bit
    double A[256][1024];
    // [256]: 8 bit, [1024]: 10 bit, double(offset is 8 byte): 3 bit, total: 8+10+3 = 21 bit
    int log8 = 3;
    int log256 = 8;
    int log512 = 9;
    int log1024 = 10;
    int logindex1 = 9;
    int logoffset = 6;
    int logindex2 = 12;
    double B[1024][512];
    // [1024]: 10 bit, [512]: 9bit, double(offset is 8 byte): 3 bit, total: 10 + 9 +3 = 22 bit
    double C[256][512];
    // [256]: 8 bit, [512]: 9 bit, double(offset is 8 byte): 3 bit, total: 8 + 9 + 3 = 20 bit

    double a,b,c;
    int i,j,k;

    long baseA = long(A);
    //cout << "baseA:\t" << bitset<64>(baseA) <<endl;
    cout << "baseA:\t" << hex << baseA << endl;
    long baseB = long(B);
    //cout << "baseB:\t" << bitset<64>(baseB) <<endl;
    cout << "baseB:\t" << hex << baseB << endl;
    long baseC = long(C);
    //cout << "baseC:\t" << bitset<64>(baseC) <<endl;
    cout << "baseC:\t" << hex << baseC << endl;





    for(i = 0; i < 128; i+=2) {
          for(j = 0; j < 128; j++) {
                for(k = 0; k < 128; k+=4) {

                a = A[i][k];
                /*
                int addrA = k & ((1 << log1024) - 1);
                addrA = addrA << 3; // 3 bit is the offset (a double)
                int addrAmsb = i & ((1 << log256) - 1);
                addrA = (addrAmsb << (10 + 3)) + addrA;
                addrA = addrA + baseA;
                */
                long addrA = long(&A[i][k]);
                int indexA1 = ((unsigned)(addrA>>logoffset)) & ((1 << logindex1) - 1);
                int tagA1 = (unsigned) addrA >> (logindex1 + logoffset);
                int indexA2 = ((unsigned)(addrA>>logoffset)) & ((1 << logindex2) - 1);
                int tagA2 = (unsigned) addrA >> (logindex2 + logoffset);
                Data d1(tagA1, 0);
                Data d2(tagA2, 0);
                char asA = 'A';
                bool hitA = cache_access1(asA, indexA1, tagA1, d1);
                if(!hitA){
                    cache_access2(indexA2, tagA2, d2);
                }

                //cout << count <<endl;


                //cout << (addrAmsb << (10 + 3)) << endl;
                //cout << addrA << endl;
                //cout << std::bitset<32>((addrAmsb << (10 + 3))) <<endl;
                //cout << std::bitset<32>(tagA1) <<endl;

                b = B[k][j];
                /*
                int addrB = j & ((1 << log512) - 1);
                addrB = addrB << 3; // 3 bit is the offset (a double)
                int addrBmsb = k & ((1 << log1024) - 1);
                addrB = (addrBmsb << (9 + 3)) + addrB;
                addrB = addrB + baseB;
                */
                long addrB = long(&B[k][j]);
                int indexB1 = ((unsigned)(addrB>>logoffset)) & ((1 << logindex1) - 1);
                int tagB1 = (unsigned) addrB >> (logindex1 + logoffset);
                int indexB2 = ((unsigned)(addrB>>logoffset)) & ((1 << logindex2) - 1);
                int tagB2 = (unsigned) addrB >> (logindex2 + logoffset);
                Data db1(tagB1, 0);
                Data db2(tagB2, 0);
                char asB = 'B';
                bool hitB = cache_access1(asB, indexB1, tagB1, db1);
                if(!hitB){
                    cache_access2(indexB2, tagB2, db2);
                }
                //cout << std::bitset<32>(tagB1) <<endl;

                c = C[i][j];
                /*
                int addrC = j & ((1 << log512) - 1);
                addrC = addrC << 3; // 3 bit is the offset (a double)
                int addrCmsb = i & ((1 << log256) - 1);
                addrC = (addrCmsb << (9 + 3)) + addrC;
                addrC = addrC + baseC;
                */
                long addrC = long(&C[i][j]);
                int indexC1 = ((unsigned)(addrC>>logoffset)) & ((1 << logindex1) - 1);
                int tagC1 = (unsigned) addrC >> (logindex1 + logoffset);
                int indexC2 = ((unsigned)(addrC>>logoffset)) & ((1 << logindex2) - 1);
                int tagC2 = (unsigned) addrC >> (logindex2 + logoffset);
                Data dc1(tagC1, 0);
                Data dc2(tagC2, 0);
                char asC = 'C';
                bool hitC = cache_access1(asC, indexC1, tagC1, dc1);
                if (!hitC){
                    cache_access2(indexC2, tagC2, dc2);
                }
                //cout << std::bitset<32>(indexC1) <<endl;
                //cout << std::bitset<32>(tagC1) <<endl;

                c += a * b;

                C[i][j] = c;
                hitC = cache_access1(asC, indexC1, tagC1, dc1);
                if (!hitC){
                    cache_access2(indexC2, tagC2, dc2);
                }
                }
          }
    }
    access_count1 = hit_count1 + miss_count1;
    access_count2 = hit_count2 + miss_count2;
    //cout << "Hello world!" << endl;
    cout << "Access1: " << access_count1 <<endl;
    cout << "Hit1: " << hit_count1 <<endl;
    cout << "Miss1: " << miss_count1 <<endl;
    cout << "Eviction1: " << evict_count1 << endl;

    cout << "Access2: " << access_count2 <<endl;
    cout << "Hit2: " << hit_count2 <<endl;
    cout << "Miss2: " << miss_count2 <<endl;
    cout << "Eviction2: " << evict_count2 << endl;

    cout << "valid_cacheline1: " << valid_cacheline1 << endl;
    cout << "valid_cacheline2: " << valid_cacheline2 << endl;

    int hit_time_l1 = 2;
    int hit_time_l2 = 10;
    int miss_penalty_l2 = 256; //DRAM access
    double miss_rate_l1 = (double)miss_count1 / access_count1;
    double miss_rate_l2 = (double)miss_count2 / access_count2;
    double miss_penalty_l1 = (double)hit_time_l2 + miss_rate_l2 * miss_penalty_l2;
    double AMAT = hit_time_l1 + miss_rate_l1 * miss_penalty_l1;
    cout << "Miss rate L1:\t" << miss_rate_l1 << endl;
    cout << "Miss rate L2:\t" << miss_rate_l2 <<  endl;
    cout << "AMAT:\t" << AMAT << endl;

    return 0;
}
