#include "Index.h"
#include "BFSIndex.h"

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;
using namespace std;

#ifndef PARTIALINDEX_H
#define PARTIALINDEX_H

    class PartialIndex : public Index
    {

    public:
        void construct(Graph* mg, vector< VertexID > ordering, bool isBlockedMode, int budget);

        PartialIndex(Graph* mg);
        PartialIndex(Graph* mg, vector< VertexID > ordering);
        PartialIndex(Graph* mg, bool isBlockedMode);
        PartialIndex(Graph* mg, vector< VertexID > ordering, bool isBlockedMode);
        PartialIndex(Graph* mg, vector< VertexID > ordering, bool isBlockedMode, int budget);
        ~PartialIndex();

        bool query(VertexID source, VertexID target, LabelSet ls);
        bool queryShell(VertexID source, VertexID target, LabelSet ls);
        bool queryDirect(VertexID source, VertexID target, LabelSet ls);
        void printIndexStats();

        unsigned long getIndexSizeInBytes();
        void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);

        void determineBudget();

        void buildIndex();
        void backPropagate(VertexID v, VertexID w, LabelSet ls);

        void createOrdering();
        int buildIndexForNode(VertexID w);
        bool tryInsert(VertexID w, VertexID v, LabelSet ls);

        void addNode();
        void removeNode(VertexID v);
        void addEdge(VertexID v, VertexID w, LabelID lID);
        void removeEdge(VertexID v, VertexID w, LabelID lID);
        void changeLabel(VertexID v, VertexID w, LabelID lID);

    private:
        int visitedSetSize;

        int labelSetSize;

        int Nt; // Nt is the number of threads (multi-threading to be implemented)
        vector< VertexID > ordering; // ordering determines the order in which
        // nodes receive and send updates during each round

        int budget; // the number of nodes that are indexed per node

        int frequency; // is the number of nodes that needs to be hit to attempt
        // a direct query

        vector< VertexID > subset;
        vector< VertexID > vToSubsetID; // subset of vertices for which a partial index
        // is built
        long subsetSize;

        dynamic_bitset<> hasBeenIndexed;
    };
#endif
