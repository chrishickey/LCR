/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).
*
* IndexWP is an interface for answering LCR-queries (v,w,ls) and finding the shortest path
* connecting v and w using only the labels in ls.
*
*
*/

#ifndef INDEXWP_H
#define INDEXWP_H

#include "../../Graph/Graph.h"

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
#include <set>

#include <thread>
#include <mutex>
#include <array>
#include <sys/time.h>

#include <boost/dynamic_bitset.hpp>

using namespace graphns;
using namespace boost;

namespace indexwpns
{
    // A path is simply a vector of vertices in the order
    typedef vector< VertexID > Path;
    typedef vector< vector< VertexID > > Paths;

    // A PathSet indicates only the presence of vertices in a path
    typedef dynamic_bitset<> PathSet;
    typedef vector< PathSet > PathSets;

    // A LP consists of a path (excluding or including it's target and/or source) and the
    // set of labels necessary to traverse the path
    typedef pair< LabelSet, Path > LP;

    // A LSP consists of a PathSet (excluding or including it's target and/or source) and the
    // set of labels necessary to traverse the path
    typedef pair< LabelSet, PathSet > LPS;

    // HeapPathEntry is used when building an index for a node. These entries
    // are pushed on a heap.
    typedef struct
    {
        VertexID to; // last vertex of path
        int dist; // the number of labels in the labelset
        LP lp; // excluding last and first vertex of path
    }
    HeapPathEntry;

    typedef struct
    {
        VertexID to;
        vector< LP > entries; // a set of LP's. For any two LP's lp1 and lp2
        // we should ensure that lp1 and lp2 are no super- or subset of each
        // other.
    }
    IndexPathEntry;

    typedef vector< IndexPathEntry > IndexPerNode;
    typedef vector< IndexPerNode > IndexForNodes;

    struct comp_IPNs
    {
       bool operator()( const IndexPathEntry& lhs,  const VertexID& rhs ) const
       {
           return lhs.to < rhs;
       }

       bool operator()( const VertexID& lhs, const IndexPathEntry& rhs ) const
       {
           return lhs<rhs.to;
       }
    };

    enum IndexWPType { BFS, Exact, Landmarked };

    struct HeapPathOrdering
    {
        bool operator()(const HeapPathEntry& hpe1, const HeapPathEntry& hpe2)
        {
            // return "true" if "hpe1" is ordered after "hpe2"
            // i.e. hpe1 either has more labels (dist is greater)
            // or hpe1 has a longer path
            if( hpe1.dist == hpe2.dist )
            {
                return hpe1.lp.second.size() >= hpe2.lp.second.size();
            }

            return hpe1.dist > hpe2.dist;
        }
    };

    void addVertexToPath(VertexID v, PathSet& ps)
    {
        ps[v] = 1;
    }

    void addVertexToPath(VertexID v, Path& p)
    {
        p.push_back( v );
    }

    void concatenatePaths(Path& p1, Path& p2, Path& p3)
    {
        // concatenate path p1 to p2
        //p3.clear();
        p3 = p1;
        p3.insert( p3.end(), p2.begin(), p2.end() );
    }

    bool vertexInPath(VertexID v, Path& p)
    {
        for(int j = 0; j < p.size(); j++)
        {
            if( p[j] == v )
            {
                return true;
            }
        }

        return false;
    }

    bool findTupleInTuples(VertexID w, IndexPerNode& ipn, int& pos)
    {
        if( ipn.size() == 0 )
        {
            pos = 0;
            return false;
        }

        int low = 0;
        int high = ipn.size() - 1;
        int mid = floor( (low + high) / 2 );

        while( high >= low )
        {
            mid = floor( (low + high) / 2 );
            //cout << "findTupleInTuples loop mid=" << mid << " low=" << low << " high=" << high << " tus[mid].first=" << tus[mid].first << endl;

            if( ipn[mid].to == w )
            {
                pos = mid;
                return true;
            }

            if( ipn[mid].to > w )
            {
                high = mid - 1;
            }
            else
            {
                low = mid + 1;
            }

        }

        pos = mid;
        if( ipn[mid].to < w)
            pos++;

        return false;
    }

