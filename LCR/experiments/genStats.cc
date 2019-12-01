#include "../Graph/DGraph.cc"

#include <string>
#include <vector>

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <stdio.h>

using namespace std;
using namespace graphns;

void printToFile(DGraph* g1, string filename)
{
    string stats = g1->getStats();
    std::ofstream out(filename);
    out << stats;
    out.close();
}

bool is_file_exist(string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

bool hasNoStatsFiles(string s)
{
    return is_file_exist(s + ".csv") == false;
}

int main(int argc, char *argv[])
{
    if( argc < 2 )
    {
        cout << "genStats: <edge-file>" << endl;
        return 1;
    }

    string s = argv[1];
    bool doOverwrite = false;
    if( argc == 3 )
    {
        doOverwrite = true;
    }

    if( hasNoStatsFiles(s) == false && doOverwrite == false )
    {
        return 0;
    }

    cout << "genStats: s=" << s << endl;
    DGraph* nG = new DGraph(s + ".edge");
    printToFile(nG, s + ".csv");


    return 0;
}
