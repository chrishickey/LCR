/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

BoundedIndex.h is an interface for all indices from BFS to  for bounded LCR.

Bounded LCR is a query (v,w,L) where L is a set of labels in the graph. There is a
bound on how often each label may be taken for the query, e.g.

(1,2,{a (1,2), b (2,3)}) means
does there exists a path from 1 to 2 using at least 1 and at most 2 "a"-edges and
using at least 2 and at most 3 "b"-edges?

Restrictions approach:
- We define a budget for each label x in L
- A budget for any label x should be at least 0 and at most N.
- A path from v to w should be simple, i.e. no cycles. Otherwise we need to maintain
some path in the queue to keep our queries time efficient, i.e. prevent long looping over
a cycle.

*/

#ifndef BINDEX_H
#define BINDEX_H

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

namespace boundedindexns
{
    /* All indexes have one of the following types */
    enum BoundedIndexType { BFS };

    /* A Budget is a 4 byte-array of length L. It gives an indication how often
    each label has been used or can be used.*/
    typedef vector< unsigned int > Budget;

    typedef struct
    {
        VertexID from;
        VertexID to;
        Budget b1; // each label x must be present at least b1[x] times
    }
    B1Query;

    typedef struct
    {
        VertexID from;
        VertexID to;
        Budget b2; // // each label x must be present at most b2[x] times
    }
    B2Query;

    typedef struct
    {
        VertexID from;
        VertexID to;
        Budget b1; // each label x must be present at least b1[x] times
        Budget b2; // // each label x must be present at most b2[x] times
    }
    B12Query;

    typedef struct
    {
        VertexID x;
        Budget b;
    }
    BEntry;

    B2Query generateB2Query(VertexID f, VertexID t, Budget& maxVals)
    {
        B2Query b2q;
        b2q.from = f;
        b2q.to = t;
        b2q.b2 = maxVals;

        return b2q;
    }

    B12Query generateB12Query(VertexID f, VertexID t, Budget& minVals, Budget& maxVals)
    {
        B12Query b12q;
        b12q.from = f;
        b12q.to = t;

        for(int i = 0; i < minVals.size(); i++)
        {
            //cout << "minVals[i]=" << minVals[i] << ", maxVals[i]=" << maxVals[i] << endl;
            b12q.b1.push_back( minVals[i] );
            b12q.b2.push_back( maxVals[i] );
        }

        return b12q;
    }

    string budgetToString(Budget& budget)
    {
        string s = "{";
        for(int i = 0; i < budget.size(); i++)
        {
            s += to_string(budget[i]);

            if( i < budget.size()-1 )
                s += ",";
        }
        s += "}";
        return s;
    }

    string queryToString(B2Query& q)
    {
        string s = "(";
        s += to_string(q.from) + "," + to_string(q.to) + ",";
        s += budgetToString(q.b2);
        s += ")";
        return s;
    }

    string queryToString(B12Query& q)
    {
        string s = "(";
        s += to_string(q.from) + "," + to_string(q.to) + ",";
        s += budgetToString(q.b1) + ",";
        s += budgetToString(q.b2);
        s += ")";
        return s;
    }
}

class BoundedIndex
{
    protected:
        Graph* graph;
        long M,N,L; // the graphs number of edges, nodes and labels

        double cStart, cEnd; // start and end times of construction
        double qStart, qEnd; // start and end times of last query

        long bMaxLimit;

        boundedindexns::BoundedIndexType indexType;
        string name;

    public:

        bool query(boundedindexns::B12Query& q)
        {
            if( isValidB12Query(q) == false )
            {
                cerr << "queryShell B12Query invalid query" << endl;
                return false;
            }

            cout << "query B12query=" << queryToString(q) << endl;

            qStart = getCurrentTimeInMilliSec();
            bool b = queryShell(q);
            qEnd = getCurrentTimeInMilliSec();
            return b;
        }

        bool query(boundedindexns::B2Query& q)
        {
            if( isValidB2Query(q) == false )
            {
                cerr << "queryShell B2Query invalid query" << endl;
                return false;
            }

            cout << "query B2query=" << queryToString(q) << endl;

            qStart = getCurrentTimeInMilliSec();
            bool b = queryShell(q);
            qEnd = getCurrentTimeInMilliSec();
            return b;
        }

        virtual bool queryShell(boundedindexns::B12Query& q) = 0;
        virtual bool queryShell(boundedindexns::B2Query& q) = 0;

        bool isValidB2Query(boundedindexns::B2Query& q)
        {
            if(q.from < 0 || q.from >= N || q.to < 0 || q.to >= N)
            {
                return false;
            }

            if( q.b2.size() != L )
            {
                return false;
            }

            for(int i = 0; i < L; i++)
            {
                if( q.b2[i] < 0 || q.b2[i] > bMaxLimit )
                {
                    return false;
                }
            }

            return true;
        }

        bool isValidB12Query(boundedindexns::B12Query& q)
        {
            //cout << "q.from=" << q.from << " ,q.to=" << q.to << endl;

            if( q.from < 0 || q.from >= N || q.to < 0 || q.to >= N )
            {
                return false;
            }

            if( q.b1.size() != q.b2.size() && q.b1.size() != L )
            {
                return false;
            }

            for(int i = 0; i < L; i++)
            {
                //cout << "d i=" << i << " ,q.b1[i]=" << q.b1[i] << " ,q.b2[i]=" << q.b2[i] << endl;

                if( q.b2[i] < 0 || q.b2[i] > bMaxLimit || q.b1[i] < 0 || q.b1[i] > bMaxLimit )
                {
                    return false;
                }

                if( q.b2[i] < q.b1[i] )
                {
                    return false;
                }
            }

            return true;
        }

        double getIndexConstructionTimeInSec()
        {
            double delta = max(cEnd - cStart, 0.00000001) + graph->getGraphConstructionTime();;
            return delta;
        }

        double getLastQueryTimeInSec()
        {
            double delta = max(qEnd - qStart, 0.00000001);
            return delta;
        }

        boundedindexns::BoundedIndexType getIndexType()
        {
            return indexType;
        }

        string getIndexTypeAsString()
        {
            if( name.size() != 0 )
            {
                return name;
            }

            string a = "none";
            switch(this->indexType)
            {
                case boundedindexns::BFS:
                    a = "BFS";
                default:
                    break;
            }

            return a;
        }

        long getIndexSizeInBytes()
        {
            long size = graph->getGraphSizeInBytes();
            return size;
        };

        long getIndexSizeInBytes(VertexID v)
        {
            return 0;
        };

        string toString()
        {
            return "";
        };

        string toString(VertexID v)
        {
            return "";
        };


};
#endif
