#include "../../Index/UnboundedLCR/BFSIndex.cc"

#include "../../Graph/DGraph.cc"

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace graphns;
using namespace indexns;

/**
* This file tests DoubleBFS
**/

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

void runQueryAll(Index* index, VertexID v, LabelSet ls, dynamic_bitset<>& answer, int N, int& t)
{
    dynamic_bitset<> marked = dynamic_bitset<>(N);
    index->queryAll(v, ls, marked);
    test( marked.is_subset_of(answer) && answer.is_subset_of(marked), t );
    cout << "time=" << index->getLastQueryTime() << endl;
}

int main(int argc, char *argv[])
{
    int t = 1;
    DGraph* g1;
    Index* bfs;
    int N;

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    bfs = new BFSIndex(g1);
    N = g1->getNumberOfVertices();

    runQuery(bfs, 44, 0, 344, true, t);
    runQuery(bfs, 48, 1, 337, true, t);
    runQuery(bfs, 77, 1, 120, true, t);
    runQuery(bfs, 98, 1, 275, true, t);

    runQuery(bfs, 75, 6, 141, false, t);
    runQuery(bfs, 83, 6, 393, false, t);
    runQuery(bfs, 88, 6, 269, false, t);
    runQuery(bfs, 97, 6, 393, false, t);

    g1 = new DGraph("tests/graphs/tGraph0.edge");
    bfs = new BFSIndex(g1);
    N = g1->getNumberOfVertices();

    dynamic_bitset<> answer = dynamic_bitset<>(N);
    answer[0] = 1; answer[1] = 1;
    runQueryAll(bfs, 0,4, answer,N,t);
    answer.reset();
    answer[0] = 1; answer[1] = 1; answer[2] = 1; answer[3] = 1; answer[4] = 1; answer[5] = 1;
    runQueryAll(bfs, 0,5, answer,N,t);

    return 1;
}
