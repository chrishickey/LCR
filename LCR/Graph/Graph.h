/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).

Graph.h is an interface any graph that needs to be extended from here.
graphns has a namespace that defines VertexID, LabelSet

A LabelSet is an unsigned integer (or short, char or long) that indicates by its
bits which labels are present, e.g. {a, b} = 00000011

A LabelID is an unsigned integer taken from a graph file. For instance,
1 2 0 indicates an edge from 1 to 2 using label a

LabelID LabelSet
0       1 == a
1       2 == b
2       4 == c
3       8 == d

{a , c , d} has a LabelSet with value 8 + 4 + 1 = 13
*/

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

#include <sys/time.h>

#include <boost/dynamic_bitset.hpp>

using namespace std;
using namespace boost;

#ifndef GRAPH_H
#define GRAPH_H

    namespace graphns
    {
        // ideally this should be in a config file
        const int BIGENDIAN = 1;
        const int LITTLEENDIAN = 0;
        const int endianness = LITTLEENDIAN;

        const int JOINFLAG = 0x8000; // assumes LabelSet is short (2 bytes)

        // Vertex IDs (max size)
        typedef unsigned int VertexID;
        // Edge label IDs (max size)
        typedef unsigned int LabelID;

        // An edge
        // NOTE: gcc on Mac OS doesn't understand "tuple", i.e., the following doesn't work:
        // typedef std::tuple < VertexID, LabelID, VertexID > Edge;
        // fix this later.
        // first ID is the source, second ID is the target, LabelID is edge label
        typedef pair < VertexID, pair < VertexID, LabelID > > Edge;
        // List of edges
        typedef vector < Edge > EdgeSet;

        // A label set is an int in which ocurrence of a label is
        // a binary, e.g. acd = 00000000 00001101.
        // Note that
        // LabelID (1) -> LabelSet (00000001)
        // LabelID (2) -> LabelSet (00000010)
        typedef unsigned int LabelSet;

        // A small edge is a single pair (w,l) in which l is a LabelSet
        typedef pair < VertexID, LabelSet > SmallEdge;
        // List of small edges
        typedef vector < SmallEdge > SmallEdgeSet;

        typedef vector< vector < SmallEdge > > SmallEdgeSets;

        // A block is an array of char (1 byte) belonging to a node v
        // in which tuples (w,l) reside indicating an edge (v,w,l) exists
        typedef vector< char > Block;
        // List of blocks
        typedef vector< Block > BlockList;

        // timing from Akiba
        inline double getCurrentTimeInMilliSec() {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return tv.tv_sec + tv.tv_usec * 1e-6;
        }
        // the end

        string print_digits(double d, int precision)
        {
            if( d < (1.0/(pow(10.0,precision))) )
            {
                d = (1.0/(pow(10.0,precision)));
                std::stringstream s;
                s << std::fixed << std::setprecision(std::numeric_limits<double>::digits10) << d;
                std::string res = s.str();

                return "<" + res;
            }

            std::stringstream s;
            s << std::fixed << std::setprecision(std::numeric_limits<double>::digits10) << d;
            std::string res = s.str();

            size_t dotIndex = res.find(".");
            std::string final_res = res.substr(0, min(dotIndex + precision + 1, res.length()));

            // add _ to e.g. 1000.0 -> 1_000.0
            // 1_000_000.0
            int i = dotIndex - 3;
            while(i >= 1)
            {
                final_res = final_res.substr(0,i) + "_" + final_res.substr(i,final_res.length());
                i -= 3;
            }

            return final_res;
        }

        inline unsigned char getUnsignedChar(LabelSet ls1, int i)
        {
            unsigned char c = 0;

            if( endianness == LITTLEENDIAN )
            {
                c = (ls1 >> (i*8));
            }
            else
            {
                c = (ls1 >> ((sizeof(ls1)-1-i)*8));
            }

            //cout << "c=" << (int) c << endl;

            return c;
        }

        inline void setUnsignedChar(LabelSet& ls1, int i, unsigned char c)
        {
            if( endianness == LITTLEENDIAN )
            {
                for(int j = 0; j < 8; j++)
                {
                    int bit = (c >> j) & 1;
                    if( bit == 1)
                    {
                        ls1 |= (1 << (i*8 + j) );
                    }
                    else
                    {
                        ls1 &= ~(1 << (i*8 + j) );
                    }
                }
            }
            else
            {
                for(int j = 0; j < 8; j++)
                {

                }
            }

            //cout << "ls1=" << ls1 << endl;
        }

        /*
        Verifies whether ls1 is a subset of ls2
        */
        inline bool isLabelSubset(LabelSet ls1, LabelSet ls2)
        {

            return ((ls1 & ls2) == ls1);
        };

        inline bool isLabelEqual(LabelSet ls1, LabelSet ls2)
        {
            /* verifies whether ls1 and ls2 are equal
            */
            return ls1 == ls2;
        }

        /*
        Verifies whether ls1 is an empty labelset
        */
        inline bool isEmptyLabelSet(LabelSet ls1)
        {
            return ls1 == 0;
        }

        /*
        Returns the number of labels in ls1, e.g. if ls1 = { a, b} then this
        method returns 2
        */
        inline int getNumberOfLabelsInLabelSet(LabelSet ls1)
        {
            int size = 0;
            for(int i = 0; i < sizeof(ls1); i++)
            {
                unsigned char c = getUnsignedChar(ls1, i);

                if( c == 0 )
                    continue;

                for(int j = 0; j < 8; j++)
                {
                    int bit = (c >> j) & 1;
                    //cout << "bit=" << bit << endl;
                    if( bit == 1 )
                    {
                        size += 1;
                    }
                }
            }

            return size;
        };

        /*
        For a labelset ls return the set of label ID's in ls.
        E.g. {a,c} returns a vector < 0, 2 >
        */
        inline void getLabelIDsFromLabelSet(LabelSet ls1, vector< LabelID >& lVec)
        {
            lVec.clear();
            for(int i = 0; i < sizeof(ls1); i++)
            {
                unsigned char c = getUnsignedChar(ls1, i);

                for(int j = 0; j < 8; j++)
                {
                    int bit = (c >> j) & 1;
                    //cout << "bit=" << bit << endl;
                    if( bit == 1 )
                    {
                        lVec.push_back( i*8 + j );
                    }
                }
            }
        }

        /*
        Indicates whether the number of labels in ls1 is 1
        */
        inline bool isSingular(LabelSet ls1)
        {
            return getNumberOfLabelsInLabelSet(ls1) == 1;
        }

        // prints a labelset as a string, e.g. the first label is
        // 00000001
        inline string labelSetToString(LabelSet ls1)
        {
            string a = "";
            for(int i = sizeof(ls1)-1; i >= 0; i--)
            {
                unsigned char c = getUnsignedChar(ls1, i);

                for(int j = 7; j >= 0; j--)
                {
                    int bit = (c >> j) & 1;
                    //cout << "bit=" << bit << endl;
                    a += to_string(bit);
                }
            }

            return a;
        }

        /*
        Translates a LabelSet to a LabelID (see description on top)
        */
        inline LabelID labelSetToLabelID(LabelSet ls1)
        {
            // This method translates a LabelSet into a LabelID
            // LabelID -> LabelSet
            // 0 -> {a} 1
            // 1 -> {b} 2
            // 2 -> {c} 4
            // 3 -> {d} 8

            if( isSingular(ls1) == false || ls1 == 0 )
            {
                cerr << "ERROR: labelSetToLabelID non-singular or zero ls1=" << ls1 << endl;
                return 0;
            }

            for(int i = 0; i < sizeof(ls1); i++)
            {
                unsigned char c = getUnsignedChar(ls1, i);

                for(int j = 7; j >= 0; j--)
                {
                    int bit = (c >> j) & 1;

                    if( bit == 1 )
                    {
                        return (i*8) + j;
                    }
                }
            }

            return 0;
        }

        /*
        Translates a LabelID to a LabelSet (see top description)
        e.g. LabelID 0 is LabelSet 1 and LabelID 1 is LabelSet 2
        */
        inline LabelSet labelIDToLabelSet(LabelID id)
        {
            // used by a graph to change labelIDs from input to a Labelset
            // e.g. (LabelID) 0 -> (LabelSet) 0001
            return (1 << id);
        }

        /*
        Writes down a singular labelset to a letter, e.g.
        LabelSet 1 is "a".
        */
        inline string labelSetToLetter(LabelSet ls1)
        {
            if( isSingular(ls1) == false )
            {
                cerr << "labelSetToLetter ls1" << endl;
                return "?";
            }

            string a = "?";
            string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            LabelID id = labelSetToLabelID(ls1);

            if( id >= 0 && id <= alphabet.size()-1 )
            {
                a = alphabet[id];
            }

            //cout << "a=" << a << endl;

            return a;
        }

        /*
        Inverts the bits of a labelset, e.g. 1101 -> 0010
        Is this still used?
        */
        inline LabelSet invertLabelSet(LabelSet& ls1, int L)
        {
            for(int i = 0; i < sizeof(ls1); i++)
            {
                unsigned char c = getUnsignedChar(ls1, i);

                int j = 0;
                while( j < 8 && ((i*8)+j) < L )
                {
                    int bit = (c >> j) & 1;

                    if( bit == 1 )
                    {
                        c ^= (-0 ^ c) & (1 << j);
                    }
                    else
                    {
                        c |= 1 << j;
                    }

                    j++;
                }

                //cout << "c=" << (int) c << endl;
                setUnsignedChar(ls1, i, c);
            }

            return ls1;
        }

        /*
        Takes the union of two labelsets
        */
        inline LabelSet joinLabelSets(LabelSet& ls1, LabelSet& ls2)
        {
            return ls1 | ls2;
        }

        /*
        Takes the intersection of two labelset
        */
        inline LabelSet intersectLabelSets(LabelSet& ls1, LabelSet& ls2)
        {
            return ls1 & ls2;
        }

        /*
        Takes the Hamming Distance, i.e. the number of labels in the intersection
        */
        inline int getHammingDistance(LabelSet& ls1, LabelSet& ls2)
        {
            int distance = 0;

            for(int i = 0; i < sizeof(ls1); i++)
            {
                for(int j = 0; j < 8; j++)
                {
                    int lBit = (ls1 >> j) & 1;
                    int rBit = (ls2 >> j) & 1;
                    //cout << "lBit=" << lBit << ",rBit=" << rBit << endl;
                    if( lBit != rBit )
                        distance += 1;
                }

            }

            return distance;
        }

        inline LabelSet setJoinFlag(LabelSet ls)
        {
            ls |= JOINFLAG;
            return ls;
        }

        inline bool hasJoinFlag(LabelSet& ls)
        {
            return (ls & JOINFLAG) == JOINFLAG;
        }

        inline LabelSet removeJoinFlag(LabelSet ls)
        {
            ls &= ~(JOINFLAG);
            return ls;
        }

        inline void setLabelInLabelSet(LabelSet& ls, int j, int bit)
        {
            ls ^= (-bit ^ ls) & (1 << j);
        }


    }

    /**
    * This class is an interface for specific graph implementations.
    *
    * We assume that:
    * - the vertices are numbered 0 ... N-1
    * - the labels are numbered 0 ... L-1 (which means 0 is the first label). Hence
    *   LabelID=0 is the first label, whereas LabelSet=0000 is an empty labelset
    *
    */
    class Graph
    {
        protected:
            int N, M, L; // N, M and L are the number of vertices, edges and labels

            vector< long > countPerLabel;

        public:

            virtual void buildGraph(graphns::EdgeSet* edges) = 0;
            virtual graphns::EdgeSet* loadEdgeFile(string fileName) = 0;
            virtual void loadEdgeStats(graphns::EdgeSet* edgeSet) = 0;

            virtual int getGraphSizeInBytes() = 0;
            virtual double getGraphConstructionTime() = 0;

            virtual void getOutNeighbours(graphns::VertexID w, graphns::SmallEdgeSet& outNeighbours) = 0;
            virtual void getInNeighbours(graphns::VertexID w, graphns::SmallEdgeSet& outNeighbours) = 0;
            virtual void getAllNeighbours(graphns::VertexID w, graphns::SmallEdgeSet& allNeighbours) = 0;

            virtual void addNode() = 0;
            virtual void removeNode(graphns::VertexID w) = 0;
            virtual void addEdge(graphns::VertexID v, graphns::VertexID w, graphns::LabelID newLabel) = 0;
            virtual void addMultiEdge(graphns::VertexID v, graphns::VertexID w, graphns::LabelSet newLabelSet) = 0;
            virtual void removeEdge(graphns::VertexID v, graphns::VertexID w) = 0;
            virtual void changeLabel(graphns::VertexID v, graphns::VertexID w, graphns::LabelID) = 0;
            virtual bool hasEdge(graphns::VertexID v , graphns::VertexID w) = 0;
            virtual bool hasMultiEdge(graphns::VertexID v , graphns::VertexID w, graphns::LabelSet ls) = 0;
            virtual graphns::LabelID getLabelID(graphns::VertexID v , graphns::VertexID w) = 0;

            virtual long getCountPerLabel(graphns::LabelID l) = 0;

            virtual void tarjan(vector< vector< graphns::VertexID > >& SCCs) = 0;

            // prints stats of the graph
            virtual std::string toString() = 0;

            virtual int getNumberOfVertices() = 0;
            virtual int getNumberOfLabels() = 0;
            virtual int getNumberOfEdges() = 0;

            void DijkstraSSP(graphns::VertexID v1, vector< int >& distances)
            {
                distances.clear();

                for(int i = 0; i < N; i++)
                {
                    distances.push_back(-1); // no reachable vertex can have a distance
                    // equal to N*N
                }

                dynamic_bitset<> marked = dynamic_bitset<>(N);
                queue< pair<graphns::VertexID, int> > q;
                q.push( make_pair(v1,0) );
                while( q.empty() == false )
                {
                    graphns::VertexID v = q.front().first;
                    int dist = q.front().second;
                    q.pop();

                    if( marked[v] == 1 )
                    {
                        continue;
                    }
                    marked[v] = 1;
                    distances[v] = dist;
                    //cout << "v1=" << v1 << ",v=" << v << ",dist=" << dist << endl;

                    graphns::SmallEdgeSet ses;
                    getOutNeighbours(v, ses);
                    for(int i = 0; i < ses.size(); i++)
                    {
                        graphns::VertexID w = ses[i].first;
                        q.push( make_pair(w, dist+1) );
                    }
                }
            }

            int findLongestShortestPath(graphns::VertexID v1)
            {
                dynamic_bitset<> marked = dynamic_bitset<>(N);
                queue< pair<graphns::VertexID, int> > q;
                q.push( make_pair(v1,0) );
                int maxDiameter = 0;
                while( q.empty() == false )
                {
                    graphns::VertexID v = q.front().first;
                    int dist = q.front().second;
                    q.pop();

                    if( marked[v] == 1 )
                    {
                        continue;
                    }
                    marked[v] = 1;
                    maxDiameter = max(maxDiameter, dist);

                    graphns::SmallEdgeSet ses;
                    getOutNeighbours(v, ses);
                    for(int i = 0; i < ses.size(); i++)
                    {
                        graphns::VertexID w = ses[i].first;
                        q.push( make_pair(w, dist+1) );
                    }
                }

                return maxDiameter;
            }

            bool topologicalSort(vector< graphns::VertexID >& ordering)
            {
                ordering.clear();
                dynamic_bitset<> marked = dynamic_bitset<>(N);
                vector<int> countPerVertex = vector<int>(N);


                // first compute number of in edges per node
                graphns::SmallEdgeSet ses1, ses2, ses;
                for(int i = 0; i < N; i++)
                {
                    getOutNeighbours(i, ses1);
                    getInNeighbours(i, ses2);

                    if( ses1.size() == 0 && ses2.size() == 0 )
                    {
                        ordering.push_back(i);
                        countPerVertex[i] = -1;
                        continue;
                    }

                    int c = 0;
                    for(int j = 0; j < ses2.size(); j++)
                    {
                        if( i != ses2[j].first ) // disregard self-edges
                        {
                            c++;
                        }
                    }

                    countPerVertex[i] = c;

                    //cout << "topologicalSort: i=" << i << " ,c=" << c << endl;
                }

                bool hasChanged = true;
                while( hasChanged == true )
                {
                    hasChanged = false;

                    for(int i = 0; i < N; i++)
                    {
                        if( countPerVertex[i] == 0 )
                        {
                            //cout << "add i=" << i << endl;
                            ordering.push_back(i);
                            getOutNeighbours(i, ses);

                            for(int j = 0; j < ses.size(); j++)
                            {
                                //cout << "i=" << i << " ,ses[j].first=" << ses[j].first << endl;
                                countPerVertex[ ses[j].first ]--;
                            }
                            countPerVertex[i] = -1;

                            hasChanged = true;
                        }
                    }
                }

                // no topological sort available
                // there exists at least one cycle
                if( ordering.size() < N )
                {
                    return false;
                }

                return true;
            };

    };

#endif
