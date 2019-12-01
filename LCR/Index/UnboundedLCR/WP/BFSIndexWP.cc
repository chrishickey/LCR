#include "BFSIndexWP.h"
using namespace indexwpns;

BFSIndexWP::BFSIndexWP(Graph* graph)
{
    cStart = getCurrentTimeInMilliSec();
    this->graph = graph;
    this->indexType = indexwpns::IndexTypeWP::BFS;
    this->bfsIndex = new BFSIndex(graph);

    cout << "BFSIndexWP created." << endl;
    cEnd = getCurrentTimeInMilliSec();
}

BFSIndexWP::~BFSIndexWP()
{

}

bool BFSIndexWP::queryNP(VertexID source, VertexID target, LabelSet ls)
{
    qStart = getCurrentTimeInMilliSec();
    bool b = this->bfsIndex->query(source, target, ls);
    qEnd = getCurrentTimeInMilliSec();
    return b;
};

void BFSIndexWP::queryWP(VertexID source, VertexID target, LabelSet ls, indexwpns::Paths& paths)
{
    // disregard the maximum path length
    queryWP(source, target, ls, paths, -1);
}

void BFSIndexWP::queryWP(VertexID source, VertexID target, LabelSet ls, indexwpns::Paths& paths
    , int maxPathLength )
{
    qStart = getCurrentTimeInMilliSec();
    paths.clear();
    int N = graph->getNumberOfVertices();
    int noOfPaths = 1; // can be tweaked to get more paths

    priority_queue< HeapPathEntry, vector<HeapPathEntry>, HeapPathOrdering > pq;

    Path ps = Path();
    HeapPathEntry hpe;
    hpe.to = source;
    hpe.dist = 0;
    hpe.lps = make_pair( 0 , ps );
    addVertexToPath(source, ps);
    pq.push( hpe );

    //vector<HeapPathEntry> hps = vector<HeapPathEntry>();
    int roundNo = 0;

    while( pq.empty() == false && noOfPaths > 0 )
    {
        HeapPathEntry hpe1 = pq.top();
        pq.pop();

        roundNo++;
        if( roundNo % 100 == 0 )
            cout << "hpe1.to=" << hpe1.to << " ,c=" << hpe1.lps.second.size() << endl;

        if( hpe1.to == target )
        {
            noOfPaths--;
            addVertexToPath(hpe1.to, hpe1.lps.second);
            paths.push_back( hpe1.lps.second );
            continue;
        }

        if( maxPathLength > 0 )
        {
            if( hpe1.lps.second.size() >= maxPathLength )
            {
                continue;
            }
        }

        // a "simple path" is guaranteed by the push condition
        SmallEdgeSet ses;
        this->graph->getOutNeighbours(hpe1.to, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( isLabelSubset( ses[j].second, ls ) ) // checks whether we can traverse label with ls
            {
                if( vertexInPath(ses[j].first, hpe1.lps.second) ) // checks whether path is simple
                {
                    continue;
                }

                HeapPathEntry hpe2;
                hpe2.to = ses[j].first;

                LabelSet ls2 = joinLabelSets( ses[j].second, hpe1.lps.first );
                Path ps2 = hpe1.lps.second;
                addVertexToPath(ses[j].first, ps2);

                //cout << "hpe2.to=" << hpe2.to << " ,ps2=" << ps2.count() << endl;

                hpe2.lps = make_pair( ls2, ps2 );
                hpe2.dist = getNumberOfLabelsInLabelSet(ls2);

                pq.push( hpe2 );
            }
        }
    }

    qEnd = getCurrentTimeInMilliSec();
};
