#include "../../Index/UnboundedLCR/NeighbourExchange.cc"
#include "../../Index/UnboundedLCR/DoubleBFS.cc"
#include "../../Index/UnboundedLCR/ClusteredExactIndex.cc"
#include "../../Graph/DGraph.cc"
#include <string>

using namespace std;
using namespace graphns;
using namespace indexns;

void test(bool a, int& t)
{
    if( a == true )
    {
        cout << "testID=" << to_string(t) << " passed" << endl;
    }
    else
    {
        cout << "testID=" << to_string(t) << " failed" << endl;
    }

    t++;
}

int main(int argc, char *argv[])
{
    Graph* g1 = new DGraph("tests/graphs/testGraph5.edge");
    ClusteredExactIndex cEI = ClusteredExactIndex(g1, 3);
    int t = 0;

    LabelSet ls = 0;
    ls = 6;
    test(cEI.query(0,10,ls) == true, t);

    ls = 5;
    test(cEI.query(10,2,ls) == true, t);

    ls = 7;
    test(cEI.query(7,9,ls) == true, t);

    ls = 3;
    test(cEI.query(0,5,ls) == true, t);

    ls = 1;
    test(cEI.query(2,6,ls) == true, t);

    ls = 1;
    test(cEI.query(2,4,ls) == false, t);

    ls = 3;
    test(cEI.query(0,9,ls) == false, t);

    ls = 2;
    test(cEI.query(0,3,ls) == false, t);

    cout << "-----" << endl;

    g1 = new DGraph("tests/graphs/testGraph8.edge");
    cEI = ClusteredExactIndex(g1, 3);

    ls = 1;
    test(cEI.query(1,2,ls) == true, t);

    ls = 1;
    test(cEI.query(0,11,ls) == false, t);

    ls = 3;
    test(cEI.query(0,7,ls) == false, t);

    cout << "-----" << endl;

    g1 = new DGraph("tests/graphs/testGraph6.edge");
    cEI = ClusteredExactIndex(g1, 3);

    ls = 64;
    test(cEI.query(0,10,ls) == false, t);

    ls = 17;
    test(cEI.query(0,6,ls) == true, t);

    ls = 3;
    test(cEI.query(89,96,ls) == false, t);

    ls = 3;
    test(cEI.query(9,50,ls) == true, t);

    ls = 6;
    test(cEI.query(0,60,ls) == false, t);

    cout << "-----" << endl;
    g1 = new DGraph("tests/graphs/testGraph7.edge");
    cEI = ClusteredExactIndex(g1, 2);

    ls = 36;
    test(cEI.query(0,102,ls) == true, t);

    ls = 104;
    test(cEI.query(18,68,ls) == true, t);

    cout << "-----" << endl;
    g1 = new DGraph("experiments/graphs/synthetic/L8/D5/1k/graphs/V1kD5L8uni.edge");
    cEI = ClusteredExactIndex(g1, 2);

    ls = 245;
    test(cEI.query(253,0,ls) == true, t);
    ls = 315;
    test(cEI.query(253,1,ls) == true, t);
    ls = 189;
    test(cEI.query(253,47,ls) == true, t);
    ls = 219;
    test(cEI.query(253,48,ls) == true, t);
    ls = 335;
    test(cEI.query(253,49,ls) == true, t);

    ls = 264;
    test(cEI.query(4,15,ls) == true, t);
    ls = 6;
    test(cEI.query(4,16,ls) == true, t);
    ls = 3;
    test(cEI.query(4,18,ls) == true, t);

    ls = 249;
    test(cEI.query(656,807,ls) == false, t);
    ls = 455;
    test(cEI.query(656,810,ls) == false, t);
    ls = 486;
    test(cEI.query(253,863,ls) == false, t);
    ls = 470;
    test(cEI.query(253,867,ls) == false, t);
    ls = 317;
    test(cEI.query(253,875,ls) == false, t);

    ls = 320;
    test(cEI.query(4,40,ls) == false, t);
    ls = 264;
    test(cEI.query(4,45,ls) == false, t);
    ls = 6;
    test(cEI.query(4,233,ls) == false, t);

    g1 = new DGraph("experiments/graphs/synthetic/L8/D2/10k/graphs/V10kD2L8uni.edge");
    cEI = ClusteredExactIndex(g1, 3);

    ls = 210;
    test(cEI.query(2178,1786,ls) == true, t);
    ls = 83;
    test(cEI.query(2178,1905,ls) == true, t);
    ls = 86;
    test(cEI.query(2178,2233,ls) == true, t);

    ls = 407;
    test(cEI.query(4159,293,ls) == false, t);
    ls = 235;
    test(cEI.query(4159,295,ls) == false, t);
    ls = 252;
    test(cEI.query(4159,313,ls) == false, t);

    return 0;
}