    /*
    * Tries to insert a LP (pair of LabelSet and Path) into an IndexPathEntry
    *
    * The method returns true if and only if lp was added to ipe.entries
    *
    * lp will be added if and only if:
    *
    * - there is not another lp' in ipe.entries s.t. lp'.first is a subset of
    * lp.first (1, minimality of index)
    * - if lp.first = lp'.first, then lp.second.size() < lp'.second.size() (2,choosing the shorter path)
    *
    * if there exist lp' after adding lp s.t. either condition 1 or 2 is violated,
    * then these will be removed.
    *
    * Precondition and postcondition: isMinimalIndex(), not actually tested
    * because it would slow down construction time drastically
    */
    inline bool tryInsertLabelSet(LP& lp, IndexPathEntry& ipe)
    {
        bool hasBeenPlaced = false;

        for(int i = 0; i < ipe.entries.size(); i++)
        {
            LP lp1 = ipe.entries[i];

            bool b1 = isLabelSubset(lp1.first, lp.first);
            bool b2 = isLabelSubset(lp.first, lp1.first);

            if( b1 == true && b2 == false ) // lp is covered by lp1
            {
                return false;
            }
            else if( b1 == false && b2 == true ) // lp1 is redundant because of lp
            {
                if( hasBeenPlaced == false )
                {
                    // just replace it, but remove subsequent similar cases
                    ipe.entries[i] = lp;
                    hasBeenPlaced = true;
                }
                else
                {
                    // we have already replaced one, hence we need to remove this entry
                    ipe.entries.erase( ipe.entries.begin() + i );
                    i--;
                }
            }
            else if( b1 == true && b2 == true ) // lp1 and lp2 are the same
            {
                // pick the shortest path of the two
                // just replace it and because of equality we can now stop
                // as for all remaining entries b1 and b2 will be both false
                if( lp.second.size() < lp1.second.size() )
                {
                    ipe.entries[i] = lp;
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }

        if( hasBeenPlaced == false )
        {
            // base case, the entry is no super- or subset of any other
            // entry
            ipe.entries.push_back( lp );
            hasBeenPlaced = true;
        }

        return hasBeenPlaced;
    }
}

class IndexWP
{
    protected:
        /* The actual index */
        indexwpns::IndexForNodes ifn;

        Graph* graph;
        indexwpns::IndexWPType indexType;

        /* construction and query start and end times */
        double cStart, cEnd, qStart, qEnd;

        /* Set these as the number of vertices, edges and labels */
        long N, M, L;

    public:

        virtual long getIndexSizeInBytes() = 0;
        virtual long getIndexSizeInBytes(VertexID v) = 0;
        virtual bool queryNP(VertexID v, VertexID w, LabelSet ls) = 0;
        virtual bool queryWP(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p) = 0;

        double getIndexConstructionTimeInSec()
        {
            return max(cEnd - cStart, 0.00000001);
        }

        double getLastQueryTime()
        {
            return max(qEnd - qStart, 0.00000001);
        }

        bool isMinimalIndex()
        {
            for(int i = 0; i < N; i++)
            {
                if( isMinimalIndex(i) == false )
                    return false;
            }

            return true;
        }

        bool isMinimalIndex(VertexID v)
        {
            if( v < 0 || v >= ifn.size() )
                return true;

            indexwpns::IndexPerNode ipn = ifn[v];
            VertexID prev = 0;
            for(int l = 0; l < ipn.size(); l++)
            {
                VertexID x = ipn[l].to;
                if( x < prev )
                {
                    return false;
                }

                for(int m = 0; m < ipn[l].entries.size(); m++)
                {
                    indexwpns::LP lp = ipn[l].entries[m];

                    for(int o = m; o < ipn[l].entries.size(); o++)
                    {
                        indexwpns::LP lp2 = ipn[l].entries[o];

                        bool b1 = isLabelSubset( lp.first, lp2.first );
                        bool b2 = isLabelSubset( lp2.first, lp.first );

                        if( b1 == true || b2 == true )
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        void initializeIndex()
        {
            initializeIndex(N);
        }

        void initializeIndex(int size)
        {
            ifn = indexwpns::IndexForNodes();
            for(int i = 0; i < size; i++)
            {
                indexwpns::IndexPerNode ipn = indexwpns::IndexPerNode();
                ifn.push_back( ipn );
            }
        }

        string toString()
        {
            string s = "";
            for(int i = 0; i < N; i++)
            {
                s += toString(i) + "\n";
            }
            return s;
        }

        string toString(VertexID i)
        {
            string s = "";
            s += "v=" + to_string(i) + "\n{\n";
            for(int l = 0; l < ifn[i].size(); l++)
            {
                s += "*w=" + to_string(ifn[i][l].to) + "\n*{\n";
                for(int j = 0; j < ifn[i][l].entries.size(); j++)
                {
                    indexwpns::LP lp = ifn[i][l].entries[j];
                    s += "(" + to_string( lp.first ) + ",\n";
                    for(int k = 0; k < lp.second.size(); k++)
                    {
                        s += to_string(lp.second[k]);

                        if( k < ( lp.second.size()-1 ) )
                            s += ",";
                    }
                    s += ")\n";
                }
                s += "*}\n";

            }
            s += "};\n";
            return s;
        }

        bool tryInsertLPToIndex(indexwpns::LP& lp, VertexID w, VertexID v)
        {
            // isBlockedMode is false by default here and doIndexPaths is
            // assumed to be true
            int pos = 0;
            bool b = indexwpns::findTupleInTuples(v, ifn[w], pos);

            if( b == false )
            {
                // entry was not found and should be inserted
                indexwpns::IndexPathEntry ipn;
                ipn.to = v;
                ipn.entries = vector< indexwpns::LP >();
                ipn.entries.reserve( graph->getNumberOfLabels() * 2 );
                ifn[w].insert( ifn[w].begin() + pos, ipn );
            }

            //cout << "tryInsertLPToIndex: w=" << w << " ,v=" << v << ", pos=" << pos << " ,b=" << b << endl;

            b = indexwpns::tryInsertLabelSet(lp, ifn[w][pos]);

            return b;
        }
};

#endif
