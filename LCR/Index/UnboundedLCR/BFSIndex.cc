#include "BFSIndex.h"

#include <iostream>
#include <queue>
#include <set>

using namespace std;
using namespace indexns;

BFSIndex::BFSIndex(Graph* mg)
{
    constStartTime = getCurrentTimeInMilliSec();
    graph = mg;
    indexType = IndexType::BFS;
    buildIndex();

    queryStart = 0.0;
    queryEndTime = 0.0;
    visitedSetSize = 0;
    indexDirection = NONEINDEX;

    totalConstTime = constEndTime - constStartTime;
    constEndTime = getCurrentTimeInMilliSec();
    this->didComplete = true;
}

BFSIndex::~BFSIndex()
{

}

unsigned long BFSIndex::getIndexSizeInBytes()
{
    return getIndexSizeInBytesM();
};

bool BFSIndex::queryShell(VertexID source, VertexID target, LabelSet ls,set< VertexID >& visitedSet)
{
    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    //cout << "BFSIndex::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;

    queue< VertexID > q;
    q.push(source);

    int N = graph->getNumberOfVertices();
    dynamic_bitset<> marked = dynamic_bitset<>(N);

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();
        //cout << "BFSIndex::query se.first=" << to_string(se.first) << ",se.second=" << labelSetToString(se.second) << endl;

        if( x == target )
            return true;

        if( marked[x] == 1 )
        {
            continue;
        }
        marked[x] = 1;
        visitedSetSize++;

        SmallEdgeSet ses;
        graph->getOutNeighbours(x, ses);
        for(int i = 0; i < ses.size(); i++)
        {
            if( isLabelSubset(ses[i].second, ls) == true )
            {
                //cout << "BFSIndex::query push ses[i].first=" << to_string(ses[i].first) << ",ses[i].second=" << labelSetToString(ses[i].second) << endl;
                q.push( ses[i].first );
            }
        }
    }

    return false;
};

bool BFSIndex::query(VertexID source, VertexID target, LabelSet ls)
{
    set< VertexID > visitedSet;
    visitedSetSize = 0;
    visitedSet.clear();
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls, visitedSet);
    queryEndTime = getCurrentTimeInMilliSec();
    return b;
};

void BFSIndex::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{
    queryStart = getCurrentTimeInMilliSec();

    queue< VertexID > q;
    q.push(source);

    int N = graph->getNumberOfVertices();
    canReach = dynamic_bitset<>(N);

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();

        if( canReach[x] == 1 )
        {
            continue;
        }
        canReach[x] = 1;

        SmallEdgeSet ses;
        graph->getOutNeighbours(x, ses);
        for(int i = 0; i < ses.size(); i++)
        {
            if( isLabelSubset(ses[i].second, ls) == true )
            {
                q.push( ses[i].first );
            }
        }
    }

    queryEndTime = getCurrentTimeInMilliSec();
};

int BFSIndex::getVisitedSetSize()
{
    return visitedSetSize;
}

void BFSIndex::buildIndex()
{
};
