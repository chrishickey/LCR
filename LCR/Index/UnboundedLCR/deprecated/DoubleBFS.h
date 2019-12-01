/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

Despite its name it runs a single labeled BFS from each vertex (in the original idea this was done twice).
In this way, all paths are explored between two vertices.

For any node v we find all LabelSets to all other nodes w that v can reach. The size of the index
might grow really large.

TO DO: finish multi-threading
*/

#include "Index.h"
#include "BFSIndex.h"

#include <boost/dynamic_bitset.hpp>
//#include <boost/thread.hpp>
//#include <boost/date_time.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;
using namespace std;

#ifndef DOUBLEBFS_H
#define DOUBLEBFS_H

    namespace dbfsns
    {

    }

    class DoubleBFS : public Index
    {

        public:
            DoubleBFS(Graph* mg);
            DoubleBFS(Graph* mg, vector< VertexID > ordering);
            DoubleBFS(Graph* mg, bool isBlockedMode);
            DoubleBFS(Graph* mg, vector< VertexID > ordering, bool isBlockedMode, bool useHeap);
            void construct(Graph* mg, vector< VertexID > ordering, bool isBlockedMode, bool useHeap);
            ~DoubleBFS();
            unsigned long getIndexSizeInBytes();

            bool query(VertexID source, VertexID target, LabelSet ls);
            bool queryShell(VertexID source, VertexID target, LabelSet ls);
            void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);
            void printIndexStats();

            void buildIndex(bool doRebuild);
            void backPropagate(VertexID v, VertexID w, LabelSet ls);

            void createOrdering();
            int buildIndexForNodeMultiThread(VertexID w, int threadID);
            int buildIndexForNode(VertexID w);
            int buildIndexForNodeB(VertexID w);
            bool tryInsert(VertexID w, VertexID v, LabelSet ls);

            void addNode();
            void removeNode(VertexID v);
            void addEdge(VertexID v, VertexID w, LabelID lID);
            void removeEdge(VertexID v, VertexID w);
            void changeLabel(VertexID v, VertexID w, LabelID lID);


        private:
            int visitedSetSize;

            int labelSetSize;

            vector< VertexID > ordering; // ordering determines the order in which
            // nodes receive and send updates during each round

            dynamic_bitset<> hasBeenIndexed;
            bool useHeap;

            // Multi-threading only supported in DoubleBFS
            std::vector<std::mutex> mutexes;
            int Nt; // Nt is the number of threads
            vector< bool > isActive;
            std::mutex isActiveMutex;
    };
#endif
