#include "../../Index/BoundedLCR/BoundedIndex.h"
#include "../../Index/BoundedLCR/BoundedBFS.cc"
#include "../../Graph/DGraph.cc"

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace graphns;
using namespace boundedindexns;


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
    int t = 1;
    DGraph* ng = new DGraph("tests/graphs/V100D5L8uni.edge");
    int L = ng->getNumberOfLabels();
    BoundedIndex* bbfs = new BoundedBFS(ng);
    B12Query q;
    Budget b1, b2;

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[4] = 1;
    q = generateB12Query(1,0, b1, b2);
    test(bbfs->query(q) == true, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[4] = 1;
    b2[7] = 1;
    q = generateB12Query(1,9, b1, b2);
    test(bbfs->query(q) == true, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[0] = 2;
    q = generateB12Query(1,9, b1, b2);
    test(bbfs->query(q) == false, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[1] = 2;
    q = generateB12Query(1,9, b1, b2);
    test(bbfs->query(q) == false, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[0] = 3;
    q = generateB12Query(3,1, b1, b2);
    test(bbfs->query(q) == true, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b2[6] = 3;
    q = generateB12Query(3,97, b1, b2);
    test(bbfs->query(q) == false, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b1[1] = 2;
    b2[1] = 2;
    q = generateB12Query(50,40, b1, b2);
    test(bbfs->query(q) == true, t);

    b1 = Budget(L, 0);
    b2 = Budget(L, 0);
    b1[1] = 3;
    b2[1] = 4;
    q = generateB12Query(50,40, b1, b2);
    test(bbfs->query(q) == false, t);

    return 0;
}
