#include "../../Graph/Graph.h"
#include "Index.h"

#include <set>
#include <list>

using namespace graphns;
using namespace indexns;

/**
* NeighbourExchange/Exact:
*
* This class builds an exact index. For each node v we construct all minimal
* labelsets from any other node w s.t. w can reach v.
*
* First we create an ordering among the nodes. This comes down to giving each
* node a number which says which node will be processed first.
*
* The construction process takes several rounds. As long as there 'updates' in at
* least one node we need a new round.
*
* A round loops over the nodes according to the ordering. For each node v we first
* check whether there updates and for each update that had an effect on the index
* of v we send an update.
*
* Suppose the index of node 0 is 1 1100 (meaning 1 can reach 0 using labelset 1100).
* and we get an update 1 1000 (meaning 1 can reach 0 using labelset 1000). Then we
* can update the index of node 0 to 1 1000 (because 1000 is a subset of 1100).
* Now we have updated the index and we need to send an update to all neighbours of 0.
* Suppose 2 is a neighbour of 0 of label 0010. Then we can send to 2 an update with
* the contents 1 1010 (meaning 1 can reach 2 using labels 1010).
*
* The result is an index that says for each node v a list of nodes W (subset of V).
* For each w in W we show all minimal paths from w to v.
*
* So if toString() says:
*
* v=1 {
* 0
* 10000000
}
* this means that 0 can reach 1 by using LabelSet 10000000 (which is the first label)
*/
#ifndef NEIGHBOUREXCHANGE_H
#define NEIGHBOUREXCHANGE_H
    namespace neighbourexchangens
    {
    }

    class NeighbourExchange : public Index
    {
        public:
            void construct(Graph* mg, vector<VertexID> pordering, int pNt);

            // Constructor for a custom ordering and a self-defined number of threads
            NeighbourExchange(Graph* mg, vector<VertexID> pordering, int pNt);
            // Default constructor
            NeighbourExchange(Graph* mg);
            ~NeighbourExchange();

            bool query(VertexID source, VertexID target, LabelSet ls);
            // queryShell actually runs the query
            bool queryShell(VertexID source, VertexID target, LabelSet ls);
            void buildIndex();

            void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);

            // builds a part of the index, i.e. on a subset of the nodes
            // can be used by multi-threading
            void buildIndexPart(vector<VertexID>& vertices);

            // checks whether a vertex v has received updates and sends them
            // returns true if at least one update was sent
            bool checkAndSendUpdates(VertexID v);

            void createOrdering();

            unsigned long getIndexSizeInBytes();

        private:
            int visitedSetSize;

            int Nt; // Nt is the number of threads (multi-threading to be implemented)
            vector< VertexID > ordering; // ordering determines the order in which
            // nodes receive and send updates during each round

            EIndex cUp; // used to store updates
            // during construction
    };
#endif
