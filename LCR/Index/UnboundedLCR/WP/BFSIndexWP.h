/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).
*
* Solution for LCR-WP queries using BFS
*
*/


#ifndef BFSINDEXWP_H
#define BFSINDEXWP_H

#include "../../Graph/Graph.h"
#include "../../Graph/DGraph.h"
#include "IndexWP.h"
#include "BFSIndex.h"

#include <boost/dynamic_bitset.hpp>


    using namespace graphns;
    using namespace boost;
    using namespace indexns;

    namespace bfsindexnswp
    {

    }


    class BFSIndexWP : public IndexWP
    {
        private:
            BFSIndex* bfsIndex;

        public:

            BFSIndexWP(Graph* g);
            ~BFSIndexWP();

            // regular LCR query, just returns true or false. Ignores path
            bool queryNP(VertexID source, VertexID target, LabelSet ls);

            // LCR query that returns the path
            void queryWP(VertexID source, VertexID target, LabelSet ls, indexwpns::Paths& paths);
            void queryWP(VertexID source, VertexID target, LabelSet ls, indexwpns::Paths& paths,
                int maxPathLength);


    };
#endif
