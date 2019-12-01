#include "../../Index/UnboundedLCR/PartialIndex.cc"
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
    Graph* g1 = new DGraph("tests/graphs/testGraph3.edge");
    int t = 1;
    PartialIndex* pI = new PartialIndex(g1);

    runQuery(pI, 0, 1, 1, true, t);
    runQuery(pI, 0, 7, 3, true, t);
    runQuery(pI, 0, 16, 3, true, t);

    runQuery(pI, 0, 1, 2, false, t);

    runQuery(pI, 7, 9, 1, true, t);
    runQuery(pI, 7, 0, 3, true, t);

    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    pI = new PartialIndex(g1);

    runQuery(pI, 5, 7, 129, true, t);
    runQuery(pI, 66, 4, 130, true, t);
    runQuery(pI, 178, 0, 160, true, t);
    runQuery(pI, 265, 0, 129, true, t);

    runQuery(pI, 139, 36, 144, false, t);
    runQuery(pI, 256, 7, 136, false, t);
    runQuery(pI, 280, 64, 132, false, t);
    runQuery(pI, 282, 64, 144, false, t);

    pI = new PartialIndex(g1, false);
    runQuery(pI, 5, 7, 129, true, t);
    runQuery(pI, 66, 4, 130, true, t);
    runQuery(pI, 178, 0, 160, true, t);
    runQuery(pI, 265, 0, 129, true, t);

    runQuery(pI, 139, 36, 144, false, t);
    runQuery(pI, 256, 7, 136, false, t);
    runQuery(pI, 280, 64, 132, false, t);
    runQuery(pI, 282, 64, 144, false, t);

    g1 = new DGraph("tests/graphs/V5kD5L8exp.edge");
    pI = new PartialIndex(g1);

    return 0;
}
