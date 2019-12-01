#include "../../Index/UnboundedLCR/DoubleBFS.cc"
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

void testByIndex(int& t, Index* eI2, vector<QueryWithAnswer>& a)
{
    cout << "testByIndex: indexType=" << eI2->getIndexTypeAsString();

    for(int i = 0; i < a.size(); i++)
    {
        runQuery(eI2, a[i].first.first.first, a[i].first.first.second, a[i].first.second, a[i].second, t);
    }

    cout << "---" << endl;
}

void testAddAndRemoveEdge(DoubleBFS* dbfs, VertexID v, VertexID w, LabelSet ls, VertexID v2,
    VertexID w2, LabelID lID, int& t)
{
    cout << "testAddAndRemoveEdge: t=" << t << endl;
    long oldSize = dbfs->getIndexSizeInBytes();
    runQuery(dbfs, v, w, ls, false, t);
    dbfs->addEdge(v2, w2, lID);
    runQuery(dbfs, v, w, ls, true, t);
    dbfs->removeEdge(v2, w2);
    runQuery(dbfs, v, w, ls, false, t);
    test(oldSize == dbfs->getIndexSizeInBytes(), t); // should be the same
}

void testChangeAndUnchangeLabel(DoubleBFS* dbfs, VertexID v, VertexID w, LabelID oldlID, LabelID newlID, VertexID vx, VertexID wx, LabelSet lIDx, bool answerX, int& t)
{
    // changing the label of (v,w) to lID should change the answer of the query
    cout << "testChangeAndUnchangeLabel: t=" << t << endl;
    long oldSize = dbfs->getIndexSizeInBytes();
    runQuery(dbfs, vx, wx, lIDx, answerX, t);
    dbfs->changeLabel(v, w, newlID);
    runQuery(dbfs, vx, wx, lIDx, !answerX, t);
    dbfs->changeLabel(v, w, oldlID);
    runQuery(dbfs, vx, wx, lIDx, answerX, t);
    test(oldSize == dbfs->getIndexSizeInBytes(), t); // should be the same
}

