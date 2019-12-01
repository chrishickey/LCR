#include "../../Index/BoundedLCR/BoundedBFS.cc"
#include "../../Graph/DGraph.cc"

#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <curses.h>
#include <time.h>
#include <chrono>
#include <thread>

using namespace std;
using namespace graphns;
using namespace boundedindexns;

using namespace std::chrono;

void writeQuerySetToFile(int index, string fileName, bool isTrue)
{
    string newFileName = "";
    if( isTrue )
    {
        newFileName = fileName + to_string(index) + ".true";
    }
    else
    {
        newFileName = fileName + to_string(index) + ".false";
    }

    ofstream queryFile (newFileName);
    if( queryFile.is_open() )
    {
        /*for(auto q : qs)
        {
            VertexID v = q.first.first;
            VertexID w = q.first.second;
            string ls = to_string( q.second );
            queryFile << v << " " << w << " " << ls << "\n";
        }*/
    }
    queryFile.close();
}



void generateAllQueriesC(int nqs, int nq, BoundedBFS* ind, Graph* mg, vector< int >& noOfLabels)
{
    int N = mg->getNumberOfVertices();
    int L = mg->getNumberOfLabels();

    int MAX_DIFFICULTY = 0;
    int MIN_DIFFICULTY = 0;
    if( N >= 100000 )
    {
        MAX_DIFFICULTY = 10 + sqrt(N);
        MIN_DIFFICULTY = 5;
    }
    else
    {
        MAX_DIFFICULTY = max((int) N/10 , 4);
        MIN_DIFFICULTY = min((int) log2(N), N);
    }

    if( MAX_DIFFICULTY <= (MIN_DIFFICULTY-2) )
    {
        MAX_DIFFICULTY = MIN_DIFFICULTY + 2;
    }

    cout << "MAX_DIFFICULTY=" << MAX_DIFFICULTY << ",MIN_DIFFICULTY=" << MIN_DIFFICULTY << endl;

    // two random distributions: one for picking a vertex, the second for
    // picking a difficulty
    unsigned int seed = time(NULL)*time(NULL) % 1000;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

    seed = (time(NULL)*time(NULL)*time(NULL)) % 1000;
    std::default_random_engine generator2(seed);
    std::uniform_int_distribution<int> diffDistribution(MIN_DIFFICULTY, MAX_DIFFICULTY - 1);

    seed = (2*time(NULL)*time(NULL)*time(NULL)) % 1000;
    std::default_random_engine generator3(seed);
    std::uniform_int_distribution<int> labelDistribution(0, L);

}

/*
Kicks off the whole process.
*/
int runGenQuery(string s, int nq, int nqs, vector<int>& noOfLabels)
{
    cout << "s=" << s << ",nqs=" << nqs << ",nq=" << nq << endl;

    DGraph* mg = new DGraph(s);
    BoundedBFS* ind = new BoundedBFS(mg);

    generateAllQueriesC(nqs, nq, ind, mg, noOfLabels);
}

void help()
{
    cout << "boundedGenQuery <s: edge file> <k: number of query sets> <l: number of queries per set per true or false>" << endl;
    cout << "<k numbers denoting the sum of the number of labels in the minimum part> <k offsets added to the minimum>"  << endl;
    cout << "e.g. 'foo.edge 3 100 32 48 64 8 8 8' creates 6 query sets (one true and one false)." << endl;
    cout << "Each has 100 queries of the form (v,w,B1,B2), in which the sum of the number of labels in B1 is either 32, 48 or 64" << endl;
    cout << "and the sum of the number of labels in B2 is either 40, 56 or 72" << endl;
}

int main(int argc, char *argv[])
{
    /*
    * Generates 2*k sets of bounded queries each containing l queries for a specific file s
    *
    * Output is written to <s>.edge0.boundedtrue <s>.edge0.boundedfalse ... <s>.edge(k-1).boundedtrue <s>.edge(k-1).boundedfalse
    *
    */

    if( argc < 3 )
    {
        help();
        return 1;
    }

    string s = argv[1];
    int nqs = atoi(argv[2]);
    int nq = atoi(argv[3]);
    vector< int > noOfLabels;

    if( argc < (3+2*nqs) )
    {
        help();
        return 1;
    }

    for(int i = 0; i < 2*nqs; i++)
    {
        int nO = atoi(argv[4+i]);
        noOfLabels.push_back( nO );
    }

    runGenQuery(s, nq, nqs, noOfLabels);

    return 0;
}
