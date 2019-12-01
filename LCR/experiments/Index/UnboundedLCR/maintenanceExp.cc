#include <cstdlib>
#include <sstream>
#include <vector>
#include <random>
#include <queue>
#include <algorithm>
#include <string>
#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <unistd.h>
#include <sys/time.h>

#include <iostream>
#include <chrono>

#include "../../Index/UnboundedLCR/Index.h"
#include "../../Index/UnboundedLCR/BFSIndex.cc"
#include "../../Index/UnboundedLCR/LandmarkedIndex.cc"
#include "../../Graph/DGraph.cc"

using namespace std;
using namespace indexns;
using namespace graphns;

LabelSet generateLabelSet(LabelSet& ls, int nK, int L, std::uniform_int_distribution<int>& distribution, std::default_random_engine& generator)
{
    /*std::this_thread::sleep_for(std::chrono::milliseconds(1));
    double t = fmod(getCurrentTimeInMilliSec()*100000,10000000);
    int s = (int) round(t);

    unsigned int seed = (s) % 10000000;
    //cout << "t=" << t << ",seconds=" << s << ",seed=" << seed << endl;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, L);*/

    int nL = getNumberOfLabelsInLabelSet(ls);

    while(nL != nK)
    {
        int j = distribution(generator);

        if( nL < nK )
        {
            setLabelInLabelSet(ls, j , 1);
        }

        if( nL > nK )
        {
            setLabelInLabelSet(ls, j , 0);
        }

        nL = getNumberOfLabelsInLabelSet(ls);
        //cout << "nL1 > nK j=" << j << ",nL1=" << nL1 << ",ls2=" << labelSetToString(ls2) << endl;
    }

    return ls;
}

int testIndices(Index* ind1, Index* ind2, std::uniform_int_distribution<VertexID>& vertexDist,
std::default_random_engine& generator, int L)
{
    unsigned int seed = (time(NULL)*time(NULL)*time(NULL)) % 1000;
    std::default_random_engine generator2(seed);
    std::uniform_int_distribution<int> labelDistribution(0, L - 1);

    int tries = 200;
    for(int j = 0; j < tries; j++)
    {
        VertexID s = vertexDist(generator); // a random start point
        VertexID t = vertexDist(generator); // a random start point

        LabelSet ls = 0;
        ls = generateLabelSet(ls, L/4 + 1, L, labelDistribution, generator2);
        bool b1 = ind1->query(s,t,ls);
        bool b2 = ind2->query(s,t,ls);

        if( b1 != b2 )
        {
            cout << "query " << j << " failed: " << s << "," << t << "," << ls << "," << b1 << "," << b2 << endl;
            return 1;
        }
    }

    cout << "all " << tries << " tests passed " << endl;

    return 0;
}

/*
maintenanceExp is an experiment to test the speed and correctness of updates on
the index.
*/
int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        cout << "./maintenanceExp <edge_file>" << endl;
        return 1;
    }


    string edge_file = argv[1]; // the file containing the graph
    DGraph* dg = new DGraph(edge_file);
    DGraph* dgc = new DGraph(edge_file);


    auto start = std::chrono::high_resolution_clock::now();

    // operation to be timed ...
    for(int i = 0; i < 3; i++)
    {
        int j = 0;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    //std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(finish-start).count() << "ns\n";
    long dur = (finish-start).count();
    cout << dur << endl;

    int N = dg->getNumberOfVertices();
    int L = dg->getNumberOfLabels();

    if( L < 3 || N < 100 )
    {
        cout << "L or N too small" << endl;
        return 1;
    }

    int K = 30;
    int Nc = N + K;
    int Lc = L + K;

    int k = N/50; int l = 0;
    LandmarkedIndex* lI = new LandmarkedIndex(dg, k , 1, 0, true, l);
    LandmarkedIndex* lI2;

    double timelI = lI->getIndexConstructionTimeInSec();
    cout << "addEdge time(s)=" << timelI << endl;

    unsigned int seed = time(NULL)*time(NULL) % 1000;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

    seed = (time(NULL)*time(NULL)*time(NULL)) % 1000;
    std::default_random_engine generator2(seed);
    std::uniform_int_distribution<int> labelDistribution(0, L - 1);

    /*// addition of edge
    cout << "addition of edges" << endl;
    for(int i = 0; i < K; )
    {
        VertexID s = vertexDistribution(generator); // a random vertex
        VertexID t = vertexDistribution(generator); // a random vertex
        LabelID l = labelDistribution(generator2);

        if( dgc->hasEdge(s,t) == false )
        {
            dgc->addEdge(s,t,l);
            lI->addEdge(s,t,l);
            i++;
        }
    }

    lI2 = new LandmarkedIndex(dgc, k , 1, 0, true, l);
    if( testIndices(lI, lI2, vertexDistribution, generator, L) == 1 )
    {
        return 1;
    }*/

    // removal of edge
    cout << "removal of edges" << endl;
    dg = new DGraph(edge_file);
    dgc = new DGraph(edge_file);
    lI = new LandmarkedIndex(dg, k , 1, 0, true, l);

    for(int i = 0; i < K; )
    {
        VertexID s = vertexDistribution(generator); // a random vertex
        VertexID t = vertexDistribution(generator); // a random vertex

        if( dgc->hasEdge(s,t) == true )
        {
            dgc->removeEdge(s,t);
            lI->removeEdge(s,t,0);
            i++;
        }
    }

    lI2 = new LandmarkedIndex(dgc, k , 1, 0, true, l);
    if( testIndices(lI, lI2, vertexDistribution, generator, L) == 1 )
    {
        return 1;
    }

    return 0;
}
