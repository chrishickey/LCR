#include "BoundedBFS.h"

BoundedBFS::BoundedBFS(Graph* g)
{
    cStart = getCurrentTimeInMilliSec();
    this->indexType = BoundedIndexType::BFS;

    this->graph = g;
    this->N = g->getNumberOfVertices();
    this->M = g->getNumberOfEdges();
    this->L = g->getNumberOfLabels();

    this->bMaxLimit = N;
    cEnd = getCurrentTimeInMilliSec();

    cout << "BoundedBFSIndex N=" << N << endl;
};

BoundedBFS::~BoundedBFS()
{

};

bool BoundedBFS::queryShell(B2Query& q)
{
    queue< BEntry > qu;
    BEntry be;
    be.x = q.from;
    be.b = q.b2;
    qu.push( be );

    dynamic_bitset<> marked = dynamic_bitset<>(N); // path should be simple
    roundNo = 0;

    while( qu.empty() == false )
    {
        BEntry bx = qu.front();
        qu.pop();
        roundNo++;

        if( bx.x == q.to )
        {
            return true;
        }

        if( marked[bx.x] == 1 )
            continue;
        marked[bx.x] = 1;

        SmallEdgeSet ses;
        this->graph->getOutNeighbours(bx.x, ses);
        for(int i = 0; i < ses.size(); i++)
        {
            BEntry by;
            by.x = ses[i].first;

            LabelID lID = labelSetToLabelID(ses[i].second);
            by.b[lID] = bx.b[lID] - 1;

            // push condition
            if( by.b[lID] >= 0 )
            {
                qu.push( by );
            }
        }
    }

    return false;
};

bool BoundedBFS::queryShell(B12Query& q)
{
    queue< BEntry > qu;
    BEntry be;
    be.x = q.from;
    be.b = q.b2;
    qu.push( be );

    dynamic_bitset<> marked = dynamic_bitset<>(N); // path should be simple
    roundNo = 0;

    while( qu.empty() == false )
    {
        BEntry bx = qu.front();
        qu.pop();
        roundNo++;

        if( bx.x == q.to )
        {
            bool b = true;
            for(int i = 0; i < L; i++)
            {
                b &= ( bx.b[i] <= (q.b2[i]-q.b1[i]) ); // label i has been used the minimum number of times
                // label i has not exceeded its maximum which is ensured by push condition
                if( !b )
                    break;
            }

            if( b )
                return true;
        }

        if( marked[bx.x] == 1 )
            continue;
        marked[bx.x] = 1;

        SmallEdgeSet ses;
        this->graph->getOutNeighbours(bx.x, ses);
        for(int i = 0; i < ses.size(); i++)
        {
            BEntry by;
            by.x = ses[i].first;
            LabelID lID = labelSetToLabelID(ses[i].second);

            //cout << "by.x=" << by.x << ",lID=" << lID << endl;

            by.b = bx.b;
            by.b[lID] = bx.b[lID] - 1;

            // push condition: label lID has not exceeded its maximum
            if( by.b[lID] >= 0 )
            {
                qu.push( by );
            }
        }
    }

    return false;
};

long BoundedBFS::getLastRoundNo()
{
    return roundNo;
}
