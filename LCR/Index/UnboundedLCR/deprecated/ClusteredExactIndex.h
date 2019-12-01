#include "Index.h"
#include "NeighbourExchange.h"
#include "DoubleBFS.h"

#include "../../Graph/Graph.h"
#include "../../Graph/DGraph.h"

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
#include <set>
#include <list>

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;

#ifndef CLUSTEREDEXACT_INDEX_H
#define CLUSTEREDEXACT_INDEX_H

namespace clusteredexactindexns
{
}


class ClusteredExactIndex : public Index
{
    public:
        void construct(Graph* mg, IndexType pIndexType, int noOfClusters, string clusterFile);

        ClusteredExactIndex(Graph* mg);
        ClusteredExactIndex(Graph* mg, string clusterFile);
        ClusteredExactIndex(Graph* mg, int noOfClusters);
        ClusteredExactIndex(Graph* mg, IndexType pIndexType, int noOfClusters, string clusterFile);
        ~ClusteredExactIndex();

        bool queryShell(VertexID source, VertexID target, LabelSet ls);
        bool query(VertexID source, VertexID target, LabelSet ls);

        unsigned long getIndexSizeInBytes();
        void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);

        void createClustering(string s);
        void createClustering();
        void buildIndex();
        void printClustering();

        double clusterQuality();

    private:
        int noOfClusters;

        vector< int > vTocID;
        vector< int > vToLocalID;
        vector< int > countPerCluster;

        vector< vector < VertexID > > outPortsPerCluster;

        IndexType clusterType; // currently only supports DoubleBFS

        vector< Index* > indices;

};



#endif
