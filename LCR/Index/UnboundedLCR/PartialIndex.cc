#include "PartialIndex.h"
#include "BFSIndex.h"

#include <queue>
#include <iostream>

using namespace indexns;

void PartialIndex::construct(Graph* mg, vector< VertexID > pordering, bool isBlockedMode, int budget)
{
    this->graph = mg;
    this->indexType = IndexType::Partial;
    int N = graph->getNumberOfVertices();

    if( pordering.size() != graph->getNumberOfVertices())
    {
        createOrdering();
    }
    else
    {
        this->ordering = pordering;
    }

    this->indexDirection = OUTINDEX;
    this->isBlockedMode = isBlockedMode;
    if( budget == -1 )
    {
        determineBudget();
    }
    else
    {
        this->budget = budget;
    }

    this->frequency = (int) max(sqrt(N)/5,1.0);
    this->subsetSize = (int) sqrt(N)*5;
    buildIndex();

    cout << "PartialIndex construction time=" << getIndexConstructionTimeInSec() << endl;
    cout << "PartialIndex size=" << getIndexSizeInBytes() << ",isBlockedMode=" << this->isBlockedMode << ", frequency=" << frequency << " ,budget=" << this->budget << " ,subsetSize=" << this->subsetSize << endl;
};

PartialIndex::PartialIndex(Graph* mg, vector< VertexID > pordering, bool isBlockedMode, int budget)
{
    construct(mg, pordering, isBlockedMode, budget);
}

PartialIndex::PartialIndex(Graph* mg, vector< VertexID > pordering, bool isBlockedMode)
{
    construct(mg, pordering, isBlockedMode, -1 );
}

PartialIndex::PartialIndex(Graph* mg, bool isBlockedMode)
{
    construct(mg, vector<VertexID>(), isBlockedMode, -1 );
}

PartialIndex::PartialIndex(Graph* mg, vector< VertexID > pordering)
{
    construct(mg, pordering, false, -1 );
}

PartialIndex::PartialIndex(Graph* mg)
{
    construct(mg, vector<VertexID>(), false, -1 );
}

PartialIndex::~PartialIndex()
{
    deconstruct();
}

unsigned long PartialIndex::getIndexSizeInBytes()
{
    return getIndexSizeInBytesM();
};

void PartialIndex::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};

void PartialIndex::buildIndex()
{
    constStartTime = getCurrentTimeInMilliSec();

    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();
    long quotum = max(subsetSize/5, (long) 2);

    long size = 0; // estimated size of the index while being under construction

    initializeIndex(subsetSize);

    // noOfLandmarks known, pick the noOfLandmarks nodes with the highest
    // out-degree
    vector< pair< VertexID, int > > outDegreePerNode;
    vToSubsetID = vector< VertexID >(N, -1);
    subset = vector< VertexID >();

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses;
        graph->getOutNeighbours(i, ses);
        outDegreePerNode.push_back( make_pair(i , ses.size() ));
    }

    struct sort_pred {
        bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
            return left.second > right.second;
        }
    };

    sort(outDegreePerNode.begin(), outDegreePerNode.end(), sort_pred() );

    for(int i = 0; i < subsetSize; i++)
    {
        subset.push_back( outDegreePerNode[i].first );
        vToSubsetID[ outDegreePerNode[i].first ] = i;
    }

    // Start a labeled BFS for each node in subset
    //hasBeenIndexed = dynamic_bitset<>(N);
    for(int i = 0; i < subsetSize; i++)
    {
        if( getCurrentTimeInMilliSec()-constStartTime >= TIMEOUT )
        {
            cout << "PartialIndex::buildIndex times out" << endl;
            break;
        }

        if( size > MEM_LIMIT )
        {
            // quit, the index size is too large
            cout << "PartialIndex::size above MEM_LIMIT" << endl;
            break;
        }

        int a = buildIndexForNode( subset[i] );
        //hasBeenIndexed[ subset[i] ] = 1;
        if( ((i+1)%quotum) == 0 )
        {
            cout << "PartialIndex::buildIndex i=" << i << " " << (( (double) i+1 )/((double) subsetSize) * 100) << "% , subset[i]=" << subset[i] << ",roundNo=" << a << ",size=" << size << endl;
        }
        size += getIndexSizeInBytesM( subset[i] );

        // we can back propagate to ordering[i]'s in-neighbours
        SmallEdgeSet ses;
        graph->getInNeighbours(subset[i], ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( vToSubsetID[ses[j].first] == -1 )
            {
                continue;
            }

            backPropagate(subset[i], ses[j].first, ses[j].second);
        }
    }

    constEndTime = getCurrentTimeInMilliSec();
};

