/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).
*/

#include "IndexWD.h"
#include "../Index.h"

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;
using namespace std;

#ifndef LANDMARKEDWD_H
#define LANDMARKEDWD_H

    namespace landmarkedwdns
    {

    }

    class LandmarkedIndexWD : public IndexWD
    {
        public:
            LandmarkedIndexWD(Graph* mg, int noOfLandmarks, int othersBudget);
            ~LandmarkedIndexWD();

            void construct(Graph* mg, int noOfLandmarks, int othersBudget);

            bool query(VertexID source, VertexID target, LabelSet ls);
            bool queryShell(VertexID source, VertexID target, LabelSet ls);
            bool queryShellAdapted(VertexID source, VertexID target, LabelSet ls);
            bool queryDirect(VertexID source, VertexID target, LabelSet ls);
            void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);
            bool extensiveQueryDirect(VertexID source, VertexID target, LabelSet ls,
                dynamic_bitset<>& marked);

            unsigned long distanceQuery(VertexID v, VertexID w, LabelSet ls);

            void printIndexStats();

            void buildIndex();
            void determineLandmarks();

            int buildIndexForNodePQ(VertexID w, bool doPropagate, bool isMaintenance);
            int findPathstoLandmarks(VertexID w, bool doPropagate);
            bool tryInsert(VertexID w, VertexID v, LabelSet ls, int dist);

            unsigned long getIndexSizeInBytes();
            unsigned long getIndexSizeInBytes(VertexID v);

        private:
            vector< VertexID > landmarks;
            vector< int > vToLandmark;

            int noOfLandmarks;

            dynamic_bitset<> hasBeenIndexed;
            int othersBudget;

    };
#endif
