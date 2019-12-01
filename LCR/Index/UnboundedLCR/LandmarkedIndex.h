/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

An index similar to DoubleBFS. However it only builds a full index for a subset of nodes,
called landmarks.

For all other nodes we can build a small index by setting doIndexOthers to true and
specifying a othersBudget. This builds for every non-landmark vertex an index containing
at most <othersBudget> number of LabelSets.

At query time (v,w,L) we do the following:

if we did not index the non-landmark nodes:

if v is a landmark:
    queryDirect(v,w,L)
otherwise we do a BFS:
    start BFS
    if we hit a landmark v' for the <frequency>'th time, queryDirect(v',w,L)
    else just continue BFS

else:
if v is a landmark:
    queryDirect(v,w,L)
otherwise we do a BFS:
    first try all indexed landmarks v' for v. If v can reach v' and v' can reach w,
    we have a direct hit
    start BFS
    if we hit a landmark v' for the <frequency>'th time, queryDirect(v',w,L)
    else just continue BFS

*/

#include "Index.h"
#include "BFSIndex.h"

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;
using namespace std;

#ifndef LANDMARKED_H
#define LANDMARKED_H

    namespace landmarkedns
    {
        typedef struct
        {
            LabelSet ls;
            VertexID x;
            int dist;
            int id;
        }
        BitEntry;

        typedef vector< Triplet > BitEntries;

        struct compBitEntries
        {
           bool operator()( const BitEntry& lhs,  const VertexID& rhs ) const
           {
               return lhs.x < rhs;
           }

           bool operator()( const VertexID& lhs, const BitEntry& rhs ) const
           {
               return lhs > rhs.x;
           }
        };

        struct PQBitEntries
        {
            bool operator()(BitEntry const & t1, BitEntry const & t2)
            {
                // return "true" if "p1" is ordered before "p2", for example:
                return t1.dist > t2.dist;
            }
        };


        typedef struct
        {
            LabelSet ls;
            dynamic_bitset<> V;
            int count;
        }
        SequenceEntry;

    }

    class LandmarkedIndex : public Index
    {
        public:

            /*
            Default constructor. Will start building the index immediately.

            mg: the directed labelled graph used as input
            doIndexOthers : 2nd extension of paper (OTH)
            doExtensive: 3rd extension of paper (EXTv2), 1st extension is no longer available
            k: number of landmarks
            b: budget per non-landmark vertex, determining the number of entries for each

            */
            LandmarkedIndex(Graph* mg, bool doIndexOthers, bool doExtensive, int k, int b);

            /* Other constructors */
            LandmarkedIndex(Graph* mg);
            LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode);
            LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget);
            LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget, int coverPercentage);
            LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget, int coverPercentage, bool doExtensive);
            ~LandmarkedIndex();

            /*
            Method called by all constructors. Does the actual work.

            propagateCode: can turn on/off propagation (fwProp in thesis)
            method: method used to find all landmarks (1 by default)

            */
            void construct(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget, int coverPercentage,
                bool doExtensive);

            bool query(VertexID source, VertexID target, LabelSet ls);
            bool queryShell(VertexID source, VertexID target, LabelSet ls);
            bool queryShellAdapted(VertexID source, VertexID target, LabelSet ls);
            bool queryDirect(VertexID source, VertexID target, LabelSet ls);
            void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);
            bool extensiveQueryDirect(VertexID source, VertexID target, LabelSet ls,
                dynamic_bitset<>& marked);

            void printIndexStats();

            /*
            Starts the index building.

            By giving a different continueCode (0,1,2) we can either do a full
            rebuild (0) or skip some part (1 or 2).
            */
            void buildIndex(int continueCode);

            int labelledBFSPerLM(VertexID w, bool doPropagate, bool isMaintenance);
            int labelledBFSPerLM(VertexID w, bool doPropagate, bool isMaintenance,
                priority_queue< landmarkedns::BitEntry, vector<landmarkedns::BitEntry>, landmarkedns::PQBitEntries >& q);
            int labelledBFSPerNonLM(VertexID w, bool doPropagate);

            bool tryInsert(VertexID w, VertexID v, LabelSet ls);
            int insertSeqEntry(VertexID w, LabelSet ls);
            int findSeqEntryId(LabelSet ls, VertexID w);

            void addNode();
            void removeNode(VertexID v);
            void addEdge(VertexID v, VertexID w, LabelID lID);
            void removeEdge(VertexID v, VertexID w, LabelID lID);
            void changeLabel(VertexID v, VertexID w, LabelID lID);

            bool queryShellBfs(VertexID source, VertexID target, LabelSet ls);
            void determineLandmarks();

            void setDoExtensive(bool isExtensive);
            void setOthersBudget(int budgetOthers);
            void setNoOfLandmarks(int noOfLandmarks);

            double computeQueryFilterProbability(LabelSet ls);
            unsigned long getIndexSizeInBytes();

            void updateName();

        private:
            double landmarkTime, othersTime;

            int Nt; // Nt is the number of threads (multi-threading to be implemented)

            vector< VertexID > landmarks;
            vector< int > vToLandmark;

            int noOfLandmarks;

            /* Instead of specifying a fixed number of landmarks you can also specify
            coverPercentage, which makes buildIndex index up to k landmarks s.t.
            the sum of outdegrees of the k landmarks over |E| equals coverPercentage.
            This is only supported for method 1 currently. */
            int coverPercentage;

            int frequency;
            int method;

            dynamic_bitset<> hasBeenIndexed;

            bool doIndexOthers;
            int budgetOthers;

            int propagateCode;

            int minPathLabelSetSize;
            int maxPathLabelSetSize;
            int noOfPaths;
            bool doExtensive;
            vector< int > mapping;
            vector< vector < vector< VertexID > > > landmarkPaths; // only used by landmarkedindex

            vector< VertexID > ordering;

            vector< vector< landmarkedns::SequenceEntry > > seqEntries;
    };
#endif
