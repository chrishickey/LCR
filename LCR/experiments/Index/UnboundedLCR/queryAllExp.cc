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

int runQueryAll(Index* lI, Index* bfs, VertexID v, LabelSet ls, int N, double& sum)
{
    dynamic_bitset<> bs1 = dynamic_bitset<>(N);
    dynamic_bitset<> bs2 = dynamic_bitset<>(N);

    bfs->queryAll(v,ls,bs1);
    lI->queryAll(v,ls,bs2);

    if( bs1.count() < N / 10 )
    {
        return 2;
    }

    bool b = bs1.is_subset_of(bs2) && bs2.is_subset_of(bs1);
    cout << "(" << v << "," << ls << ")," << print_digits( (bfs->getLastQueryTime() / lI->getLastQueryTime()),2 )
       << "," << bs1.count() << "," << bs2.count() << endl;

    sum += (bfs->getLastQueryTime() / lI->getLastQueryTime());

    bs1.reset();
    bs2.reset();

    if( b == false )
    {
        cout << "query (" << v << "," << ls << ") failed." << endl;
        return 1;
    }

    return 0;
}

/*
With this file we can test the speed-up and correctness of queryAll-queries,
i.e. find all nodes v' s.t. for a given v in V and L' subset of L we have that
query(v,v',L) == True
*/
int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        cout << "./queryAll <edge_file>" << endl;
        return 1;
    }

    string edge_file = argv[1]; // the file containing the graph
    DGraph* dg = new DGraph(edge_file);

    int N = dg->getNumberOfVertices();
    int L = dg->getNumberOfLabels();

    if( L < 3 || N < 100 )
    {
        cout << "L or N too small" << endl;
        return 1;
    }

    int noOfDifficulties = 2;
    int* arr = new int[2]{ L/2, L-2 };

    unsigned int seed = time(NULL)*time(NULL) % 1000;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

    seed = (time(NULL)*time(NULL)*time(NULL)) % 1000;
    std::default_random_engine generator2(seed);
    std::uniform_int_distribution<int> labelDistribution(0, L - 1);

    Index* bfs = new BFSIndex(dg);
    int k = N/50; int l = 0;
    Index* lI = new LandmarkedIndex(dg, k , 1, 0, true, l); // rebuild

    for(int i = 0; i < noOfDifficulties; i++)
    {
        cout << "arr[i]=" << arr[i] << endl;
        double sum = 0.0;
        int Nk = 100;

        for(int j = 0; j < Nk; j++)
        {
            VertexID s = vertexDistribution(generator); // a random start point
            LabelSet ls = 0;
            ls = generateLabelSet(ls, arr[i], L, labelDistribution, generator2);
            int a = runQueryAll(lI, bfs, s, ls, N, sum);
            if( a == 1 )
            {
                return 1;
            }
            if( a == 2 )
            {
                j--;
            }
        }

        cout << "arr[i]=" << arr[i] << ", avg=" << (sum / Nk) << endl;
    }

    return 0;
}
