#include "LandmarkedIndexWD.h"

using namespace landmarkedwdns;
using namespace indexwdns;

LandmarkedIndexWD::LandmarkedIndexWD(Graph* mg, int noOfLandmarks, int othersBudget)
{
    construct(mg, noOfLandmarks, othersBudget);
};

LandmarkedIndexWD::~LandmarkedIndexWD()
{

};

void LandmarkedIndexWD::construct(Graph* mg, int noOfLandmarks, int othersBudget)
{
    this->graph = mg;
    this->noOfLandmarks = noOfLandmarks;
    this->othersBudget = othersBudget;

    this->name = "LandmarkedIndexWD_k=" + to_string(noOfLandmarks) + "_b=" + to_string(othersBudget);
    this->totalConstTime = 0.0;

    buildIndex();

    cout << this->name << " time(s)=" << getIndexConstructionTimeInSec() << " ,size(byte)="
        << getIndexSizeInBytes() << endl;
};

void LandmarkedIndexWD::buildIndex()
{
    constStartTime = getCurrentTimeInMilliSec();
    int N = graph->getNumberOfVertices();
    int M = graph->getNumberOfEdges();

    determineLandmarks();
    intializeIndex( N );

    long size = 0; // estimated size of the index while being under construction
    hasBeenIndexed = dynamic_bitset<>(N);
    int quotum = max(N/20, 1);

    for(int i = 0; i < N; i++)
    {
        if( getCurrentTimeInMilliSec()-constStartTime >= TIMEOUT )
        {
            cout << this->name << "LandmarkedIndex::buildIndex times out" << endl;
            break;
        }

        if( size > MEM_LIMIT )
        {
            // quit, the index size is too large
            cout << this->name << "::size above MEM_LIMIT" << endl;
            break;
        }

        bool isLandmark = (vToLandmark[i] != -1);
        int a = 0;
        if( isLandmark == true )
        {
            buildIndexForNodePQ(i, true, false);
        }

        hasBeenIndexed[ i ] = 1;
        size += getIndexSizeInBytes( i );

        if( (i%quotum) == 0 && i > 0 )
        {
            cout << this->name << "::buildIndex (o) i=" << i << " " << (( (double) i )/((double) N) * 100) << "% , " << ",roundNo=" << a
                << ",size=" << size << " ,isLandmark=" << isLandmark << " vToLandmark[i]=" << vToLandmark[i] << endl;
        }

    }

    constEndTime = getCurrentTimeInMilliSec();
    this->totalConstTime += (constEndTime - constStartTime);
};

int LandmarkedIndexWD::buildIndexForNodePQ(VertexID w, bool doPropagate, bool isMaintenance)
{
    priority_queue< TripletWD, vector<TripletWD>, priorityQueueTripletsWD > q;

    TripletWD t;
    t.x = w;
    t.ls = 0;
    t.labelDist = 0;
    t.edgeDist = 0;

    q.push( t );

    int roundNo = 0;
    int N = this->graph->getNumberOfVertices();

    while( q.empty() == false )
    {
        roundNo++;

        TripletWD tr = q.top();
        VertexID v1 = tr.x;
        VertexID ls1 = tr.ls;
        int d1 = tr.edgeDist;
        q.pop();

        //cout << "v1=" << v1 << endl;

        // we try to insert the entry (v1,ls1) to cIn[w]
        // b is false if and only if inserting (v1,ls1) would create
        // a conflict in cIn[w], that is adding a superset of an existing entry
        if( w != v1 )
        {
            bool b = tryInsert(w, v1, ls1, tr.edgeDist);
            if( b == false )
            {
                continue;
            }
        }

        if( doPropagate == true )
        {
            if( hasBeenIndexed[v1] == 1 && vToLandmark[v1] != -1 )
            {
                // v1 has been indexed and is a landmark
                // we can copy all entries from v1 and do not need to get further here
                for(int i = 0; i < Ind[v1].size(); i++)
                {
                    VertexID y = Ind[v1][i].x;
                    if( y == w )
                        continue;

                    for(int j = 0; j < Ind[v1][i].lsds.size(); j++)
                    {
                        LabelSet ls2 = Ind[v1][i].lsds[j].first;
                        unsigned int d2 = Ind[v1][i].lsds[j].second;

                        tryInsert( w, y, joinLabelSets(ls2 , ls1) , d2 + d1 );
                    }
                }

                continue;
            }
        }

        // we push the neighbours of v1 to the stack
        // and union the labels
        SmallEdgeSet ses;
        this->graph->getOutNeighbours(v1, ses);

        for(int j = 0; j < ses.size(); j++)
        {
            TripletWD t2;
            t2.x = ses[j].first;
            t2.ls = joinLabelSets( ls1, ses[j].second );
            t2.labelDist = tr.labelDist;
            if( t2.ls != ls1 )
            {
                t2.labelDist += 1;
            }

            t2.edgeDist = d1 + 1;

            q.push( t2 );
        }
    }

};

