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
#include "../../Index/UnboundedLCR/WD/LandmarkedIndexWD.cc"

#include "../../Graph/DGraph.cc"

using namespace std;
using namespace indexns;
using namespace graphns;

#ifdef _WIN32
#include <windows.h>
#define SYSERROR()  GetLastError()
#else
#include <errno.h>
#define SYSERROR()  errno
#endif

bool is_file_exist(string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        cout << "usage: ./runWDExperiment <edge_file>" << endl;
        exit(1);
    }

    string edge_file = argv[1]; // the file containing the graph
    Graph* g1 = new DGraph(edge_file);
    int t = 1;
    int N = g1->getNumberOfVertices();
    int k = N/10, l = 0;

    LandmarkedIndexWD* lIWD = new LandmarkedIndexWD(g1, k , l );
    Index* lI = new LandmarkedIndex(g1, k , 1, 0, true, l, 0, true);

    double b1 = lI->getIndexSizeInBytes() / 1000;
    double b2 = lIWD->getIndexSizeInBytes() / 1000;
    double t1 = lI->getIndexConstructionTimeInSec();
    double t2 = lIWD->getIndexConstructionTimeInSec();

    cout << "lI-size(MB)=" << b1 << ",lIWD-size(MB)=" << b2 << ",r="
        << print_digits(b2 / b1, 2) << endl;
    cout << "lI-time(s)=" << t1 << ",lIWD-time(s)=" << t2 << ",r="
        << print_digits(t2 / t1, 2) << endl;

    exit(EXIT_SUCCESS);
}
