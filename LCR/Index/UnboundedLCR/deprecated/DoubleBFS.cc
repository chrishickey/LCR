#include "DoubleBFS.h"
#include "BFSIndex.h"

#include <queue>
#include <iostream>

using namespace indexns;
using namespace dbfsns;

/**
* To make the code compilable >= g++-4.6.2, we had to do something like this.
* We removed all constructors of the form A(int x) : A( x, 0 ), where A(int x, int y)
* is the root-constructor.
*/
void DoubleBFS::construct(Graph* mg, vector< VertexID > pordering, bool isBlockedMode, bool useHeap)
{
    this->graph = mg;
    this->indexType = IndexType::DoubleBFSenum;

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
    this->useHeap = useHeap;
    buildIndex(false);
};

DoubleBFS::DoubleBFS(Graph* mg, vector< VertexID > pordering, bool isBlockedMode, bool useHeap)
{
    construct(mg, pordering, isBlockedMode, useHeap);
}

DoubleBFS::DoubleBFS(Graph* mg, bool isBlockedMode)
{
    construct(mg, vector<VertexID>(), false, true );
}

DoubleBFS::DoubleBFS(Graph* mg, vector< VertexID > pordering)
{
    construct(mg, pordering, false, true );
}

DoubleBFS::DoubleBFS(Graph* mg)
{
    construct(mg, vector<VertexID>(), false, true);
}

DoubleBFS::~DoubleBFS()
{
    deconstruct();
}

unsigned long DoubleBFS::getIndexSizeInBytes()
{
    unsigned long size = getIndexSizeInBytesM();
    return size;
}