int main(int argc, char *argv[])
{
    int t = 1;
    DGraph* g1;
    Index* eI2;
    vector< QueryWithAnswer > a;
    long size;
    int sV = sizeof(VertexID);
    int sL = sizeof(LabelSet);

    // test the size
    g1 = new DGraph("tests/graphs/verysmallgraph.edge");
    eI2 = new DoubleBFS(g1);
    size = eI2->getIndexSizeInBytes();
    test(size == eI2->getIndexSizeInBytes(), t);
    test(eI2->isMinimalIndex() == true, t);

    eI2 = new DoubleBFS(g1);

    test(eI2->getIndexSizeInBytes() == size, t); // the two should have the same size
    test(eI2->isMinimalIndex() == true, t);

    g1 = new DGraph("tests/graphs/testGraph7.edge");
    a.push_back( make_pair(make_pair(make_pair(2,11),136), true) );
    a.push_back( make_pair(make_pair(make_pair(2,10),74), true) );
    a.push_back( make_pair(make_pair(make_pair(0,11),162), true) );
    a.push_back( make_pair(make_pair(make_pair(1,17),136), true) );
    a.push_back( make_pair(make_pair(make_pair(1,17),144), true) );
    a.push_back( make_pair(make_pair(make_pair(1,21),96), true) );
    a.push_back( make_pair(make_pair(make_pair(66,157),3), true) );

    a.push_back( make_pair(make_pair(make_pair(2,11),1), false) );
    a.push_back( make_pair(make_pair(make_pair(1,56),88), false) );
    a.push_back( make_pair(make_pair(make_pair(1,57),76), false) );
    a.push_back( make_pair(make_pair(make_pair(3,20),192), false) );
    a.push_back( make_pair(make_pair(make_pair(3,38),9), false) );
    a.push_back( make_pair(make_pair(make_pair(3,19),130), false) );

    eI2 = new DoubleBFS(g1);
    test(eI2->isMinimalIndex() == true, t);
    testByIndex(t, eI2, a);

    cout << "comparing blocked and non-blocked mode" << endl;

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    eI2 = new DoubleBFS(g1);

    runQuery(eI2, 44, 0, 344, true, t);
    runQuery(eI2, 48, 1, 337, true, t);
    runQuery(eI2, 77, 1, 120, true, t);
    runQuery(eI2, 98, 1, 275, true, t);

    runQuery(eI2, 75, 6, 141, false, t);
    runQuery(eI2, 83, 6, 393, false, t);
    runQuery(eI2, 88, 6, 269, false, t);
    runQuery(eI2, 97, 6, 393, false, t);

    eI2 = new DoubleBFS(g1, false);
    runQuery(eI2, 44, 0, 344, true, t);
    runQuery(eI2, 48, 1, 337, true, t);
    runQuery(eI2, 77, 1, 120, true, t);
    runQuery(eI2, 98, 1, 275, true, t);

    runQuery(eI2, 75, 6, 141, false, t);
    runQuery(eI2, 83, 6, 393, false, t);
    runQuery(eI2, 88, 6, 269, false, t);
    runQuery(eI2, 97, 6, 393, false, t);

    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    eI2 = new DoubleBFS(g1);

    runQuery(eI2, 5, 7, 129, true, t);
    runQuery(eI2, 66, 4, 130, true, t);
    runQuery(eI2, 178, 0, 160, true, t);
    runQuery(eI2, 265, 0, 129, true, t);

    runQuery(eI2, 139, 36, 144, false, t);
    runQuery(eI2, 256, 7, 136, false, t);
    runQuery(eI2, 280, 64, 132, false, t);
    runQuery(eI2, 282, 64, 144, false, t);

    eI2 = new DoubleBFS(g1, false);

    runQuery(eI2, 5, 7, 129, true, t);
    runQuery(eI2, 66, 4, 130, true, t);
    runQuery(eI2, 178, 0, 160, true, t);
    runQuery(eI2, 265, 0, 129, true, t);

    runQuery(eI2, 139, 36, 144, false, t);
    runQuery(eI2, 256, 7, 136, false, t);
    runQuery(eI2, 280, 64, 132, false, t);
    runQuery(eI2, 282, 64, 144, false, t);

    g1 = new DGraph("tests/graphs/V1kD3L8uni.edge");
    eI2 = new DoubleBFS(g1);

    eI2 = new DoubleBFS(g1, false);

    g1 = new DGraph("experiments/graphs/synthetic/PA/uni/graphs/V1kD2L8uni.edge");
    eI2 = new DoubleBFS(g1);

    g1 = new DGraph("experiments/graphs/synthetic/PA/uni/graphs/V1kD5L8uni.edge");
    eI2 = new DoubleBFS(g1);

    cout << "heap-tests" << endl;
    g1 = new DGraph("tests/graphs/V1kD3L8exp.edge");
    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, true); // with heap

    runQuery(eI2, 560, 5, 129, true, t);
    runQuery(eI2, 567, 2, 160, true, t);
    runQuery(eI2, 716, 5, 160, true, t);

    runQuery(eI2, 717, 36, 132, false, t);
    runQuery(eI2, 768, 83, 129, false, t);
    runQuery(eI2, 891, 36, 136, false, t);

    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, false); // without heap

    runQuery(eI2, 560, 5, 129, true, t);
    runQuery(eI2, 567, 2, 160, true, t);
    runQuery(eI2, 716, 5, 160, true, t);

    runQuery(eI2, 717, 36, 132, false, t);
    runQuery(eI2, 768, 83, 129, false, t);
    runQuery(eI2, 891, 36, 136, false, t);

    g1 = new DGraph("tests/graphs/V2kD3L8exp.edge");
    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, true); // with heap

    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, false); // without heap

    g1 = new DGraph("tests/graphs/V2kD5L8exp.edge");
    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, true); // with heap

    eI2 = new DoubleBFS(g1, vector< VertexID >(), false, false); // without heap


    return 0;
}
