//#include "../../Graph/Graph.h"
#include "../Graph/DGraph.cc"

using namespace std;
using namespace graphns;

void test(bool a, int &testID)
{
    if( a == true )
    {
        cout << "testID=" << to_string(testID) << " passed" << endl;
    }
    else
    {
        cout << "testID=" << to_string(testID) << " failed" << endl;
    }

    testID++;
}

void allTests(Graph* mg, int& t)
{
    std::cout << mg->toString();

    test(mg->getNumberOfEdges() == 18, t);
    test(mg->getNumberOfVertices() == 13, t);
    test(mg->getNumberOfLabels() == 6, t);

    SmallEdgeSet edgeSet;
    mg->getOutNeighbours(0, edgeSet);
    test(edgeSet[0].first == 4, t);
    test(edgeSet[0].second == 1, t);
    test(edgeSet.size() == 1, t);

    mg->getOutNeighbours(9, edgeSet);
    test(edgeSet.size() == 2, t);
    test(edgeSet[0].first == 0 && edgeSet[1].first == 10, t);
    test(edgeSet[0].second == 1 && edgeSet[1].second == 1, t);

    mg->getInNeighbours(12, edgeSet);
    test(edgeSet[0].first == 10 && edgeSet[1].first == 11, t);
    test(edgeSet[0].second == 2 && edgeSet[1].second == 1, t);
    test(edgeSet.size() == 2, t);

    cout << "testing adding and removing nodes, edges and labels" << endl;
    mg->addNode();
    test(mg->getNumberOfVertices() == 14, t);
    mg->removeNode(4);
    test(mg->getNumberOfVertices() == 13, t);
    mg->getOutNeighbours(4, edgeSet);
    test(edgeSet[0].first == 6, t);
    test(edgeSet.size() == 1, t);

    mg->addEdge(11, 12, 1);
    mg->addEdge(11, 0, 1);
    mg->getOutNeighbours(11, edgeSet);
    test(edgeSet.size() == 2, t);
    mg->removeEdge(11,0);
    mg->getOutNeighbours(11, edgeSet);
    test(edgeSet.size() == 1, t);
    mg->changeLabel(11, 12, 2);
    mg->getOutNeighbours(11, edgeSet);
    test(edgeSet[0].second == 4, t);
}

int main(int argc, char *argv[])
{
    string s = "tests/graphs/testGraph1.edge";
    DGraph* mg = new DGraph(s);
    int t = 1;
    int L = mg->getNumberOfLabels();


    allTests(mg, t);

    cout << "- testing DGraph addNode, addEdge, hasEdge, removeEdge, changeLabel" << endl;
    EdgeSet* es = new EdgeSet();
    mg = new DGraph(es, 100, 8);
    mg->addNode();
    mg->addNode();
    mg->addNode();
    mg->addNode();
    mg->addNode();

    mg->addEdge(0,1,0);
    mg->addEdge(0,2,0);
    mg->addEdge(0,3,0);
    mg->addEdge(0,4,0);

    mg->removeEdge(0,1);
    mg->removeEdge(0,4);

    test(mg->hasEdge(0,3) == true, t);
    test(mg->hasEdge(0,2) == true, t);
    test(mg->hasEdge(0,4) == false, t);
    test(mg->hasEdge(0,1) == false, t);

    mg->removeEdge(0,2);
    mg->removeEdge(0,3);

    test(mg->hasEdge(0,2) == false, t);

    mg = new DGraph(es, 100, 8, true);
    mg->addEdge(0,4,0);
    mg->addEdge(0,4,0);

    cout << "testing graphns methods" << endl;

    LabelSet ls1 = 0;
    LabelSet ls2 = 1;
    test(isLabelSubset(ls1,ls2) == true, t); //1
    test(isLabelSubset(ls2,ls1) == false, t); //2

    ls1 = 3;
    test(isLabelSubset(ls1,ls2) == false, t); //3
    test(isLabelSubset(ls2,ls1) == true, t); //4

    test(isLabelEqual(ls1,ls2) == false, t); //5
    ls2 = 3;
    test(isLabelEqual(ls1,ls2) == true, t); //6

    test(getUnsignedChar(ls1,0) == 3, t); //7
    ls2 = 256;
    test(getUnsignedChar(ls2,0) == 0, t); //8
    test(getUnsignedChar(ls2,1) == 1, t); // 9

    ls2 = 255;
    test(getUnsignedChar(ls2,0) == 255, t); // 10

    ls2 = 0;
    setUnsignedChar(ls2,0,245);
    test(ls2 == 245, t);
    ls2 = 0;
    setUnsignedChar(ls2,1,1);
    test(ls2 == 256, t);

    ls2 = 255;
    test(getNumberOfLabelsInLabelSet(ls2) == 8, t);
    ls2 = 3;
    test(getNumberOfLabelsInLabelSet(ls2) == 2, t);
    ls2 = 0;
    test(getNumberOfLabelsInLabelSet(ls2) == 0, t);

    cout << "- labelSetToLabelID tests" << endl;
    ls1 = 1;
    test(labelSetToLabelID(ls1) == 0, t);
    ls1 = 2;
    test(labelSetToLabelID(ls1) == 1, t);
    ls1 = 4;
    test(labelSetToLabelID(ls1) == 2, t);
    ls1 = 8;
    test(labelSetToLabelID(ls1) == 3, t);

    ls1 = 1;
    test(labelSetToLetter(ls1) == "a", t);
    ls1 = 2;
    test(labelSetToLetter(ls1) == "b", t);
    ls1 = 8;
    test(labelSetToLetter(ls1) == "d", t);

    ls1 = 1;
    test(invertLabelSet(ls1, 8) == 254, t);

    test(invertLabelSet(ls1, 8) == 1, t);

    test(labelIDToLabelSet(0) == 1, t);
    test(labelIDToLabelSet(2) == 4, t);
    test(labelIDToLabelSet(3) == 8, t);

    ls1 = 1;
    ls2 = 0;
    test(joinLabelSets(ls1,ls2) == 1, t);
    ls2 = 3;
    test(joinLabelSets(ls1,ls2) == 3, t);
    test(intersectLabelSets(ls1,ls2) == 1, t);

    ls1 = 3;
    ls2 = 3;
    test(getHammingDistance(ls1,ls2) == 0, t);

    ls1 = 0;
    ls1= setJoinFlag(ls1);
    test(hasJoinFlag(ls1) == true, t);
    //cout << "ls1=" << labelSetToString(ls1) << endl;
    ls1 = removeJoinFlag(ls1);
    test(hasJoinFlag(ls1) == false, t);
    //cout << "ls1=" << labelSetToString(ls1) << endl;

    cout << "stats tests" << endl;
    DGraph* g1 = new DGraph("tests/graphs/testGraph10.edge");
    test(g1->computerNumberOfTriangles() == 2, t);

    //cout << "g1->computeClusterCoefficient()=" << g1->computeClusterCoefficient() << endl;
    vector< vector < VertexID > > sccs;
    g1 = new DGraph("tests/graphs/testGraph11.edge");
    g1->tarjan(sccs);
    test( sccs.size() == 4, t);
    test(g1->computeDiameter() == 7, t);
    //cout << "diameter=" << g1->computeDiameter() << endl;

    s = "tests/graphs/verysmallgraph.edge";
    g1 = new DGraph(s);
    test(g1->getGraphSizeInBytes() == 12*(sizeof(LabelSet)+sizeof(VertexID)), t );

    return 0;
}
