#include "NeighbourExchange.h"

#include <iostream>
#include <queue>
#include <set>

using namespace std;
using namespace neighbourexchangens;
using namespace indexns;

void NeighbourExchange::construct(Graph* mg, vector<VertexID> pordering, int pNt)
{
    this->graph = mg;
    this->Nt = pNt;
    this->indexType = IndexType::Exact;

    this->queryStart = 0.0;
    this->queryEndTime = 0.0;
    this->visitedSetSize = 0;
    this->indexDirection = ININDEX;
    this->isBlockedMode = true; // currently only supports blocked mode

    if( pordering.size() == 0 )
    {
        createOrdering();
    }
    else
    {
        this->ordering = pordering;
    }

    buildIndex();
    cout << "NeighbourExchange construction time=" << getIndexConstructionTimeInSec() << endl;
    cout << "NeighbourExchange size=" << getIndexSizeInBytes() << ",isBlockedMode=" << isBlockedMode << endl;
};

NeighbourExchange::NeighbourExchange(Graph* mg, vector<VertexID> pordering, int pNt)
{
    construct(mg, pordering, pNt);
};

NeighbourExchange::NeighbourExchange(Graph* mg)
{
     construct(mg, vector<VertexID>(), 1 );
};

NeighbourExchange::~NeighbourExchange()
{
    deconstruct();
}

void NeighbourExchange::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};

void NeighbourExchange::createOrdering()
{
    int N = graph->getNumberOfVertices();

    for(int i = 0; i < N; i++)
    {
        ordering.push_back(i);
    }

}

void NeighbourExchange::buildIndex()
{
    // currently we assume a single-threaded scenario
    Nt = 1;
    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();
    VertexID Kt = N / Nt;

    cUp = EIndex();
    cIn = EIndex();

    // initialize the empty vectors
    for(int i = 0; i < N; i++)
    {
        LabelSetsPerNode v1 = LabelSetsPerNode();
        LabelSetsPerNode v2 = LabelSetsPerNode();

        for(int j = 0; j < N; j++)
        {
            LabelSets Lss1 = LabelSets();
            LabelSets Lss2 = LabelSets();
            v1.push_back(Lss1);
            v2.push_back(Lss2);
        }

        cUp.push_back(v1);
        cIn.push_back(v2);
    }

    constStartTime = getCurrentTimeInMilliSec();
    //cout << "buildIndex: N=" << N << ",Nt=" << Nt << ",Kt=" << Kt << endl;

    for(int i = 0; i < Nt; i++)
    {
        if( getCurrentTimeInMilliSec()-constStartTime >= TIMEOUT )
        {
            cout << "PartialIndex::buildIndex times out" << endl;
            break;
        }

        vector<VertexID> vertices;
        vertices.reserve(Kt);
        for(int j = (i*Kt); j < (i+1)*Kt; j++)
        {
            vertices.push_back( ordering[j] );
        }
        buildIndexPart(vertices);
    }

    constEndTime = getCurrentTimeInMilliSec();
}

void NeighbourExchange::buildIndexPart(vector<VertexID>& vertices)
{
    if( vertices.size() == 0 )
        return;

    int roundNo = 1;
    bool hasChanges = true;
    long size = 0; // estimated size of the index while being under construction

    while( hasChanges )
    {
        cout << "NeighbourExchange::buildIndexPart roundNo=" << roundNo << endl;
        hasChanges = false;
        int i = 0;

        if( getCurrentTimeInMilliSec()-constStartTime >= TIMEOUT )
        {
            hasChanges = false;
            cout << "NeighbourExchange::buildIndex times out" << endl;
            break;
        }

        size = getIndexSizeInBytes();
        if( size > MEM_LIMIT )
        {
            hasChanges = false;
            cout << "NeighbourExchange::buildIndex size above MEM_LIMIT" << endl;
            break;
        }

        while( i < vertices.size() )
        {
            VertexID v = vertices[i];

            if( roundNo == 1 )
            {
                // During first round we add an artificial update (v,0) to
                // start the flow. The initial update is then sent to the
                // neighbours of v.
                cUp[v][v].push_back(0);
            }

            hasChanges |= checkAndSendUpdates(v);

            i++;
            //cout << endl;
        }

        roundNo++;
    }

}

bool NeighbourExchange::checkAndSendUpdates(VertexID v)
{
    bool hasChanges = false;

    int N = graph->getNumberOfVertices();
    LabelSetsPerNode up = cUp[v];
    LabelSetsPerNode in = cIn[v];

    SmallEdgeSet ses;
    graph->getOutNeighbours(v, ses);

    // loop over all nodes that could have an update
    for(int i = 0; i < N; i++)
    {
        // loop over all the updates
        for(int k = 0; k < up[i].size(); k++)
        {
            LabelSet lss1 = up[i][k];

            // insert the update in <in>
            bool b1 = false;
            if( i == v )
            {
                if( lss1 == 0 )
                {
                    b1 = true;
                }
                else
                {
                    continue;
                }
            }
            else
            {
                //b1 = tryInsertLabelSet(lss1, cIn[v][i]);
                b1 = tryInsertLabelSetToIndex(lss1, i, v);
            }


            // if lss1 could not be inserted into in then we do not need to
            // propagate the update
            if( b1 == false )
            {
                continue;
            }

            // loop over all v's neighbours (w) and send the update by appending
            // the edge label of (v,w) to the label set of the update entry
            for(int j = 0; j < ses.size(); j++)
            {
                VertexID vNeighbour = ses[j].first;
                LabelSet lss2 = ses[j].second;

                LabelSet lss3 = joinLabelSets(lss1, lss2);

                bool b2 = tryInsertLabelSet(lss3, cUp[vNeighbour][i]);

                if( b2 == true )
                {
                    hasChanges = true;
                }
            }
        }

        if( up[i].size() > 0 )
            up[i].clear(); // the changes have been processed and can be deleted
    }

    return hasChanges;
}

bool NeighbourExchange::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "NeighbourExchange::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls);
    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "NeighbourExchange::query answer=" << b << endl;
    return b;
}

unsigned long NeighbourExchange::getIndexSizeInBytes()
{
    return getIndexSizeInBytesM();
};

bool NeighbourExchange::queryShell(VertexID source, VertexID target, LabelSet ls)
{
    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    if( isBlockedMode == true )
    {
        for(int i = 0; i < cIn[source][target].size(); i++)
        {
            LabelSet ls2 = cIn[source][target][i];

            if( isLabelSubset(ls2,ls) == true )
                return true;
        }
    }
    else
    {
        int pos = findTupleInTuples(target, tIn[source]);
        if( tupleExists(target, tIn[source], pos) == true )
        {
            for(int i = 0; i < tIn[source][pos].second.size(); i++)
            {
                LabelSet ls2 = tIn[source][pos].second[i];

                if( isLabelSubset(ls2,ls) == true )
                    return true;
            }
        }
    }

    return false;
};
