/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

Index.h is an interface for all indices from BFS to Zou.

There can be two modes for an index.

- blocked. We have a 2dim array N by N with an entry for each vertex-pair. Index building
goes faster, but has more memory overhead. Not practical for large N. Uses cIn.

- non-blocked. Each vertex holds a list of tuples. A Tuple is a pair of the form < VertexID , list of LabelSets >.
Uses tIn.

Triplet is a structure s.t. entries during index construction can be ordered according to
the number of labels in the LabelSet of the triplet. E.g. if t.ls = 0101 then t.dist should
be 2.

Index.h has some methods to initialize the index, calculate the index size, write the index as
as a string or insert a LabelSet into a set of LabelSets.
*/


#ifndef INDEX_H
#define INDEX_H

#include "../../Graph/Graph.h"
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


    using namespace graphns;
    using namespace boost;

    namespace indexns
    {
        // used for sorting pairs on second value
        struct sort_pred
        {
            bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right)
            {
                return left.second < right.second;
            }
        };

        typedef pair< pair < VertexID, VertexID > , LabelSet > Query;
        typedef pair< Query, bool > QueryWithAnswer;
        typedef set < Query > QuerySet;
        typedef vector< QuerySet > QuerySets;

        /* Default way of an index. */
        typedef vector< LabelSet > LabelSets;
        typedef vector< LabelSets > LabelSetsPerNode;
        typedef vector< LabelSetsPerNode > EIndex;

        /* Alternative way to create index */
        // A tuple is a VertexID with a list of labels both used for updates
        // and the index
        typedef pair< VertexID, vector< LabelSet > > Tuple;
        //Tuples is a list of Tuple. The index for any node consists of a list
        // of tuples. We assume that each Tuple in Tuples is sorted on VertexID
        typedef vector< Tuple > Tuples;
        // This is the list of Tuples which is basically a type for the full
        // index
        typedef vector< Tuples> TuplesList;

        struct comp_tuples
        {
           bool operator()( const Tuple& lhs,  const int& rhs ) const
           {
               return lhs.first < rhs;
           }

           bool operator()( const int& lhs, const Tuple& rhs ) const
           {
               return lhs<rhs.first;
           }
        };

        /* altIndexPN is a different kind of index. For each node v we create K vectors
        where K stands for the number of subsets of labelset L that are at most of length
        maxL. If v can reach w through a labelset L' and |L'| is at most maxL then we add
        w to the corresponding vector.
        This should be used in combination with another index for completeness.*/
        typedef vector< VertexID > altIndexPLS;
        typedef vector< altIndexPLS > altIndexPN;
        typedef vector< altIndexPN > altIndexF;

        /* All indexes have one of the following types */
        enum IndexType { BFS, Exact, DoubleBFSenum, Join, Clustered, Partial, Landmarked, ZouType };

        typedef struct
        {
            LabelSet ls;
            VertexID x;
            int dist;
        }
        Triplet;

        typedef vector< Triplet > Triplets;

        struct compTriplets
        {
           bool operator()( const Triplet& lhs,  const VertexID& rhs ) const
           {
               return lhs.x < rhs;
           }

           bool operator()( const VertexID& lhs, const Triplet& rhs ) const
           {
               return lhs > rhs.x;
           }
        };

        struct priorityQueueTriplets {
            bool operator()(Triplet const & t1, Triplet const & t2) {
                // return "true" if "p1" is ordered before "p2", for example:
                return t1.dist > t2.dist;
            }
        };

        typedef struct
        {
            LabelSet ls;
            VertexID x;
            VertexID y; // the predecessor of x, i.e. there exists an edge (y,x)
            int dist;
        }
        Quadret;

        typedef vector< Quadret > Quadrets;

        struct compQuadrets
        {
           bool operator()( const Quadret& lhs,  const VertexID& rhs ) const
           {
               return lhs.x < rhs;
           }

           bool operator()( const VertexID& lhs, const Quadret& rhs ) const
           {
               return lhs > rhs.x;
           }
        };

        struct priorityQueueQuadrets {
            bool operator()(Quadret const & t1, Quadret const & t2) {
                // return "true" if "p1" is ordered before "p2", for example:
                return t1.dist > t2.dist;
            }
        };

        inline string printDigits(double d)
        {
            std::stringstream s;
            s << std::fixed << std::setprecision(std::numeric_limits<double>::digits10) << d;
            std::string res = s.str();

            size_t dotIndex = res.find(".");
            std::string final_res = res.substr(0, dotIndex + 8);

            return final_res;
        }

        /*
        * checks whether ls is a superset of any entry in lss. Returns true
        * if this is the case.
        */
        inline bool labelSetInLabelSets(LabelSet ls, LabelSets lss)
        {
            bool b1 = false;
            for(int i = 0; i < lss.size(); i++)
            {
                LabelSet ls1 = lss[i];

                if( isLabelSubset(ls1, ls) )
                {
                    return true;
                }
            }

            return b1;
        }

        /**
        * A tuple is a pair <VertexID, LabelSets >
        */
        inline int findTupleInTuples(VertexID v, Tuples& tuples)
        {
            //cout << "findTupleInTuples: v=" << v << ",tuples.size()=" << tuples.size() << endl;

            auto pos = std::lower_bound(tuples.begin(),tuples.end(),v, comp_tuples() );
            return pos - tuples.begin();
        }

        inline bool findTupleInTuples(VertexID v, Tuples& tuples, int& pos)
        {
            //cout << "findTupleInTuples: v=" << v << ",tuples.size()=" << tuples.size() << endl;
            pos = (std::lower_bound(tuples.begin(),tuples.end(),v, comp_tuples() ) - tuples.begin() );

            if( pos < 0 || pos >= tuples.size() )
            {
                return false;
            }

            return tuples[pos].first == v;
        }

        inline bool tupleExists(VertexID v, Tuples& tuples, int pos)
        {
            //cout << "tupleExists: v=" << v << ",tuples.size()=" << tuples.size() << ",pos=" << pos << endl;

            if( pos < 0 || pos >= tuples.size() )
            {
                return false;
            }

            return tuples[pos].first == v;
        }

        /* Tries to insert a LabelSet ls into lss, thereby removing
         all supersets of ls in lss. It returns true if and only if
         ls was inserted somewhere in lss.

         For example, inserting { a } into { {a,b}, {a,c} } yields { {a} } in the end (true)
         whereas inserting {a,b} into {{a}, {b,c} } yields {{a}, {b,c} } in the end (false)
        */
        inline bool tryInsertLabelSet(LabelSet ls, vector< LabelSet >& lss)
        {

            //cout << "tryInsertLabelSet ls=" << labelSetToString(ls) << ",lss.size()=" << lss.size() << endl;
            bool hasReplaced = false;

            for(int i = 0; i < lss.size(); i++)
            {
                LabelSet ls2 = lss[i];
                bool b = isLabelSubset(ls, ls2);
                bool b2 = isLabelSubset(ls2, ls);

                //cout << "tryInsertLabelSet loop ls=" << graph->labelSetToString(ls) << ",ls2=" << graph->labelSetToString(ls2) << endl;
                //cout << "tryInsertLabelSet loop b=" << b << ",b2=" << b2 << endl;

                if( b == true && b2 == false )
                {
                    // inserted labelset is subset of existing
                    // which means replacement
                    // in general you do not want this
                    if( hasReplaced == false )
                    {
                        //cout << "tryInsertLabelSet1 lss[i]=" << graph->labelSetToString(lss[i]) << ",lss.size()=" << lss.size() << endl;
                        lss[i] = ls;
                        //cout << "tryInsertLabelSet2 lss[i]=" << graph->labelSetToString(lss[i]) << ",lss.size()=" << lss.size() << endl;
                        hasReplaced = true;
                    }
                    else
                    {
                        lss.erase( lss.begin() + i );
                        i--;
                    }
                }

                if( b2 == true )
                {
                    // ls is a superset of ls2 and hence redundant or the same as ls2
                    //cout << "tryInsertLabelSet false" << endl;
                    return false;
                }

            }

            // if we get here, then none of the labelsets in lss
            // is a sub- or superset of ls, hence it is a new element
            //cout << "tryInsertLabelSet true" << endl;
            if( hasReplaced == false )
                lss.push_back( ls );

            return true;
        };

        /*
        * Verifies whether there does not exist two label sets ls1, ls2 s.t.
        * ls1 is a sub- (or super-) set of the other.
        */
        inline bool isMinimalLabelSets(LabelSets& lss)
        {
            for(int i = 0; i < lss.size(); i++)
            {
                for(int j = 0; j < lss.size(); j++)
                {
                    if( i == j )
                        continue;

                    bool b1 = isLabelSubset(lss[i],lss[j]);

                    if( b1 == true )
                    {
                        return false;
                    }
                }
            }

            return true;
        }

        // ensures that for any two LabelSet ls1 and ls2 we have that
        // ls1 and ls2 are no sub- or superset of each other
        inline void minimizeLabelSets(LabelSets& lss)
        {
            for(int i = 0; i < lss.size(); i++)
            {
                for(int j = i + 1; j < lss.size(); j++)
                {
                    bool b1 = isLabelSubset( lss[i], lss[j] ); // j superset over i
                    bool b2 = isLabelSubset( lss[j], lss[i] ); // i superset of j

                    if( b1 == true )
                    {
                        lss.erase( lss.begin() + j );
                        j--;
                        if( j >= i )
                            i--;
                    }
                    else if( b2 == true )
                    {
                        lss.erase( lss.begin() + i );
                        j--;
                        i--;
                    }
                }
            }
        }

        inline void calculateLabelSetsDifference(LabelSets& left, LabelSets& right, LabelSets& result)
        {
            result.clear();
            for(int i = 0; i < left.size(); i++)
            {
                bool b1 = true;

                for(int j = 0; j < right.size(); j++)
                {
                    if( right[j] == left[i] )
                    {
                        b1 = false;
                        break;
                    }
                }

                if( b1 == true )
                {
                    //cout << left[i] << " ";
                    result.push_back( left[i] );
                }

            }
        }

        // constants for the index direction
        const int OUTINDEX = 0;
        const int ININDEX = 1;
        const int BOTHINDEX = 2;
        const int NONEINDEX = 3;
        const double TIMEOUT = 6.0 * 60.0 * 60.0; // maximal time (s) that index building
        // may require (6 hrs)

        const unsigned long MEM_LIMIT = 128000000000; // the maximum size of the index that can be in (128GB)
        // memory at a time

        const long MAX_VERTICES = sqrt(MEM_LIMIT)/30; // determines number of nodes per cluster and
        // maximal number of nodes before the index switches to non-blocked mode
    }

    class Index
    {
        protected:
            Graph* graph;
            indexns::IndexType indexType;
            int indexDirection; // says something about the 'direction' of the
            // index, e.g. 0 means an out-index, 1 means an in-index, 2 means
            // both and 3 means neither. BFS is 3, Exact is 1, ExactHop is 0
            // and Joindex is 2. This has to do with the structure of cIn
            //, for example cIn[v] = {w, 1000} means in Exact that w can reach v
            // with 1000 ('In') whereas in ExactHop  this means that v can reach
            // w with 1000 ('Out').

            double queryEndTime, queryStart;
            double constStartTime, constEndTime, totalConstTime;

            bool isBlockedMode; // tells whether the index uses cIn or tIn
            // for representing the index which depends on the graph instance
            // used as input of the index
            indexns::EIndex cIn;
            indexns::TuplesList tIn;

            string name;

            bool didComplete; // returns whether the idnex construction process completed or not

        public:
            virtual bool query(VertexID source, VertexID target, LabelSet ls) = 0;
            virtual unsigned long getIndexSizeInBytes() = 0;
            virtual void queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach) = 0;

            indexns::IndexType getIndexType()
            {
                return indexType;
            };

            int getIndexDirection()
            {
                return indexDirection;
            }

            string getIndexTypeAsString()
            {
                if( name.size() != 0 )
                {
                    return name;
                }

                string a = "none";
                switch(indexType)
                {
                    case indexns::IndexType::BFS:
                        a = "BFS";
                        break;
                    case indexns::IndexType::Exact:
                        a = "NeighbourExchange";
                        break;
                    case indexns::IndexType::DoubleBFSenum:
                        a = "DoubleBFS";
                        break;
                    case indexns::IndexType::Join:
                        a = "Join";
                        break;
                    case indexns::IndexType::Clustered:
                        a = "Clustered";
                        break;
                    case indexns::IndexType::Partial:
                        a = "Partial";
                        break;
                    case indexns::IndexType::Landmarked:
                        a = "Landmarked";
                        break;
                    case indexns::IndexType::ZouType:
                        a = "Zou";
                        break;
                    default:
                        break;
                }
                return a;
            }

            void unlabeledBFSWithDepth(VertexID v, int depth, vector<VertexID>& vertices)
            {
                vertices.clear();

                queue< pair<VertexID,int> > q;
                q.push( make_pair(v,0) );
                vector<VertexID> visited;

                while( q.empty() == false )
                {
                    VertexID w1 = q.front().first;
                    int depth1 = q.front().second;
                    q.pop();

                    if( find(visited.begin(), visited.end(), w1) != visited.end() )
                    {
                        continue;
                    }

                    visited.push_back( w1 );

                    if( depth1 == depth )
                    {
                        vertices.push_back( w1 );
                        continue;
                    }

                    SmallEdgeSet ses;
                    graph->getOutNeighbours(w1, ses);

                    for(int i = 0; i < ses.size(); i++)
                    {
                        VertexID w2 = ses[i].first;
                        q.push( make_pair(w2,depth1+1) );
                    }
                }
            }

            void labeledBFSWithMinDepth(VertexID v, LabelSet& ls, int minDepth, vector<VertexID>& vertices)
            {
                // This method finds all vertices from v using labelset ls with at least minDepth steps from v
                vertices.clear();

                queue< pair<VertexID,int> > q;
                q.push( make_pair(v,0) );
                vector<VertexID> visited;

                while( q.empty() == false )
                {
                    VertexID w1 = q.front().first;
                    int depth1 = q.front().second;
                    q.pop();

                    if( find(visited.begin(), visited.end(), w1) != visited.end() )
                    {
                        continue;
                    }

                    visited.push_back( w1 );

                    if( depth1 >= minDepth )
                    {
                        vertices.push_back( w1 );
                    }

                    SmallEdgeSet ses;
                    graph->getOutNeighbours(w1, ses);

                    for(int i = 0; i < ses.size(); i++)
                    {
                        VertexID w2 = ses[i].first;
                        LabelSet ls2 = ses[i].second;
                        bool b2 = isLabelSubset(ls2, ls);

                        if( b2 == true )
                        {
                            q.push( make_pair(w2,depth1+1) );
                        }
                    }
                }
            }

            double getLastQueryTime()
            {
                // the minimal delta is 5e-8
                double delta = max(queryEndTime - queryStart, 0.0000001);
                return delta;
            };

            double getIndexConstructionTimeInSec()
            {
                /* This used to be constEndTime-constStartTime */

                return max(totalConstTime, 0.0000001) + graph->getGraphConstructionTime();
            };

            // checks whether for any pair (v,w) ind[v][w] does not contain
            // duplicate labelsets i.e. no ls1 and ls2 s.t. ls1 is a subset of ls2
            bool isMinimalIndex()
            {
                int N = graph->getNumberOfVertices();

                if( isBlockedMode == true )
                {
                    for(int i = 0; i < cIn.size(); i++)
                    {
                        for(int j = 0; j < cIn[i].size(); j++)
                        {
                            bool b1 = indexns::isMinimalLabelSets(cIn[i][j]);
                            if( b1 == false )
                            {
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    for(int i = 0; i < tIn.size(); i++)
                    {
                        for(int j = 0; j < tIn[i].size(); j++)
                        {
                            indexns::LabelSets lss = tIn[i][j].second;
                            bool b1 = indexns::isMinimalLabelSets(lss);
                            if( b1 == false )
                            {
                                return false;
                            }
                        }
                    }
                }

                return true;
            }

            /**
            * Tries to insert the entry (v,ls) to the index at position w. It can
            fail if (v,ls) is already covered by another entry (v,ls'), i.e.
            ls' is a subset of ls.
            */
            bool tryInsertLabelSetToIndex(LabelSet ls, VertexID w, VertexID v)
            {
                bool b = true;
                if( isBlockedMode == true )
                {
                    b = indexns::tryInsertLabelSet(ls, cIn[w][v]);
                }
                else
                {
                    int pos = 0;
                    b = indexns::findTupleInTuples(v, tIn[w], pos);

                    if( b == false )
                    {
                        // entry was not found and should be inserted
                        indexns::LabelSets lss = indexns::LabelSets();
                        lss.reserve( graph->getNumberOfLabels() * 2 );
                        indexns::Tuple newTuple = make_pair(v, lss );
                        tIn[w].insert( tIn[w].begin() + pos, newTuple );
                    }

                    b = indexns::tryInsertLabelSet(ls, tIn[w][pos].second);
                }

                return b;
            }

            string toString()
            {
                string s = "";
                int N = graph->getNumberOfVertices();

                if( isBlockedMode == true )
                {
                    for(int i = 0; i < cIn.size(); i++)
                    {
                        s += "v=" + to_string(i) + "\n{\n";
                        for(int j = 0; j < cIn[i].size(); j++)
                        {
                            if( cIn[i][j].size() == 0 )
                                continue;

                            s += to_string(j) + "\n";
                            for(int k = 0; k < cIn[i][j].size(); k++)
                            {
                                s += labelSetToString(cIn[i][j][k]) + "\n";
                            }
                        }
                        s += "} \n";
                    }
                }
                else
                {
                    for(int i = 0; i < tIn.size(); i++)
                    {
                        s += "v=" + to_string(i) + "\n{\n";
                        for(int j = 0; j < tIn[i].size(); j++)
                        {
                            indexns::Tuple tu = tIn[i][j];
                            if( tu.second.size() == 0 )
                                continue;

                            s += to_string(tu.first) + "\n";
                            for(int k = 0; k < tu.second.size(); k++)
                            {
                                s += labelSetToString(tu.second[k]) + "\n";
                            }
                        }
                        s += "} \n";
                    }
                }


                return s;
            };

            unsigned long getIndexSizeInBytesM()
            {
                unsigned long size = 0;
                int N = graph->getNumberOfVertices();
                int emptyVectorSize = 3 * sizeof(int); // estimated 3 times 32-bits
                // for the pointer and the capacity and the size

                for(int i = 0; i < N; i++)
                {
                    size += getIndexSizeInBytesM(i);
                }

                size += graph->getGraphSizeInBytes();
                return size;
            };

            unsigned long getIndexSizeInBytesM(VertexID v)
            {
                unsigned long size = 0;
                int L = sizeof(LabelSet);
                int N = graph->getNumberOfVertices();
                int emptyVectorSize = 3 * sizeof(int); // estimated 3 times 32-bits
                // for the pointer and the capacity and the size
                indexns::LabelSets lss;

                if( isBlockedMode == true )
                {
                    if( cIn.size() <= v )
                    {
                        return size;
                    }

                    for(int w = 0; w < cIn[v].size(); w++)
                    {
                        getLabelSetsPerPair(v,w,lss);
                        size += lss.size() * L + emptyVectorSize;
                    }
                }
                else
                {
                    if( tIn.size() <= v )
                    {
                        return size;
                    }

                    for(int w = 0; w < tIn[v].size(); w++)
                    {
                        getLabelSetsPerPair(v,w,lss);
                        if( lss.size() == 0 )
                        {
                            continue;
                        }
                        size += sizeof(VertexID);
                        size += lss.size() * L + emptyVectorSize;
                    }
                }

                return size;
            };

            void getLabelSetsPerPair(VertexID source, VertexID target, indexns::LabelSets& lss)
            {
                lss.clear();

                if( isBlockedMode == true )
                {
                    if( cIn.size() <= source )
                    {
                        return;
                    }

                    if( cIn[source].size() <= target )
                    {
                        return;
                    }

                    lss = cIn[source][target];
                }
                else
                {
                    int pos = 0;
                    bool b = indexns::findTupleInTuples(target, tIn[source], pos);
                    if( b == true )
                    {
                        lss = tIn[source][pos].second;
                    }
                }
            }

            void initializeIndex(int size)
            {
                int N = graph->getNumberOfVertices();
                int L = graph->getNumberOfLabels();

                if( isBlockedMode == true )
                {
                    // Initialize the cIn
                    cIn = indexns::EIndex();

                    for(int i = 0; i < size; i++)
                    {
                        indexns::LabelSetsPerNode LssPN = indexns::LabelSetsPerNode();

                        for(int j = 0; j < N; j++)
                        {
                            indexns::LabelSets Lss = indexns::LabelSets();
                            Lss.reserve(L*2);
                            LssPN.push_back(Lss);
                        }

                        cIn.push_back(LssPN);
                    }
                }
                else
                {
                    // Initialize tIn
                    tIn = indexns::TuplesList();

                    for(int i = 0; i < size; i++)
                    {
                        indexns::Tuples tus = indexns::Tuples();
                        tIn.push_back(tus);
                    }
                }

                cout << getIndexTypeAsString() << "-Index initialized" << endl;

            }

            void initializeIndex()
            {
                int N = graph->getNumberOfVertices();
                int L = graph->getNumberOfLabels();

                initializeIndex(N);
            }

            void setEntry(VertexID v, VertexID w, indexns::LabelSets& lss)
            {
                if( isBlockedMode == true )
                {
                    cIn[v][w].clear();
                    for(int i = 0; i < lss.size(); i++)
                    {
                        cIn[v][w].push_back( lss[i] );
                    }
                }
                else
                {
                    int pos = 0;
                    bool b = indexns::findTupleInTuples(w, tIn[v], pos);
                    if( b == false )
                    {
                        indexns::LabelSets lss = indexns::LabelSets();
                        lss.reserve( graph->getNumberOfLabels() * 2 );
                        indexns::Tuple newTuple = make_pair(w, lss );
                        tIn[v].insert( tIn[v].begin() + pos, newTuple );
                    }
                    else
                    {
                        tIn[v][pos].second = lss;
                    }
                }
            }

            void indexShrinkToFit()
            {
                int N = graph->getNumberOfVertices();
                int L = graph->getNumberOfLabels();

                indexns::LabelSets lss;

                if( isBlockedMode == true )
                {
                    for(int v = 0; v < N; v++)
                    {
                        if( cIn.size() <= v )
                        {
                            return;
                        }

                        for(int w = 0; w < cIn[v].size(); w++)
                        {
                            getLabelSetsPerPair(v,w,lss);
                            lss.shrink_to_fit();
                        }
                    }
                }
                else
                {
                    for(int v = 0; v < N; v++)
                    {
                        if( tIn.size() <= v )
                        {
                            return;
                        }

                        for(int w = 0; w < tIn[v].size(); w++)
                        {
                            getLabelSetsPerPair(v,w,lss);
                            lss.shrink_to_fit();
                        }
                    }
                }
            }

            bool isBlockedModePredicate(Graph* mg)
            {
                return mg->getNumberOfVertices() <= indexns::MAX_VERTICES || (mg->getNumberOfEdges() / mg->getNumberOfVertices() >= 6);
            }

            void deconstruct()
            {
                if( this->isBlockedMode == true )
                {
                    this->cIn.clear();
                }
                else
                {
                    this->tIn.clear();
                }
            }

            void indexToFile(string outputFileName)
            {
                int N = graph->getNumberOfVertices();
                vector< VertexID > vertices;
                for(int i = 0; i < N; i++)
                {
                    vertices.push_back(i);
                }
                indexToFile(outputFileName, vertices);
            }

            void indexToFile(string outputFileName, vector< VertexID >& vertices)
            {
                int N = graph->getNumberOfVertices();

                fstream outputFile;
                outputFile.open (outputFileName, std::fstream::in | std::fstream::out | std::ofstream::trunc);
                if (outputFile.is_open())
                {
                    for(int i = 0; i < vertices.size(); i++)
                    {
                        VertexID v = vertices[i];
                        outputFile << v << "[";
                        for(int w = 0; w < N; w++)
                        {
                            indexns::LabelSets lss;
                            getLabelSetsPerPair(v,w,lss);
                            if( lss.size() == 0 || v == w )
                            {
                                continue;
                            }

                            outputFile << w << "(";

                            for(int k = 0; k < lss.size(); k++)
                            {
                                outputFile << lss[k];

                                if( k < lss.size()-1 )
                                {
                                    outputFile << ",";
                                }
                            }

                            outputFile << ")";
                        }
                        outputFile << "]" << endl;
                    }

                    outputFile.flush();
                    outputFile.close();
                }
                else
                {
                    cerr << "indexToFile could not create file=" << outputFileName << endl;
                }
            }

            void indexFromFile(string inputFileName)
            {
                // to be implemented
            };

            /*
            * Returns how often the label set ls occurs in the index
            */
            int countLabel(LabelSet ls)
            {
                if( isBlockedMode == true )
                    return 0;

                int count = 0;
                int total = 0;
                for(int i = 0; i < tIn.size(); i++)
                {
                    for(int j = 0; j < tIn[i].size(); j++)
                    {
                        for(int k = 0; k < tIn[i][j].second.size(); k++)
                        {
                            if( tIn[i][j].second[k] == ls )
                            {
                                count++;
                            }
                            total++;
                        }
                    }
                }

                cout << "ls=" << ls << " , count=" << count << " , total=" << total << endl;

                return count;
            }

            /*
            * Deletes (v,ls) from Ind(w)
            * Assumes non-blocked mode
            */
            void deleteEntry(VertexID v, LabelSet ls, VertexID w)
            {
                int pos = 0;
                bool b1 = indexns::findTupleInTuples(v, tIn[w], pos);

                //cout << "deleteEntry (v=" << v << ",ls=" << ls << "), Ind(w)=" << w << ",pos=" << pos << ",b1=" << b1 << endl;

                if( b1 == false || pos < 0 || pos >= tIn[w].size() )
                    return;

                for(int i = 0; i < tIn[w][pos].second.size(); i++)
                {
                    if( tIn[w][pos].second[i] == ls )
                    {
                        cout << "deleteEntry delete (v=" << v << ",ls=" << ls << "), Ind(w)=" << w << endl;
                        tIn[w][pos].second.erase( tIn[w][pos].second.begin() + i );
                        i--;
                    }
                }

            }

            bool didCompleteBuilding()
            {
                return this->didComplete;
            }

    };
#endif