void DoubleBFS::buildIndex(bool doRebuild)
{
    cout << "DoubleBFS::buildIndex start" << endl;
    constStartTime = getCurrentTimeInMilliSec();

    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();
    int quotum = max(N/5, 2);

    this->Nt = 1; // multi-threading: not working
    if( this->Nt > 1 )
    {
        isActive = vector< bool >(Nt-1, false); // discounting main thread
        size_t count = N;
        this->mutexes = std::vector<std::mutex>(count);
    }

    if( doRebuild == true )
    {
        createOrdering();
    }
    else
    {
        hasBeenIndexed = dynamic_bitset<>(N);
    }

    initializeIndex();

    long size = 0; // estimated size of the index while being under construction

    // Start a labeled BFS for each node
    int i = 0;
    while(i < N)
    {
        if( getCurrentTimeInMilliSec()-constStartTime >= TIMEOUT )
        {
            cout << "DoubleBFS::buildIndex times out" << endl;
            break;
        }

        if( size > MEM_LIMIT )
        {
            // quit, the index size is too large
            cout << "DoubleBFS::size above MEM_LIMIT" << endl;
            break;
        }

        if( (i%quotum) == 0 )
        {
            cout << "DoubleBFS::buildIndex i=" << i << " " << (( (double) i )/((double) N) * 100) << "% , ordering[i]=" << ordering[i] << endl;
        }

        int a = 0;
        if( Nt > 1 )
        {
            bool allThreadsActive = true;
            int tID = 0;
            while( allThreadsActive == true )
            {
                for(int j = 0; j < Nt-1; j++)
                {
                    isActiveMutex.lock();
                    if( isActive[j] == false )
                    {
                        // a thread has completed or has not been started yet
                        tID = j;
                        allThreadsActive = false;
                        j = Nt-1;
                    }
                    isActiveMutex.unlock();
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            VertexID w = ordering[i];
            //std::thread t1 (&DoubleBFS::buildIndexForNodeMultiThread, this, w, tID);//( [this, w, tID] { this->buildIndexForNodeMultiThread(w, tID); } );
            //t1.detach();//(&DoubleBFS::buildIndexForNodeMultiThread, *this, w, tID); // //(buildIndexForNodeMultiThread, w, tID );
            //boost::thread* thr = new boost::thread(boost::bind( &DoubleBFS::buildIndexForNodeMultiThread, this, w, tID) );
            hasBeenIndexed[ w ] = 1;
            size += getIndexSizeInBytesM( w );
        }
        else
        {
            if( this->useHeap == true )
                a = buildIndexForNodeB( ordering[i] );
            else
                a = buildIndexForNode( ordering[i] );
            hasBeenIndexed[ ordering[i] ] = 1;
            size += getIndexSizeInBytesM( ordering[i] );
        }

        if( (i%quotum) == 0 )
        {
            cout << "DoubleBFS::buildIndex i=" << i << " " << (( (double) i )/((double) N) * 100) << "% , ordering[i]="
                << ordering[i] << " ,a=" << a << " ,size=" << size << endl;
        }

        i++;
    }
    indexShrinkToFit();

    constEndTime = getCurrentTimeInMilliSec();
    cout << "DoubleBFS::buildIndex complete" << endl;
    cout << "DoubleBFS construction time(s)=" << getIndexConstructionTimeInSec() << endl;
    cout << "DoubleBFS size=" << getIndexSizeInBytes() << ",isBlockedMode=" << this->isBlockedMode << endl;
};

int DoubleBFS::buildIndexForNodeMultiThread(VertexID w, int threadID)
{
    isActiveMutex.lock();
    isActive[threadID] = true;
    isActiveMutex.unlock();

    int a;

    if( this->useHeap == true )
        a = buildIndexForNodeB( w );
    else
        a = buildIndexForNode( w );

    isActiveMutex.lock();
    isActive[threadID] = false;
    isActiveMutex.unlock();

    return a;
};

void DoubleBFS::backPropagate(VertexID v, VertexID w, LabelSet ls)
{
    //cout << "backPropagate: v=" << v << ",w=" << w << ",ls=" << labelSetToString(ls) << endl;

    // assume we have edge (w,v,ls), i.e. v is an in-neighbour of w
    int N = graph->getNumberOfVertices();
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

int DoubleBFS::buildIndexForNodeB(VertexID w)
{
    /* buildIndexForNodeB does the same as buildIndexForNode but uses a heap
    with the Triplet-struct s.t. whenever w hits a node v with a path with
    labelset LS it has not been visited before with a subset of LS.

    Over larger datasets it seems to work better than buildIndexForNode. Besides
    this there is no difference.
    */

    // start with a queue containing pair (w,0)
    priority_queue< Triplet, vector<Triplet>, priorityQueueTriplets > q;

    Triplet t;
    t.x = w;
    t.ls = 0;
    t.dist = 0;

    q.push( t );

    int roundNo = 0;
    int N = graph->getNumberOfVertices();

    while( q.empty() == false )
    {
        roundNo++;
        Triplet tr = q.top();
        VertexID v1 = tr.x;
        VertexID ls1 = tr.ls;
        q.pop();

        // we try to insert the entry (v1,ls1) to cIn[w]
        // b is false if and only if inserting (v1,ls1) would create
        // a conflict in cIn[w], that is adding a superset of an existing entry
        if( w != v1 )
        {
            bool b = tryInsert(w, v1, ls1);
            if( b == false )
            {
                //cout << "aaa: useHeap=" << useHeap << "" << endl;
                continue;
            }
        }

        // forward propagation case. Vertex v1 has been index.
        // We can copy all entries from v1 and do not need to get further here
        if( hasBeenIndexed[v1] == 1  )
        {
            LabelSets lss;

            for(int i = 0; i < N; i++)
            {
                if( i == v1 || i == w )
                    continue;

                getLabelSetsPerPair(v1,i,lss);

                for(int j = 0; j < lss.size(); j++)
                {
                    tryInsert(w, i, joinLabelSets(lss[j],ls1) );
                }
            }

            continue;
        }

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

            int dist = tr.dist;
            if( ls3 != ls1 || ls3 != ls2 )
            {
                dist += 1; // labels are added one by one
            }

            Triplet tr2;
            tr2.x = v2;
            tr2.ls = ls3;
            tr2.dist = dist;

            q.push( tr2 );
        }
    }

    return roundNo;
}

int DoubleBFS::buildIndexForNode(VertexID w)
{
    // start with a queue containing pair (w,0)
    queue< pair< VertexID, LabelSet > > q;
    q.push( make_pair(w, 0) );

    int roundNo = 0;
    int N = graph->getNumberOfVertices();

    while( q.empty() == false )
    {
        roundNo++;
        VertexID v1 = q.front().first;
        LabelSet ls1 = q.front().second;
        q.pop();

        // we try to insert the entry (v1,ls1) to cIn[w]
        // b is false if and only if inserting (v1,ls1) would create
        // a conflict in cIn[w], that is adding a superset of an existing entry
        if( w != v1 )
        {
            bool b = tryInsert(w, v1, ls1);
            if( b == false )
            {
                //cout << "bbb: useHeap=" << useHeap << "" << endl;
                continue;
            }
        }

        if( hasBeenIndexed[v1] == 1  )
        {
            // v1 has been indexed
            // we can copy all entries from v1 and do not need to get further here
            LabelSets lss;

            for(int i = 0; i < N; i++)
            {
                if( i == v1 || i == w )
                    continue;

                getLabelSetsPerPair(v1,i,lss);

                for(int j = 0; j < lss.size(); j++)
                {
                    tryInsert(w, i, joinLabelSets(lss[j],ls1) );
                }
            }

            continue;
        }

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

bool DoubleBFS::tryInsert(VertexID w, VertexID v, LabelSet ls)
{
    if( w == v )
    {
        return false;
    }

    std::mutex& m = mutexes[w];
    m.lock();

    bool b2 = tryInsertLabelSetToIndex(ls, w, v);

    m.unlock();
    //cout << "tryInsert: w=" << w << ",v=" << v << ",ls=" << labelSetToString(ls) << ",b2=" << b2 << endl;
    return b2;
}

bool DoubleBFS::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "DoubleBFS::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls);
    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "DoubleBFS::query answer =" << b << endl;
    return b;
}

void DoubleBFS::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};

bool DoubleBFS::queryShell(VertexID source, VertexID target, LabelSet ls)
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
        int pos = 0;
        bool b = findTupleInTuples(target, tIn[source], pos);
        if( b == true )
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

void DoubleBFS::printIndexStats()
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

void DoubleBFS::createOrdering()
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

    cout << "DoubleBFS::createOrdering ordering generated." << endl;
}

void DoubleBFS::addNode()
{
    this->graph->addNode();
};

void DoubleBFS::removeNode(VertexID v)
{
    this->graph->removeNode(v);
    this->buildIndex(true);
};


void DoubleBFS::addEdge(VertexID v, VertexID w, LabelID lID)
{
    // An edge (v,w,lID) has been added
    priority_queue< Triplet, vector<Triplet>, priorityQueueTriplets > q;

    Triplet t;
    t.x = v;
    t.ls = labelIDToLabelSet(lID);
    t.dist = 1;

    q.push( t );

    int roundNo = 0;
    int N = graph->getNumberOfVertices();

    while( q.empty() == false )
    {
        roundNo++;
        Triplet tr = q.top();
        VertexID v1 = tr.x;
        LabelSet ls1 = tr.ls;
        q.pop();

        if( w != v1 )
        {
            LabelSets lss;
            bool b = false;

            for(int i = 0; i < N; i++)
            {
                if( i == v1 )
                    continue;

                getLabelSetsPerPair(v1,i,lss);

                for(int j = 0; j < lss.size(); j++ )
                {
                    LabelSet ls2 = joinLabelSets(lss[j], ls1);
                    b |= tryInsert(v1, w, ls2);
                }
            }

            if( b == false )
            {
                continue;
            }
        }

        SmallEdgeSet ses;
        graph->getInNeighbours(v1, ses);
        for(int i = 0; i < ses.size(); i++)
        {
            Triplet t2;
            t2.x = ses[i].first;
            t2.ls = joinLabelSets(ls1, ses[i].second);
            t2.dist = getNumberOfLabelsInLabelSet(t2.ls);

            q.push(t2);
        }
    }

    this->graph->addEdge(v,w,lID);
};

void DoubleBFS::removeEdge(VertexID v, VertexID w)
{
    // TO DO: works, but maybe a better approach

    double A = getCurrentTimeInMilliSec();

    // An edge v,w,lID has been removed
    LabelID lID = graph->getLabelID(v,w);
    LabelSet lsR = labelIDToLabelSet(lID);
    int N = graph->getNumberOfVertices();

    // actually remove the edge
    this->graph->removeEdge(v,w);

    // find all vertices w' that w can reach (spread)
    dynamic_bitset<> spread = dynamic_bitset<>(N);
    queue< VertexID > q;
    q.push(w);

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();

        if( spread[x] == 1 )
        {
            continue;
        }
        spread[x] = 1;

        SmallEdgeSet ses;
        graph->getAllNeighbours(x, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            q.push(ses[j].first);
        }
    }

    hasBeenIndexed = dynamic_bitset<>(N);
    for(int i = 0;  i < N; i++)
    {
        if( spread[i] == 0 )
            continue;

        if( isBlockedMode == true )
        {
            cIn[i].clear();
        }
        else
        {
            tIn[i].clear();
        }

        if( this->useHeap == true )
        {
            this->buildIndexForNodeB(i);
        }
        else
        {
            this->buildIndexForNode(i);
        }

        hasBeenIndexed[i] = 1;
    }

    /*// delete for any v all entries (v,v') s.t. v' is in the spread and the labelset
    // contains lsR and rebuild. This should be less work.
    // rebuild index for infected nodes, i.e. nodes with an out-edge having label lsR
    hasBeenIndexed = dynamic_bitset<>(N);

    for(int i = 0; i < N; i++)
    {
        // determine whether out-edge with lsR exists
        SmallEdgeSet ses;
        this->graph->getOutNeighbours(i, ses);
        bool lsRExists = false;

        for(int j = 0; j < ses.size(); j++)
        {
            if( isLabelSubset(lsR, ses[j].first) == true )
            {
                lsRExists = true;
                break;
            }
        }

        // retain all entries with no lsR in it
        if( lsRExists == true )
        {
            LabelSets lss, lss2;
            for(int j = 0; j < N; j++)
            {
                if( i == j || spread[j] == 0 )
                    continue;

                getLabelSetsPerPair(i,j,lss);
                lss2.clear();
                for(int k = 0; k < lss.size(); k++)
                {
                    if( isLabelSubset(lsR , lss[k]) == false )
                    {
                        lss2.push_back( lss[k] );
                    }
                }

                setEntry(i, j, lss2);
            }
        }
        else
        {
            if( isBlockedMode == true )
            {
                cIn[i].clear();
            }
            else
            {
                tIn[i].clear();
            }
        }

        if( this->useHeap == true )
        {
            this->buildIndexForNodeB(i);
        }
        else
        {
            this->buildIndexForNode(i);
        }

        hasBeenIndexed[i] = 1;
    }*/



    double B = getCurrentTimeInMilliSec();
    cout << "DoubleBFS::removeEdge time(s)=" << (B-A) << endl;
    cout << "DoubleBFS::removeEdge size(byte)=" << getIndexSizeInBytes() << endl;
};

void DoubleBFS::changeLabel(VertexID v, VertexID w, LabelID lID)
{
    double A = getCurrentTimeInMilliSec();

    // An edge v,w,lID has been removed
    LabelID lIDOld = graph->getLabelID(v,w);
    LabelSet lsR = labelIDToLabelSet(lID);
    int N = graph->getNumberOfVertices();

    // actually change the label
    this->graph->changeLabel(v,w,lID);

    dynamic_bitset<> spread = dynamic_bitset<>(N);
    queue< VertexID > q;
    q.push(w);

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();

        if( spread[x] == 1 )
        {
            continue;
        }
        spread[x] = 1;

        SmallEdgeSet ses;
        graph->getAllNeighbours(x, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            q.push(ses[j].first);
        }
    }

    hasBeenIndexed = dynamic_bitset<>(N);
    for(int i = 0;  i < N; i++)
    {
        if( spread[i] == 0 )
            continue;

        if( isBlockedMode == true )
        {
            cIn[i].clear();
        }
        else
        {
            tIn[i].clear();
        }

        if( this->useHeap == true )
        {
            this->buildIndexForNodeB(i);
        }
        else
        {
            this->buildIndexForNode(i);
        }

        hasBeenIndexed[i] = 1;
    }
};
