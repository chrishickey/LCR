#include "../../Index/UnboundedLCR/LandmarkedIndex.cc"
#include "../../Index/UnboundedLCR/BFSIndex.cc"
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
    cout << "query(" << v << "," << w << "," << ls << ") took time=" << index->getLastQueryTime() << endl;
}

void runQueryAll(Index* lI, Index* bfs, VertexID v, LabelSet ls, int N, int& t)
{
    dynamic_bitset<> bs1 = dynamic_bitset<>(N);
    dynamic_bitset<> bs2 = dynamic_bitset<>(N);

    bfs->queryAll(v,ls,bs1);
    lI->queryAll(v,ls,bs2);

    test(bs1.is_subset_of(bs2) && bs2.is_subset_of(bs1), t);
    cout << "queryAll (" << v << "," << ls << "), speed-up=" << (bfs->getLastQueryTime() / lI->getLastQueryTime() )
        << ",bs1.count()=" << bs1.count() << ",bs2.count()=" << bs2.count() << endl;
}

void testUpdates(LandmarkedIndex* lI, Graph* g1, int& t, int& N)
{
    cout << "---\nLandmarkedIndex updates\n---" << endl;
    cout << "* Add edge" << endl;

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    lI = new LandmarkedIndex(g1, 10 , 1, 0, true, 5);
    runQuery(lI,27,3,26,false,t);
    runQuery(lI,39,3,324,false,t);
    runQuery(lI,48,3,22,false,t);
    runQuery(lI,58,0,328,false,t);

    runQuery(lI,10,5,420,true,t);
    runQuery(lI,16,0,15,true,t);
    runQuery(lI,34,1,353,true,t);
    runQuery(lI,45,0,43,true,t);

    // adding edges to V100D5L8uni , which now matches V100D5L8WithExtraEdges
    lI->addEdge(0, 90, 0);
    lI->addEdge(0, 91, 1);
    lI->addEdge(0, 92, 2);
    lI->addEdge(0, 98, 0);
    lI->addEdge(1, 2, 2);
    lI->addEdge(1, 6, 3);
    lI->addEdge(1, 10, 0);
    lI->addEdge(6, 4, 7);
    lI->addEdge(7, 6, 0);

    cout << "*- true queries withExtraEdges" << endl;
    runQuery(lI, 0, 1 , 133 ,true,t);
    runQuery(lI, 1, 0 , 321 ,true,t);
    runQuery(lI, 2, 3 , 176 ,true,t);
    runQuery(lI, 6, 0 , 11 ,true,t);
    runQuery(lI, 8, 0 , 14 ,true,t);
    runQuery(lI, 8, 0 , 50 ,true,t);
    runQuery(lI, 18, 7 , 13 ,true,t);
    runQuery(lI, 43, 0 , 37 ,true,t);
    runQuery(lI, 43, 1 , 69 ,true,t);
    runQuery(lI, 45, 0 , 97 ,true,t);
    runQuery(lI, 45, 1 , 131 ,true,t);
    runQuery(lI, 46, 0 , 13 ,true,t);
    runQuery(lI, 53, 0 , 35 ,true,t);
    runQuery(lI, 54, 3 , 176 ,true,t);
    runQuery(lI, 74, 0 , 140 ,true,t);
    runQuery(lI, 87, 0 , 324 ,true,t);
    runQuery(lI, 87, 8 , 164 ,true,t);
    runQuery(lI, 90, 0 , 97 ,true,t);
    runQuery(lI, 91, 0 , 322 ,true,t);
    runQuery(lI, 94, 1 , 11 ,true,t);

    cout << "*- false queries withExtraEdges" << endl;
    runQuery(lI, 2, 3 , 146 ,false,t);
    runQuery(lI, 4, 6 , 296 ,false,t);
    runQuery(lI, 6, 3 , 322 ,false,t);
    runQuery(lI, 16, 4 , 352 ,false,t);
    runQuery(lI, 17, 4 , 296 ,false,t);
    runQuery(lI, 20, 0 , 400 ,false,t);
    runQuery(lI, 21, 3 , 74 ,false,t);
    runQuery(lI, 31, 7 , 328 ,false,t);
    runQuery(lI, 40, 0 , 392 ,false,t);
    runQuery(lI, 42, 5 , 259 ,false,t);
    runQuery(lI, 46, 7 , 265 ,false,t);
    runQuery(lI, 50, 8 , 273 ,false,t);
    runQuery(lI, 54, 1 , 266 ,false,t);
    runQuery(lI, 61, 7 , 81 ,false,t);
    runQuery(lI, 64, 8 , 259 ,false,t);
    runQuery(lI, 71, 3 , 146 ,false,t);
    runQuery(lI, 79, 3 , 74 ,false,t);
    runQuery(lI, 95, 3 , 70 ,false,t);
    runQuery(lI, 97, 3 , 276 ,false,t);
    runQuery(lI, 97, 5 , 266 ,false,t);

    // removing edge
    cout << "* Remove edge" << endl;
    lI = new LandmarkedIndex(g1, 10 , 1, 0, true, 5); // rebuild
    lI->removeEdge(0, 3, 0);
    lI->removeEdge(0, 9, 7);
    lI->removeEdge(2, 0, 1);
    lI->removeEdge(2, 1, 1);
    lI->removeEdge(4, 0, 1);
    lI->removeEdge(4, 2, 0);
    lI->removeEdge(5, 0, 1);
    lI->removeEdge(5, 2, 6);
    lI->removeEdge(6, 0, 7);
    lI->removeEdge(6, 1, 4);

    cout << "* Remove edge True-queries" << endl;
    runQuery(lI, 0, 1 , 13 ,true,t);
    runQuery(lI, 6, 0 , 81 ,true,t);
    runQuery(lI, 21, 0 , 13 ,true,t);
    runQuery(lI, 23, 0 , 38 ,true,t);
    runQuery(lI, 28, 0 , 56 ,true,t);
    runQuery(lI, 30, 0 , 112 ,true,t);
    runQuery(lI, 33, 5 , 162 ,true,t);
    runQuery(lI, 38, 5 , 82 ,true,t);
    runQuery(lI, 40, 1 , 137 ,true,t);
    runQuery(lI, 54, 0 , 176 ,true,t);
    runQuery(lI, 60, 0 , 69 ,true,t);
    runQuery(lI, 65, 1 , 97 ,true,t);
    runQuery(lI, 66, 2 , 13 ,true,t);
    runQuery(lI, 67, 0 , 112 ,true,t);
    runQuery(lI, 70, 1 , 131 ,true,t);
    runQuery(lI, 73, 0 , 98 ,true,t);
    runQuery(lI, 84, 0 , 52 ,true,t);
    runQuery(lI, 86, 5 , 76 ,true,t);
    runQuery(lI, 93, 5 , 41 ,true,t);
    runQuery(lI, 95, 2 , 176 ,true,t);

    cout << "* Remove edge False-queries" << endl;
    runQuery(lI, 4, 2 , 273 ,false,t);
    runQuery(lI, 11, 6 , 289 ,false,t);
    runQuery(lI, 16, 4 , 104 ,false,t);
    runQuery(lI, 25, 0 , 266 ,false,t);
    runQuery(lI, 41, 3 , 385 ,false,t);
    runQuery(lI, 45, 3 , 19 ,false,t);
    runQuery(lI, 45, 7 , 385 ,false,t);
    runQuery(lI, 47, 4 , 304 ,false,t);
    runQuery(lI, 53, 2 , 21 ,false,t);
    runQuery(lI, 59, 7 , 73 ,false,t);
    runQuery(lI, 62, 0 , 400 ,false,t);
    runQuery(lI, 68, 1 , 50 ,false,t);
    runQuery(lI, 69, 3 , 73 ,false,t);
    runQuery(lI, 76, 8 , 448 ,false,t);
    runQuery(lI, 82, 4 , 352 ,false,t);
    runQuery(lI, 83, 3 , 74 ,false,t);
    runQuery(lI, 86, 6 , 388 ,false,t);
    runQuery(lI, 95, 3 , 21 ,false,t);
    runQuery(lI, 97, 3 , 26 ,false,t);
    runQuery(lI, 99, 3 , 296 ,false,t);

    // removing node

    // changing label
}

