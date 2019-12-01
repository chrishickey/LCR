#include "../../Index/UnboundedLCR/Zou/Zou.cc"
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

void runQuery(Index* index, VertexID v, VertexID w, LabelSet ls, bool answer, int& t)
{
    test(index->query(v,w,ls) == answer, t);
    cout << "time=" << index->getLastQueryTime() << endl;
}

int main(int argc, char *argv[])
{
    Graph* g1 = new DGraph("tests/graphs/ZouGraph.edge");
    int t = 1;
    Zou* zI = new Zou(g1);

    //cout << zI->toString() << endl;

    /*runQuery(zI, 0, 1, 1, true, t);
    runQuery(zI, 0, 2, 2, true, t);
    runQuery(zI, 0, 8, 2, true, t);
    runQuery(zI, 0, 7, 5, true, t);
    runQuery(zI, 4, 7, 1, true, t);
    runQuery(zI, 4, 6, 2, true, t);
    runQuery(zI, 1, 4, 4, true, t);
    runQuery(zI, 12, 11, 3, true, t);
    runQuery(zI, 12, 8, 2, true, t);

    runQuery(zI, 2, 0, 9, true, t);
    runQuery(zI, 3, 0, 3, true, t);
    runQuery(zI, 12, 0, 6, true, t);
    runQuery(zI, 12, 0, 34, true, t);

    runQuery(zI, 0, 4, 2, false, t);
    runQuery(zI, 4, 3, 7, false, t);
    runQuery(zI, 12, 11, 1, false, t);
    runQuery(zI, 4, 0, 34, false, t);
    runQuery(zI, 5, 0, 20, false, t);
    runQuery(zI, 8, 0, 5, false, t);
    runQuery(zI, 10, 0, 9, false, t);
    runQuery(zI, 11, 0, 17, false, t);

    cout << "-------------" << endl;*/

    g1 = new DGraph("tests/graphs/V30D3L8uni.edge");
    zI = new Zou(g1);

    runQuery(zI, 5, 13, 72, true, t);
    runQuery(zI, 6, 2, 68, true, t);
    runQuery(zI, 10, 2, 130, true, t);

    runQuery(zI, 11, 0, 129, false, t);
    runQuery(zI, 12, 0, 33, false, t);
    runQuery(zI, 13, 0, 65, false, t);

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    zI = new Zou(g1);

    runQuery(zI, 44, 0, 344, true, t);
    runQuery(zI, 48, 1, 337, true, t);
    runQuery(zI, 77, 1, 120, true, t);
    runQuery(zI, 98, 1, 275, true, t);

    runQuery(zI, 75, 6, 141, false, t);
    runQuery(zI, 83, 6, 393, false, t);
    runQuery(zI, 88, 6, 269, false, t);
    runQuery(zI, 97, 6, 393, false, t);

    g1 = new DGraph("experiments/graphs/synthetic/PA/uni/graphs/V1kD2L8uni.edge");
    zI = new Zou(g1);

    g1 = new DGraph("experiments/graphs/synthetic/PA/uni/graphs/V1kD5L8uni.edge");
    zI = new Zou(g1);

    g1 = new DGraph("experiments/graphs/synthetic/PA/uni/graphs/V5kD2L8uni.edge");
    zI = new Zou(g1);

    return 0;
}
