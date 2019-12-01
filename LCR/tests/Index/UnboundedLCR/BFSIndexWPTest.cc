#include "../../../Graph/Graph.h"
#include "../../../Graph/DGraph.cc"
#include "../../../Index/UnboundedLCR/IndexWP.h"
#include "../../../Index/UnboundedLCR/BFSIndexWP.cc"
#include "../../../Index/UnboundedLCR/BFSIndex.cc"

using namespace graphns;
using namespace indexwpns;

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

void runQuery(IndexWP* index, VertexID v, VertexID w, LabelSet ls, int size, int& t)
{
    Paths ps = Paths();
    index->queryWP(v,w,ls,ps);
    test(ps.size() == size, t);
    cout << "time=" << index->getLastQueryTime() << ", ps.size()=" << ps.size() << endl;
}

int main(int argc, char *argv[])
{
    DGraph* ng = new DGraph("tests/graphs/V100D5L8uni.edge");
    BFSIndexWP* bfswp = new BFSIndexWP(ng);
    int t = 1;

    runQuery(bfswp, 10, 0, 45, 1, t);
    runQuery(bfswp, 25, 13, 390, 1, t);
    runQuery(bfswp, 28, 1, 53, 1, t);
    runQuery(bfswp, 55, 6, 216, 1, t);
    runQuery(bfswp, 64, 0, 172, 1, t);

    runQuery(bfswp, 5, 13, 432, 0, t);
    runQuery(bfswp, 10, 3, 325, 0, t);
    runQuery(bfswp, 22, 10, 71, 0, t);
    runQuery(bfswp, 31, 8, 275, 0, t);

    return 0;
}