void PartialIndex::determineBudget()
{
    // the budget may differ w.r.t. the graph properties, dense graphs can deal
    // with a much higher budget than sparse graphs
    // hence we take a subsample to determine the budget

    int N = graph->getNumberOfVertices();
    int budget = 0;

    unsigned int seed = time(NULL)*time(NULL) % 1000;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

    int total = 0;
    int maxPart = (int) ceil(N/2);

    for(int i = 0; i < N/10; i++)
    {
        VertexID s = vertexDistribution(generator);
        queue< VertexID > q;
        dynamic_bitset<> marked = dynamic_bitset<>(N);
        q.push( s );

        int count = 0;

        while( q.empty() == false )
        {
            VertexID x = q.front();
            q.pop();

            if( marked[x] == 1 )
            {
                continue;
            }
            marked[x] = 1;
            count++;

            SmallEdgeSet ses;
            graph->getOutNeighbours(x, ses);
            for(int j = 0; j < ses.size(); j++)
            {
                q.push( ses[j].first );
            }
        }

        total += min(count, maxPart);
    }

    budget = min(N, total / (N/10) + 1);
    cout << "determineBudget: total=" << total << ", maxPart=" << maxPart << " ,budget=" << budget << endl;

    this->budget = budget;
}

void PartialIndex::backPropagate(VertexID v, VertexID w, LabelSet ls)
{
    //cout << "backPropagate: v=" << v << ",w=" << w << ",ls=" << labelSetToString(ls) << endl;

    // assume we have edge (w,v,ls), i.e. v is an in-neighbour of w
    int N = graph->getNumberOfVertices();
    v = vToSubsetID[v];

    if( isBlockedMode == true )
    {
        for(int i = 0; i < N; i++)
        {
            if( i == w )
            {
                continue;
            }

            for(int j = 0; j < cIn[v][i].size(); j++)
            {
                LabelSet ls1 = joinLabelSets(ls, cIn[v][i][j]);
                tryInsert(w, i, ls1);
            }

        }
    }
    else
    {
        for(int i = 0; i < tIn[v].size(); i++)
        {
            Tuple tu = tIn[v][i];
            if( tu.first == w )
            {
                continue;
            }

            for(int j=0; j < tu.second.size(); j++)
            {
                LabelSet ls1 = joinLabelSets(ls, tu.second[j]);
                tryInsert(w, tu.first, ls1);
            }
        }
    }

};

int PartialIndex::buildIndexForNode(VertexID w)
{
    // w may budget up until budget nodes

    // start with a queue containing pair (w,0)
    int N = graph->getNumberOfVertices();

    queue< pair< VertexID, LabelSet > > q;
    q.push( make_pair(w, 0) );
    dynamic_bitset<> visited = dynamic_bitset<>(N);

    int roundNo = 0;
    int count = 0;

    while( q.empty() == false )
    {
        roundNo++;
        VertexID v1 = q.front().first;
        LabelSet ls1 = q.front().second;
        q.pop();

        if( visited[v1] == 0 && count >= budget )
        {
            continue;
        }

        //cout << "buildIndexForNode pulling from queue v1=" << v1 << ",ls1=" << labelSetToString(ls1) << endl;

        // we try to insert the entry (v1,ls1) to cIn[w]
        // b is false if and only if inserting (v1,ls1) would create
        // a conflict in cIn[w], that is adding a superset of an existing entry
        if( w != v1 )
        {
            if( visited[ v1 ] == 0 )
            {
                count++;
                visited[v1] = 1;
            }

            bool b = tryInsert(w, v1, ls1);
            if( b == false )
            {
                continue;
            }
        }

        /*if( hasBeenIndexed[v1] == 1  )
        {
            // v1 has been indexed
            // we can copy all entries from v1 and do not need to get further here
            LabelSets lss;

            for(int i = 0; i < N; i++)
            {
                if( i == v1 || i == w )
                    continue;

                getLabelSetsPerPair( vToSubsetID[v1],i,lss);

                for(int j = 0; j < lss.size(); j++)
                {
                    tryInsert(w, i, joinLabelSets(lss[j],ls1) );
                }
            }

            continue;
        }*/

        // we push the neighbours of v1 to the stack
        // and union the labels
        SmallEdgeSet ses;
        graph->getOutNeighbours(v1, ses);

        for(int i = 0; i < ses.size(); i++)
        {
            VertexID v2 = ses[i].first;
            LabelSet ls2 = ses[i].second;
            LabelSet ls3 = joinLabelSets(ls1, ls2);

            if( v2 == w )
            {
                continue;
            }

            //cout << "buildIndexForNode pushing to queue v2=" << v2 << ",ls3=" << labelSetToString(ls3) << endl;
            q.push( make_pair(v2 , ls3) );
        }
    }

    return roundNo;
};

