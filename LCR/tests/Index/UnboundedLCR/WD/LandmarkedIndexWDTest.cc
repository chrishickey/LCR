#include "../../Index/UnboundedLCR/WD/LandmarkedIndexWD.cc"
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
    Graph* g1 = new DGraph("tests/graphs/tGraph1.edge");
    int t = 1;
    int N = g1->getNumberOfVertices();
    LandmarkedIndexWD* lI = new LandmarkedIndexWD(g1, 2 , 0 );

    test(lI->distanceQuery(0, 4, 1) == 4, t);
    test(lI->distanceQuery(0, 3, 2) == 2, t);
    test(lI->distanceQuery(0, 4, 3) == 3, t);
    test(lI->distanceQuery(0, 4, 8) == 1, t);

    test(lI->distanceQuery(7, 3, 2) == 3, t);

    test(lI->distanceQuery(4, 3, 3) > 10, t);

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    lI = new LandmarkedIndexWD(g1, 10 , 0 );

    g1 = new DGraph("tests/graphs/V2kD5L8exp.edge");
    lI = new LandmarkedIndexWD(g1, 20 , 0 );

    lI = new LandmarkedIndexWD(g1, 100 , 0 );
}