int LandmarkedIndexWD::findPathstoLandmarks(VertexID w, bool doPropagate)
{

};

unsigned long LandmarkedIndexWD::distanceQuery(VertexID v, VertexID w, LabelSet ls)
{
    cout << "LandmarkedIndexWD::distanceQuery (" << v << "," << w << "," << ls << ")" << endl;

    if( v == w )
        return 0;

    if( ls == 0 )
    {
        return POS_INFINITY;
    }

    int N = this->graph->getNumberOfVertices();
    queue< pair<VertexID, unsigned long > > q;
    q.push( make_pair( v, 0 ) );

    dynamic_bitset<> visited = dynamic_bitset<>(N);

    int roundNo = 0;
    int count = 0;
    int noOfQueries = 0;

    while( q.empty() == false )
    {
        VertexID v1 = q.front().first;
        unsigned long dist = q.front().second;

        cout << "v1=" << v1 << ",dist=" << dist << endl;
        q.pop();

        if( visited[v1] == 1 )
        {
            continue;
        }

        visited[v1] = 1;
        roundNo++;

        if( v1 == w )
        {
            return dist;
        }

        if( vToLandmark[v1] != -1 )
        {
            int pos = 0;
            bool b = findTupleInTuples(v1, w, pos);

            if( b == true )
            {
                for(int i = 0; i < Ind[v1][pos].lsds.size(); i++)
                {
                    if( isLabelSubset(Ind[v1][pos].lsds[i].first , ls ) == true )
                    {
                        cout << "LandmarkedIndexWD::distanceQuery d1=" << dist << ",d2=" << Ind[v1][pos].lsds[i].second << endl;
                        return dist + Ind[v1][pos].lsds[i].second;
                    }
                }
            }

            continue;
        }

        SmallEdgeSet ses;
        this->graph->getOutNeighbours(v1, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( isLabelSubset(ses[j].second, ls) == true )
            {
                q.push( make_pair( ses[j].first , dist + 1 ) );
            }
        }
    }

    return POS_INFINITY;
};

bool LandmarkedIndexWD::tryInsert(VertexID v, VertexID w, LabelSet ls, int dist)
{
    if( v == w )
    {
        return false;
    }

    bool b = tryInsertLabelSet(v,w,ls,dist);
    //cout << "tryInsert v=" << v << ",w=" << w << ",ls=" << ls << ",dist=" << dist << ",b=" << b << endl;
    return b;
};

unsigned long LandmarkedIndexWD::getIndexSizeInBytes(VertexID v)
{
    if( Ind.size() <= v )
    {
        return 0;
    }

    unsigned long size = 0;
    unsigned int emptyVectorSize = 3 * sizeof(int);
    for(int i = 0; i < Ind[v].size(); i++)
    {
        size += sizeof( Ind[v][i].x );
        size += emptyVectorSize;
        size += Ind[v][i].lsds.size() * (sizeof( LabelSet ) + sizeof( int ));
    }
    return size;
};

unsigned long LandmarkedIndexWD::getIndexSizeInBytes()
{
    unsigned long size = this->graph->getGraphSizeInBytes();
    int N = this->graph->getNumberOfVertices();

    for(int i = 0; i < N; i++)
    {
        size += getIndexSizeInBytes( i );
    }

    return size;
};

void LandmarkedIndexWD::determineLandmarks()
{
    int N = graph->getNumberOfVertices();
    int M = graph->getNumberOfEdges();
    vToLandmark = vector< int >(N, -1);

    // pick the noOfLandmarks nodes with the highest
    // total-degree
    vector< pair< VertexID, int > > totDegreePerNode;

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
        //cout << "totDegreePerNode[i].first=" << outDegreePerNode[i].first << " ,totDegreePerNode[i].second=" << outDegreePerNode[i].second << endl;
        landmarks.push_back( totDegreePerNode[i].first );
        vToLandmark[ totDegreePerNode[i].first ] = i;
    }

    cout << "determineLandmarks noOfLandmarks=" << noOfLandmarks << endl;
};

bool LandmarkedIndexWD::query(VertexID source, VertexID target, LabelSet ls)
{

};

bool LandmarkedIndexWD::queryShell(VertexID source, VertexID target, LabelSet ls)
{

};

bool LandmarkedIndexWD::queryShellAdapted(VertexID source, VertexID target, LabelSet ls)
{

};

bool LandmarkedIndexWD::queryDirect(VertexID source, VertexID target, LabelSet ls)
{

};

void LandmarkedIndexWD::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};

bool LandmarkedIndexWD::extensiveQueryDirect(VertexID source, VertexID target, LabelSet ls,
    dynamic_bitset<>& marked)
{

};

void LandmarkedIndexWD::printIndexStats()
{

};
