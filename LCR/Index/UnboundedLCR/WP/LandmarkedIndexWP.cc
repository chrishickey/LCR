#include "IndexWP.h"
#include "LandmarkedIndexWP.h"

#include <random>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <curses.h>
#include <time.h>
#include <chrono>
#include <thread>

#include <queue>
#include <iostream>

using namespace graphns;
using namespace indexwpns;

LandmarkedWP::LandmarkedWP(Graph* graph, int noOfLandmarks)
{
    this->graph = graph;
    this->N = graph->getNumberOfVertices();
    this->M = graph->getNumberOfEdges();
    this->L = graph->getNumberOfLabels();

    this->noOfLandmarks = noOfLandmarks;

    buildIndex();

    cout << "LandmarkedWP index construction time(s)=" << getIndexConstructionTimeInSec() << endl;
};

LandmarkedWP::LandmarkedWP(Graph* graph) : LandmarkedWP(graph, graph->getNumberOfVertices() / 2 ) // (int) sqrt(graph->getNumberOfVertices())
{

}

void LandmarkedWP::buildIndex()
{
    cStart = getCurrentTimeInMilliSec();

    int quotum = 1;
    long Esize = 0; // estimated size
    hasBeenIndexed = dynamic_bitset<>(N);
    determineLandmarks();

    initializeIndex();

    for(int i = 0; i < N; i++)
    {
        bool isLandmark = vToLandmark[i] != -1;
        if( isLandmark )
            buildIndex(i);
        else
            //

        if( (i+1)%quotum == 0 )
            cout << "i=" << i << " perc=" << ( ( double (i+1) ) / N ) << " Esize=" << Esize << " ,isLandmark=" << isLandmark << endl;
    }

    cEnd = getCurrentTimeInMilliSec();
};

int LandmarkedWP::buildIndex(VertexID w)
{
    priority_queue< HeapPathEntry, vector<HeapPathEntry>, HeapPathOrdering > pq;

    Path p = Path();
    addVertexToPath(w , p);
    HeapPathEntry hpe;
    hpe.to = w;
    hpe.dist = 0;
    hpe.lp = make_pair( 0 , p );
    pq.push( hpe );

    int roundNo = 0;

    while( pq.empty() == false )
    {
        roundNo++;
        HeapPathEntry hpe1 = pq.top();
        VertexID v1 = hpe1.to;
        LabelSet ls1 = hpe1.lp.first;
        Path p1 = hpe1.lp.second;
        pq.pop();

        //cout << "buildIndex v1=" << v1 << endl;

        if( w != v1 )
        {
            indexwpns::LP lp1 = make_pair(ls1, p1);
            bool b = tryInsert(w, v1, lp1);
            if( b == false )
            {
                continue;
            }
        }

        if( hasBeenIndexed[v1] == 1 && vToLandmark[v1] != -1 )
        {
            // v1 has been indexed and is a landmark
            // we can copy all entries from v1 and do not need to get further here
            for(int i = 0; i < ifn[v1].size(); i++)
            {
                VertexID y = ifn[v1][i].to;
                if( y == w )
                    continue;

                for(int j = 0; j < ifn[v1][i].entries.size(); j++)
                {
                    LP lp2 = ifn[v1][i].entries[j];
                    Path p2;
                    concatenatePaths(p1, lp2.second, p2);
                    lp2 = make_pair( joinLabelSets(ls1, lp2.first), p2  );

                    tryInsert(v1, y, lp2);
                }
            }

            continue;
        }

        SmallEdgeSet ses;
        this->graph->getOutNeighbours(v1, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            // push the entries
            HeapPathEntry hpe2;
            hpe2.to = ses[j].first;
            Path p2 = p1;
            addVertexToPath(ses[j].first, p2);
            LabelSet ls2 = joinLabelSets(ses[j].second, ls1);
            hpe2.lp = make_pair( ls2 , p2 );
            hpe2.dist = getNumberOfLabelsInLabelSet( ls2 );

            pq.push( hpe2 );
        }
    }

    return roundNo;
};