int main(int argc, char *argv[])
{
    Graph* g1 = new DGraph("tests/graphs/testGraph3.edge");
    int t = 1;
    int N;
    LandmarkedIndex* lI = new LandmarkedIndex(g1, 3 , 0 , 0);

    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    N = g1->getNumberOfVertices();
    int k = N/50, l = 10;
    lI = new LandmarkedIndex(g1, k , 1, 0, true, l, 0, true);
    lI = new LandmarkedIndex(g1, k , 1, 0, true, l, 0, false);

    g1 = new DGraph("tests/graphs/V2kD5L8exp.edge");
    N = g1->getNumberOfVertices();
    k = N/50; l = 15;
    lI = new LandmarkedIndex(g1, k , 1, 0, true, l, 0, true);
    lI = new LandmarkedIndex(g1, k , 1, 0, true, l, 0, false);

    /*runQuery(lI, 0, 1, 1, true, t);
    runQuery(lI, 0, 7, 3, true, t);
    runQuery(lI, 0, 16, 3, true, t);
    runQuery(lI, 7, 9, 1, true, t);
    runQuery(lI, 7, 0, 3, true, t);
    runQuery(lI, 0, 23, 1, true, t);

    runQuery(lI, 0, 1, 2, false, t);
    runQuery(lI, 7, 0, 1, false, t);
    runQuery(lI, 9, 5, 4, false, t);

    //cout << lI->toString() << endl;

    lI = new LandmarkedIndex(g1, 3 , 1 , 0);
    lI = new LandmarkedIndex(g1, 3 , 2 , 0);
    lI = new LandmarkedIndex(g1, 3 , 3 , 0);
    lI = new LandmarkedIndex(g1, 3 , 4 , 0);

    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    lI = new LandmarkedIndex(g1);

    runQuery(lI, 5, 7, 129, true, t);
    runQuery(lI, 66, 4, 130, true, t);
    runQuery(lI, 178, 0, 160, true, t);
    runQuery(lI, 265, 0, 129, true, t);

    runQuery(lI, 139, 36, 144, false, t);
    runQuery(lI, 256, 7, 136, false, t);
    runQuery(lI, 280, 64, 132, false, t);
    runQuery(lI, 282, 64, 144, false, t);

    g1 = new DGraph("tests/graphs/V5kD5L8exp.edge");
    lI = new LandmarkedIndex(g1);

    cout << "---\nLandmarkedIndexTest others\n---" << endl;
    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    lI = new LandmarkedIndex(g1, 100 , 1 , 0, true, 50);

    runQuery(lI, 5, 7, 129, true, t);
    runQuery(lI, 66, 4, 130, true, t);
    runQuery(lI, 178, 0, 160, true, t);
    runQuery(lI, 265, 0, 129, true, t);

    runQuery(lI, 139, 36, 144, false, t);
    runQuery(lI, 256, 7, 136, false, t);
    runQuery(lI, 280, 64, 132, false, t);
    runQuery(lI, 282, 64, 144, false, t);*/

    /*cout << "---\nLandmarkedIndexTest with extensiveQuery\n---" << endl;
    lI = new LandmarkedIndex(g1, 0 , 1 , 0, true, 100, 10, true);

    runQuery(lI, 7, 11, 160, true, t);
    runQuery(lI, 436, 45, 130, true, t);

    runQuery(lI, 8, 0, 3, false, t);
    runQuery(lI, 57, 2, 3, false, t);
    runQuery(lI, 880, 68, 5, false, t);
    runQuery(lI, 1017, 41, 36, false, t);

    testUpdates(lI, g1, t, N);

    // queryAll
    cout << "queryAll tests" << endl;
    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    lI = new LandmarkedIndex(g1, 10 , 1, 0, true, 5); // rebuild
    N = g1->getNumberOfVertices();
    Index* bfs = new BFSIndex(g1);
    //runQueryAll(Index* lI, Index* bfs, VertexID v, LabelSet ls, int N, int& t)
    // 2 labels
    runQueryAll(lI, bfs, 0, 3, N, t );
    runQueryAll(lI, bfs, 1, 6, N, t );
    runQueryAll(lI, bfs, 2, 130, N, t );

    // 3 labels
    runQueryAll(lI, bfs, 0, 7, N, t );
    runQueryAll(lI, bfs, 1, 14, N, t );
    runQueryAll(lI, bfs, 2, 134, N, t );*/

    return 0;
}
