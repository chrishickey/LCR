/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

IndexWD.h is an extension of Index.h meant to answer queries and return the distance,
e.g.

v -(a)-> w -(b)-> x then q(v,w,{a,b}) == 2 and q(v,w,{a,b}) == INFINITY
*/


#ifndef INDEXWD_H
#define INDEXWD_H

#include "../../../Graph/Graph.h"
//#include "../Index/ClusteredExactIndex.h

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

#include "../Index.h"

using namespace graphns;
using namespace indexns;
using namespace boost;

namespace indexwdns
{
    /*
    Similar to indexns Triplet
    LabelDist is the number of labels necessary to reach x
    edgeDist is the number of edges necessary to reach x
    */
    typedef struct
    {
        LabelSet ls;
        VertexID x;
        int labelDist;
        int edgeDist;
    }
    TripletWD;

    typedef vector< TripletWD > TripletsWDs;

    struct compTripletsWD
    {
       bool operator()( const TripletWD& lhs,  const VertexID& rhs ) const
       {
           return lhs.x < rhs;
       }

       bool operator()( const VertexID& lhs, const TripletWD& rhs ) const
       {
           return lhs > rhs.x;
       }
    };

    struct priorityQueueTripletsWD
    {
        bool operator()(TripletWD const & t1, TripletWD const & t2)
        {
            return t1.edgeDist > t2.edgeDist;
        }
    };

    typedef pair< LabelSet, unsigned int > LabelSetDistancePair;
    typedef vector< LabelSetDistancePair > LabelSetDistancePairs;

    struct IE
    {
        VertexID x;
        LabelSetDistancePairs lsds;
    };

    typedef vector< IE > IEs;
    typedef vector< IEs > allIEs;

    struct compIEs
    {
       bool operator()( const IE& lhs,  const int& rhs ) const
       {
           return lhs.x < rhs;
       }

       bool operator()( const int& lhs, const IE& rhs ) const
       {
           return lhs<rhs.x;
       }
    };

    const unsigned int POS_INFINITY = 0xFFFFFFFF;
}

class IndexWD : public Index
{
    protected:
        indexwdns::allIEs Ind;

        virtual unsigned long distanceQuery(VertexID v, VertexID w, LabelSet ls) = 0;

        void intializeIndex(long size)
        {
            Ind = indexwdns::allIEs();

            for(int i = 0; i < size; i++)
            {
                indexwdns::IEs ind = indexwdns::IEs();

                Ind.push_back( ind );
            }
        }

        bool tryInsertLabelSet(VertexID v, VertexID w, LabelSet ls, int dist)
        {
            // try to insert (w,ls,dist) into Ind(v)
            int pos = 0;

            // find w in Ind(v)
            bool b = findTupleInTuples(v, w, pos);

            if( b == false )
            {
                //cout << "tryInsertLabelSet w not found" << endl;
                indexwdns::IE ie;
                ie.x = w;
                ie.lsds = indexwdns::LabelSetDistancePairs();

                indexwdns::LabelSetDistancePair pp = make_pair( ls, dist );
                ie.lsds.push_back( pp );

                Ind[v].insert( Ind[v].begin() + pos, ie );

                return true;
            }

            // Insert (ls,dist)
            bool hasBeenReplaced = false;
            bool isCovered = false;
            for(int i = 0; i < Ind[v][pos].lsds.size(); i++)
            {
                LabelSet ls2 = Ind[v][pos].lsds[i].first;
                int dist2 = Ind[v][pos].lsds[i].second;

                bool b1 = isLabelSubset(ls , ls2);
                bool b2 = isLabelSubset(ls2 , ls);

                //cout << "tryInsertLabelSet ls2=" << ls2 << ",d2=" << dist2 << ",b1=" << b1 << ",b2=" << b2 << endl;

                // ls is a subset of ls2 and has a larger distance
                // replace or remove (ls2, d2)
                if( b1 == true && b2 == false && dist <= dist2 )
                {
                    if( hasBeenReplaced == false )
                    {
                        Ind[v][pos].lsds[i].first = ls;
                        Ind[v][pos].lsds[i].second = dist;
                        hasBeenReplaced = true;
                    }
                    else
                    {
                        Ind[v][pos].lsds.erase( Ind[v][pos].lsds.begin() + i );
                        i--;
                    }
                }

                // ls and ls2 are the same
                if( b1 == true && b2 == true )
                {
                    if( dist < dist2 )
                    {
                        if( hasBeenReplaced == false )
                        {
                            Ind[v][pos].lsds[i].first = ls;
                            Ind[v][pos].lsds[i].second = dist;
                            hasBeenReplaced = true;
                        }
                        else
                        {
                            Ind[v][pos].lsds.erase( Ind[v][pos].lsds.begin() + i );
                            i--;
                        }
                    }
                    else
                    {
                        isCovered = true;
                        break;
                    }
                }

                if( b1 == false && b2 == true )
                {
                    if( dist >= dist2 )
                    {
                        // ls is a superset of ls2 but has a greater (or equal) distance
                        isCovered = true;
                        break;
                    }
                    else
                    {
                        // ls is a superset of ls2 but has a smaller distance
                    }
                }
            }

            if( isCovered == true )
            {
                return false;
            }

            if( hasBeenReplaced == false )
            {
                //cout << "tryInsertLabelSet Ind[v][pos].lsds.size()=" << Ind[v][pos].lsds.size() << endl;

                indexwdns::LabelSetDistancePair pp = make_pair( ls, dist );
                Ind[v][pos].lsds.push_back( pp );

                //cout << "tryInsertLabelSet Ind[v][pos].lsds.size()=" << Ind[v][pos].lsds.size() << endl;
            }

            return true;
        };

        bool findTupleInTuples(VertexID v, VertexID w, int& pos)
        {
            pos = (std::lower_bound(Ind[v].begin(),Ind[v].end(),w, indexwdns::compIEs() ) - Ind[v].begin() );

            if( pos < 0 || pos >= Ind[v].size() )
            {
                return false;
            }

            return Ind[v][pos].x == w;
        }
};

#endif