bool LandmarkedWP::tryInsert(VertexID w, VertexID v, indexwpns::LP& lp)
{
    if( w == v )
    {
        return false;
    }

    bool b2 = tryInsertLPToIndex(lp, w, v);
    return b2;
}

void LandmarkedWP::determineLandmarks()
{
    // pick the noOfLandmarks nodes with the highest
    // total-degree
    vector< pair< VertexID, int > > totDegreePerNode;
    vToLandmark = vector< int >(N,-1);

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses;
        graph->getAllNeighbours(i, ses);
        totDegreePerNode.push_back( make_pair(i , ses.size() ));
    }

    struct sort_pred {
        bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
            return left.second > right.second;
        }
    };

    sort(totDegreePerNode.begin(), totDegreePerNode.end(), sort_pred() );

    for(int i = 0; i < noOfLandmarks; i++)
    {
        //cout << "v=" << totDegreePerNode[i].first << " , #=" << totDegreePerNode[i].second << endl;
        landmarks.push_back( totDegreePerNode[i].first );
        vToLandmark[ totDegreePerNode[i].first ] = i;
    }

    cout << "LandmarkedIndex::determineLandmarks noOfLandmarks=" << noOfLandmarks << " ,method=" << 1 << endl;
};

LandmarkedWP::~LandmarkedWP()
{

};

long LandmarkedWP::getIndexSizeInBytes()
{

};

long LandmarkedWP::getIndexSizeInBytes(VertexID v)
{

};

bool LandmarkedWP::queryNP(VertexID v, VertexID w, LabelSet ls)
{
    if( ls == 0 )
    {
        qStart = getCurrentTimeInMilliSec();
        qEnd = qStart;
        return false;
    }

    if( v == w )
    {
        qStart = getCurrentTimeInMilliSec();
        qEnd = qStart;
        return true;
    }

    qStart = getCurrentTimeInMilliSec();
    bool b = true;
    if( vToLandmark[v] != -1 )
    {
        b = queryDirect(v,w,ls);
    }
    else
    {

    }

    qEnd = getCurrentTimeInMilliSec();
};

bool LandmarkedWP::queryWP(VertexID v, VertexID w, LabelSet ls, Path& p)
{
    p.clear();
    if( ls == 0 )
    {
        qStart = getCurrentTimeInMilliSec();
        qEnd = qStart;
        return false;
    }

    if( v == w )
    {
        qStart = getCurrentTimeInMilliSec();
        qEnd = qStart;
        return true;
    }

    qStart = getCurrentTimeInMilliSec();
    bool b = true;
    if( vToLandmark[v] != -1 )
    {
        // a landmark
        b = queryDirect(v,w,ls,p);
    }
    else
    {

    }

    qEnd = getCurrentTimeInMilliSec();
    return true;
};

bool LandmarkedWP::queryDirect(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p)
{
    //cout << "queryDirect v=" << v << " ,w=" << w << " ,ls=" << ls << endl;

    int pos = 0;
    bool b = findTupleInTuples(w, ifn[v], pos);
    if( b == true )
    {
        for(int i = 0; i < ifn[v][pos].entries.size(); i++)
        {
            LP lp = ifn[v][pos].entries[i];
            //cout << "queryDirect lp.first=" << lp.first << endl;
            if( isLabelSubset(lp.first, ls) == true )
            {
                p = lp.second;
                return true;
            }
        }
    }

    return false;
};

bool LandmarkedWP::queryDirect(VertexID v, VertexID w, LabelSet ls)
{
    int pos = 0;
    bool b = findTupleInTuples(w, ifn[v], pos);
    if( b == true )
    {
        for(int i = 0; i < ifn[v][pos].entries.size(); i++)
        {
            LP lp = ifn[v][pos].entries[i];
            if( isLabelSubset(lp.first, ls) == true )
            {
                return true;
            }
        }
    }

    return false;
};

bool LandmarkedWP::queryShell(VertexID v, VertexID w, LabelSet ls, indexwpns::Path& p)
{

};
