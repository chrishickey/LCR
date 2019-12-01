#ifndef LANDMARKEDWP_H
#define LANDMARKEDWP_H

#include "IndexWP.h"
#include "../../Graph/Graph.h"

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace boost;
using namespace indexwpns;

class LandmarkedWP : public IndexWP
{
    private:
        vector< int > vToLandmark;
        vector< VertexID > landmarks;
        int noOfLandmarks;

        dynamic_bitset<> hasBeenIndexed;

    public:
        LandmarkedWP(Graph* graph);
        LandmarkedWP(Graph* graph, int noOfLandmarks);
        ~LandmarkedWP();

        long getIndexSizeInBytes();
        long getIndexSizeInBytes(VertexID v);
        bool queryNP(VertexID v, VertexID w, LabelSet ls);
        bool queryWP(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p);

        bool queryDirect(VertexID v, VertexID w, LabelSet ls);
        bool queryDirect(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p);

        
        bool queryShell(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p);

        void buildIndex();
        int buildIndex(VertexID v);
        void determineLandmarks();
        bool tryInsert(VertexID w, VertexID v, indexwpns::LP& lp);
};

#endif