bool PartialIndex::tryInsert(VertexID w, VertexID v, LabelSet ls)
{
    if( vToSubsetID[w] == -1 )
    {
        return false;
    }

    if( w == v )
    {
        return false;
    }

    bool b2 = tryInsertLabelSetToIndex(ls, vToSubsetID[w], v);
    //cout << "tryInsert: w=" << w << ",v=" << v << ",ls=" << labelSetToString(ls) << ",b2=" << b2 << endl;
    return b2;
}

bool PartialIndex::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "PartialIndex::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls);
    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "PartialIndex::query answer =" << b << endl;
    return b;
}

bool PartialIndex::queryShell(VertexID source, VertexID target, LabelSet ls)
{
    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    int N = graph->getNumberOfVertices();

    queue< VertexID > q;
    q.push( source );
    dynamic_bitset<> visited = dynamic_bitset<>(N);

    int roundNo = 0;

    while( q.empty() == false )
    {
        VertexID v1 = q.front();
        q.pop();

        if( visited[v1] == 1 )
        {
            continue;
        }
        visited[v1] = 1;

        if( v1 == target )
        {
            return true;
        }

        if( vToSubsetID[v1] != -1 )
        {
            if( roundNo%frequency == 0 )
            {
                if( queryDirect(v1, target, ls) )
                {
                    return true;
                }
            }
            roundNo++;
        }
        // explore the out-edges
        SmallEdgeSet ses;
        graph->getOutNeighbours(v1, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( isLabelSubset(ses[j].second, ls) == true )
            {
                q.push(ses[j].first);
            }
        }
    }

    return false;
};

bool PartialIndex::queryDirect(VertexID source, VertexID target, LabelSet ls)
{
    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    if( vToSubsetID[source] == -1 )
    {
        return false;
    }

    if( isBlockedMode == true )
    {
        for(int i = 0; i < cIn[ vToSubsetID[source] ][target].size(); i++)
        {
            LabelSet ls2 = cIn[ vToSubsetID[source] ][target][i];

            if( isLabelSubset(ls2,ls) == true )
                return true;
        }
    }
    else
    {
        int pos = 0;
        bool b = findTupleInTuples(target, tIn[ vToSubsetID[source] ], pos);
        if( b == true )
        {
            for(int i = 0; i < tIn[vToSubsetID[source]][pos].second.size(); i++)
            {
                LabelSet ls2 = tIn[vToSubsetID[source]][pos].second[i];

                if( isLabelSubset(ls2,ls) == true )
                    return true;
            }
        }
    }

    return false;
};

void PartialIndex::printIndexStats()
{
    // tmp function to get stats about index
    int N = graph->getNumberOfVertices();
    for(int i = 0; i < N; i++)
    {
        cout << "vertex-" << i << " , ";

        for(int j = 0; j < N; j++)
        {
            cout << cIn[i][j].size();

            if( j < N - 1 )
            {
                cout  << " , ";
            }
        }

        cout << endl;
    }
};

void PartialIndex::createOrdering()
{
    ordering = vector<VertexID>();
    int N = graph->getNumberOfVertices();

    dynamic_bitset<> marked = dynamic_bitset<>(N);

    vector< pair<VertexID, int> > inDegreePerNode = vector< pair<VertexID, int> >();
    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses;
        graph->getOutNeighbours(i,ses);

        inDegreePerNode.push_back( make_pair(i,ses.size()) );
    }

    sort(inDegreePerNode.begin(), inDegreePerNode.end(), sort_pred() );
    for(int i = 0; i < N; i++)
    {
        VertexID v = inDegreePerNode[i].first;
        if( marked[v] == 1 )
        {
            continue;
        }

        queue< VertexID > q;
        q.push(v);

        while( q.empty() == false )
        {
            VertexID x = q.front();
            q.pop();

            if(marked[x] == 1)
            {
                continue;
            }

            marked[x] = 1;
            ordering.push_back( x );

            SmallEdgeSet ses;
            graph->getInNeighbours(x, ses);
            for(int j = 0; j < ses.size(); j++)
            {
                if( marked[ses[j].first] == 1 )
                {
                    continue;
                }

                q.push( ses[j].first );
            }
        }
    }
}
