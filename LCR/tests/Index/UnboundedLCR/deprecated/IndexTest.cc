#include "../../Index/UnboundedLCR/NeighbourExchange.cc"
#include "../../Index/UnboundedLCR/DoubleBFS.cc"
#include "../../Index/UnboundedLCR/BFSIndex.cc"
#include "../../Index/UnboundedLCR/LandmarkedIndex.cc"
#include "../../Index/UnboundedLCR/ClusteredExactIndex.cc"
#include "../../Index/UnboundedLCR/PartialIndex.cc"

#include "../../Graph/DGraph.cc"

#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace graphns;
using namespace indexns;

/**
* This file test all indexes that build a full exact index: NeighbourExchange and
* DoubleBFS
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

int main(int argc, char *argv[])
{
    int t = 1;
    DGraph* graph;

    // test index namespace methods
    LabelSets lss;
    test(labelSetInLabelSets(0,lss) == false, t);
    tryInsertLabelSet(1, lss);
    test(labelSetInLabelSets(0,lss) == false, t);
    test(labelSetInLabelSets(1,lss) == true, t);
    tryInsertLabelSet(2, lss);
    tryInsertLabelSet(3, lss);
    test(labelSetInLabelSets(2,lss) == true, t);
    test(labelSetInLabelSets(3,lss) == true, t); // true because 1 is in there
    test(find(lss.begin(), lss.end(), 3) == lss.end(), t); // false because 3 is not literally in there

    test(isMinimalLabelSets(lss) == true, t);
    lss.push_back(3);
    test(isMinimalLabelSets(lss) == false, t);

    Tuples tuples;
    LabelSets lss1, lss2;
    Tuple tu1 = make_pair(0, lss1);
    Tuple tu2 = make_pair(3, lss2);
    tuples.push_back( tu1 );
    tuples.push_back( tu2 );
    tuples.push_back( make_pair(5, LabelSets() ) );
    test(findTupleInTuples(0, tuples) == 0, t);
    test(findTupleInTuples(1, tuples) == 1, t);
    test(findTupleInTuples(3, tuples) == 1, t);
    test(findTupleInTuples(4, tuples) == 2, t);
    test(findTupleInTuples(5, tuples) == 2, t);
    test(findTupleInTuples(6, tuples) == 3, t);

    test(tupleExists(0, tuples, 0) == true, t);
    test(tupleExists(0, tuples, 1) == false, t);
    test(tupleExists(3, tuples, 1) == true, t);
    test(tupleExists(5, tuples, 3) == false, t);

    // test indexes individually
    vector< QueryWithAnswer > a;

    graph = new DGraph("tests/graphs/testGraph7.edge");
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
    a.push_back( make_pair(make_pair(make_pair(66,157),4), false) );

    for(int i = 0; i < 6; i++)
    {
        Index* index;

        if( i == 0 )
            index = new BFSIndex(graph);
        if( i == 1 )
            index = new DoubleBFS(graph);
        if( i == 2 )
            index = new PartialIndex(graph);
        if( i == 3 )
            index = new ClusteredExactIndex(graph);
        if( i == 4 )
            index = new NeighbourExchange(graph);
        if( i == 5 )
            index = new LandmarkedIndex(graph);

        cout << "i=" << i << ",index type=" << index->getIndexTypeAsString() << endl;
        testByIndex(t, index, a);
    }

    return 0;
}
