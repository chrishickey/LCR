#include "Index.h"
#include "../../Graph/DGraph.h"

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace indexns;
using namespace boost;
using namespace std;

#ifndef ZHOU_H
#define ZHOU_H

    namespace zhouns
    {
        //typedef pair< pair < LabelSet, vector< VertexID > >, int > NeighTriplet;
        typedef struct
        {
            LabelSet ls;
            vector< VertexID > path;
        }
        NeighTriplet;

        typedef vector< NeighTriplet > NeighTriplets;

        struct comp_NeighTuples
        {
           bool operator()( const NeighTriplet& lhs,  const VertexID& rhs ) const
           {
               return lhs.path[lhs.path.size()-1] < rhs;
           }

           bool operator()( const VertexID& lhs, const NeighTriplet& rhs ) const
           {
               return lhs > rhs.path[rhs.path.size()-1];
           }
        };


        inline bool covers(zhouns::NeighTriplet& nt1, zhouns::NeighTriplet& nt2)
        {
            // returns whether nt1 covers nt2
            if( nt1.path[nt1.path.size()-1] != nt2.path[nt2.path.size()-1] )
            {
                return false;
            }

            return isLabelSubset(nt1.ls, nt2.ls);
        };

        inline bool findNT(NeighTriplet& nt, NeighTriplets& nts, int& pos)
        {
            auto it = std::lower_bound(nts.begin(), nts.end(), nt.path[nt.path.size()-1], comp_NeighTuples() );
            pos = (it - nts.begin());

            if( it == nts.end() )
            {
                return false;
            }

            return true;
        }

        inline bool isNTCovered(NeighTriplet& nt, NeighTriplets& nts, int& pos)
        {
            VertexID v = nt.path[nt.path.size() - 1];
            if( findNT(nt, nts, pos) == true )
            {
                int i = pos;
                while( nts[i].path[nts[i].path.size()-1] == v )
                {
                    if( isLabelSubset(nts[i].ls, nt.ls) )
                    {
                        return true;
                    }

                    i++;

                    if( i == nts.size() )
                    {
                        break;
                    }
                }
            }

            return false;
        }

        inline bool insertNT(NeighTriplet& nt, NeighTriplets& nts, int& pos)
        {
            nts.insert( nts.begin() + pos, nt );
        }

        inline string NTToString(NeighTriplet& nt)
        {
            string pathstring = "{";
            for(int i = 0; i < nt.path.size(); i++)
            {
                pathstring += to_string(nt.path[i]);

                if( i < (nt.path.size()-1) )
                {
                    pathstring += ",";
                }
            }
            pathstring += "}";

            string s = "(";
            s += "ls=" + labelSetToString(nt.ls) + ",";
            s += "path=" + pathstring + ")";

            return s;
        }

    }

    class Zou : public Index
    {

        public:
            Zou(Graph* mg);
            ~Zou();

            void buildIndex();
            void buildIndex(int SCCID, Graph* graph);
            void eDijkstra(int SCCID, VertexID v, Graph* graph);

            bool query(VertexID source, VertexID target, LabelSet ls);
            bool queryShell(VertexID source, VertexID target, LabelSet ls);

            struct CompareTriplets {
                bool operator()(zhouns::NeighTriplet const & p1, zhouns::NeighTriplet const & p2) {
                    // return "true" if "p1" is ordered before "p2", for example:
                    return getNumberOfLabelsInLabelSet(p1.ls) > getNumberOfLabelsInLabelSet(p2.ls);
                }
            };

            void produceNeighTriplets(zhouns::NeighTriplet& nt, Graph* graph, zhouns::NeighTriplets& nts);
            bool tryInsert(VertexID w, VertexID v, LabelSet ls);
            unsigned long getIndexSizeInBytes();

            void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach);

        private:
            vector<Graph*> subGraphs;
            vector< int > vToSubGraphID, vToSCCID;
            Graph* D;

            vector< vector < VertexID > > inPortals;
            vector< vector < VertexID > > outPortals;
            vector< vector < VertexID > > SCCs;
    };
#endif
