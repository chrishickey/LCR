#include "LandmarkedIndex.h"
#include "BFSIndex.h"

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

using namespace indexns;
using namespace landmarkedns;

void LandmarkedIndex::construct(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers,
    int othersBudget, int coverPercentage, bool doExtensive)
{
    this->graph = mg;
    this->indexType = IndexType::Landmarked;
    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();

    this->indexDirection = OUTINDEX;
    this->isBlockedMode = isBlockedMode;
    this->noOfLandmarks = noOfLandmarks;

    if( noOfLandmarks == 0 )
    {
        // cover percentage only supported for method 1
        this->method = 1;
        this->coverPercentage = min(max(coverPercentage, 1),100);
    }
    else
    {
        this->coverPercentage = 0;
    }

    this->frequency = 1;
    this->method = method;
    this->doIndexOthers = doIndexOthers;
    this->budgetOthers = max(0, othersBudget);
    this->isBlockedMode = false;
    this->propagateCode = propagateCode;
    this->doExtensive = doExtensive;

    buildIndex(0);
}

LandmarkedIndex::LandmarkedIndex(Graph* mg, bool doIndexOthers, bool doExtensive, int k, int b)
{
    construct(mg, k, 1, 0, doIndexOthers, b, 0, doExtensive);
};

LandmarkedIndex::LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers,
    int othersBudget, int coverPercentage, bool doExtensive)
{
    construct(mg, noOfLandmarks, method, propagateCode, doIndexOthers, othersBudget, coverPercentage, doExtensive);
};

LandmarkedIndex::LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget, int coverPercentage)
{
    construct(mg, noOfLandmarks, method, propagateCode, doIndexOthers, othersBudget, coverPercentage, false);
}

LandmarkedIndex::LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode, bool doIndexOthers, int othersBudget)
{
    construct(mg, noOfLandmarks, method, propagateCode, doIndexOthers, othersBudget, 0, false);
};


LandmarkedIndex::LandmarkedIndex(Graph* mg, int noOfLandmarks, int method, int propagateCode)
{
    construct(mg, noOfLandmarks, method, propagateCode, true, (int) sqrt(mg->getNumberOfVertices())/2, 0, false);
};

LandmarkedIndex::LandmarkedIndex(Graph* mg)
{
    construct(mg, (int) 8*sqrt(mg->getNumberOfVertices()), 1, 0, true, (int) sqrt(mg->getNumberOfVertices()), 0, false);
};

LandmarkedIndex::~LandmarkedIndex()
{
    deconstruct();
}

unsigned long LandmarkedIndex::getIndexSizeInBytes()
{
    int N = graph->getNumberOfVertices();
    unsigned long size = getIndexSizeInBytesM();

    int emptyVectorSize = 3*sizeof(int);
    /*for(int i = 0; i < landmarkPaths.size(); i++)
    {
        for(int j = 0; j < landmarkPaths[i].size(); j++)
        {
            size += emptyVectorSize + landmarkPaths[i][j].size() * sizeof(VertexID);
        }
    }*/
    // Each SequenceEntry uses a LabelSet and a Bitset of N/8 bytes
    for(int i = 0; i < noOfLandmarks; i++)
    {
        size += emptyVectorSize + seqEntries[i].size() * ( sizeof(LabelSet) + N/8 );
    }

    return size;
}

void LandmarkedIndex::determineLandmarks()
{
    int N = graph->getNumberOfVertices();
    int M = graph->getNumberOfEdges();
    vToLandmark = vector< int >(N, -1);

    struct sort_pred
    {
        bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right)
        {
            return left.second > right.second;
        }
    };

    if( this->coverPercentage == 0 && this->noOfLandmarks <= 0 )
    {
        this->noOfLandmarks = (int) 8*sqrt(N); // the default
    }

    if( this->method == 0 )
    {
        // pick the noOfLandmarks nodes with the highest
        // out-degree
        vector< pair< VertexID, int > > outDegreePerNode;

        for(int i = 0; i < N; i++)
        {
            SmallEdgeSet ses;
            graph->getOutNeighbours(i, ses);
            outDegreePerNode.push_back( make_pair(i , ses.size() ));
        }

        sort(outDegreePerNode.begin(), outDegreePerNode.end(), sort_pred() );

        for(int i = 0; i < noOfLandmarks; i++)
        {
            //cout << "outDegreePerNode[i].first=" << outDegreePerNode[i].first << " ,outDegreePerNode[i].second=" << outDegreePerNode[i].second << endl;
            landmarks.push_back( outDegreePerNode[i].first );
            vToLandmark[ outDegreePerNode[i].first ] = i;
        }
    }
    else if( this->method == 1 )
    {
        // pick the noOfLandmarks nodes with the highest
        // total-degree
        vector< pair< VertexID, int > > totDegreePerNode;

        for(int i = 0; i < N; i++)
        {
            SmallEdgeSet ses;
            graph->getAllNeighbours(i, ses);
            totDegreePerNode.push_back( make_pair(i , ses.size() ));
        }

        sort(totDegreePerNode.begin(), totDegreePerNode.end(), sort_pred() );

        if( coverPercentage == 0 )
        {
            for(int i = 0; i < noOfLandmarks; i++)
            {
                //cout << "totDegreePerNode[i].first=" << outDegreePerNode[i].first << " ,totDegreePerNode[i].second=" << outDegreePerNode[i].second << endl;
                landmarks.push_back( totDegreePerNode[i].first );
                vToLandmark[ totDegreePerNode[i].first ] = i;
            }
        }
        else
        {
            int sum = 0;
            int i = 0;
            while( ((sum*100)/M) < coverPercentage )
            {
                landmarks.push_back( totDegreePerNode[i].first );
                vToLandmark[ totDegreePerNode[i].first ] = i;

                //cout << "outDegreePerNode[i].first=" << totDegreePerNode[i].first << " ,outDegreePerNode[i].second=" << totDegreePerNode[i].second << endl;
                SmallEdgeSet ses;
                graph->getOutNeighbours(i, ses);
                sum += ses.size();//totDegreePerNode[i].second;

                i++;
            }

            this->noOfLandmarks = i;
            this->budgetOthers = 100 + i / 20;
        }
    }
    else if( this->method == 2 )
    {
        // coverage-based: choose landmarks based on coverage
        int maxL = (int) ceil(10*sqrt(N));
        int maxLandmarks = min(N/8, maxL);
        int minLandmarks = max(N/100, (int) ceil(sqrt(N)) );

        this->noOfLandmarks = 0;
        double d = ((double) N / (double) M);
        double desiredCoverage = max(min(0.75, d*2) , 0.25);
        vector< pair< VertexID, double > > coveragePerVertex = vector< pair< VertexID, double > >();

        for(int i = 0; i < N; i++)
        {
            int count = 0;
            queue< VertexID > q;
            dynamic_bitset<> marked = dynamic_bitset<>(N);
            q.push( i );

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

            double d1 = ((double) count / (double) N);

            coveragePerVertex.push_back( make_pair(i,d1) );
        }

        // sort by coverage
        struct sort_pred
        {
            bool operator()(const std::pair<int,double>& left, const std::pair<int,double>& right)
            {
                return left.second > right.second;
            }
        };

        sort(coveragePerVertex.begin(), coveragePerVertex.end(), sort_pred() );

        // find landmarks until you either have reached a minimal coverage
        // or the m
        double desiredTotalCoverage = maxLandmarks * desiredCoverage;
        double coverageSum = 0.0;

        for(int i = 0; i < N; i++)
        {
            if( coveragePerVertex[i].second >= desiredCoverage )
            {
                coverageSum += coveragePerVertex[i].second;
                vToLandmark[ coveragePerVertex[i].first ] = noOfLandmarks;
                noOfLandmarks++;

                landmarks.push_back( coveragePerVertex[i].first );
            }

            if( coverageSum >= desiredTotalCoverage )
            {
                break;
            }
        }

        cout << "LandmarkedIndex::determineLandmarks method=2, noOfLandmarks=" << noOfLandmarks << ", desiredCoverage=" << desiredCoverage << endl;
    }
    else if( this->method == 3 )
    {
        // random
        unsigned int seed = time(NULL)*time(NULL) % 1000;
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

        int c = 0;
        while( c < noOfLandmarks )
        {
            VertexID s = vertexDistribution(generator);
            if( vToLandmark[s] != -1 )
                continue;

            landmarks.push_back( s );
            vToLandmark[ s ] = c;

            c++;
        }
    }
    else if( this->method == 4 )
    {
        // a number of random walks along the graph until we have noOfLandmarks landmarks
        unsigned int seed = time(NULL)*time(NULL) % 1000;
        std::default_random_engine generator(seed);
        std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

        int c = 0;

        // loop
        while( c < noOfLandmarks )
        {
            // select random vertex
            VertexID s = vertexDistribution(generator);

            queue< VertexID > q;
            int maxRounds = max(1,N-1);
            q.push(s);

            vector< VertexID > L;

            // walk and keep track
            while( q.empty() == false && maxRounds > 0 )
            {
                VertexID x = q.front();
                maxRounds--;
                q.pop();

                L.push_back(x);

                SmallEdgeSet ses, ses2;
                graph->getOutNeighbours(x, ses);

                if( ses.size() == 0 )
                    continue;

                // prevents looping through cycle, generates random-walk behavior
                int start = vertexDistribution(generator) % ses.size();

                for(int i = start; i < ses.size(); i++)
                {
                    VertexID y = ses[i].first;
                    q.push(y);
                    break;
                }
            }

            if( L.size() >= max(2,noOfLandmarks+1) )
            {
                int step = max(noOfLandmarks,2);
                for(int i = step; i < L.size(); i+=step)
                {
                    if( vToLandmark[L[i]] != -1 )
                        continue;

                    landmarks.push_back( L[i] );
                    vToLandmark[ L[i] ] = c;
                    c++;

                    if( c == noOfLandmarks )
                    {
                        break;
                    }
                }
            }
            else
            {
                if( vToLandmark[s] != -1 )
                    continue;

                landmarks.push_back( s );
                vToLandmark[ s ] = c;
                c++;
            }
        }
    }
    else if( this->method == 5 )
    {
        // pick the noOfLandmarks nodes with the highest out-degree
        // but spread across the graph
        // might return less landmarks
        int c = 0;
        int range = max((int) 5,1);
        vector< pair< VertexID, int > > outDegreePerNode;
        vToLandmark = vector< int >(N, -1);
        dynamic_bitset<> marked = dynamic_bitset<>(N);

        for(int i = 0; i < N; i++)
        {
            SmallEdgeSet ses;
            graph->getOutNeighbours(i, ses);
            outDegreePerNode.push_back( make_pair(i , ses.size() ));
        }

        sort(outDegreePerNode.begin(), outDegreePerNode.end(), sort_pred() );

        while( c < noOfLandmarks && marked.count() < N )
        {
            VertexID x = outDegreePerNode[c].first;
            if( vToLandmark[x] != -1 )
                continue;

            landmarks.push_back( x );
            vToLandmark[ x ] = c;
            marked[c] = 1;

            // mark all vertices within range distance from x
            queue< pair< VertexID, int > > q;
            q.push( make_pair(x,0) );
            while( q.empty() == false )
            {
                VertexID y = q.front().first;
                int dist = q.front().second;
                q.pop();

                if( dist > range )
                {
                    continue;
                }
                marked[y] = 1;

                SmallEdgeSet ses;
                graph->getOutNeighbours(y, ses);
                for(int j = 0; j < ses.size(); j++)
                {
                    q.push( make_pair( ses[j].first, dist + 1) );
                }

            }

            c++;
        }

        noOfLandmarks = c;
    }

    // set ordering
    ordering = vector< VertexID >();
    vector< pair< VertexID, int > > outDegreePerNode;
    dynamic_bitset<> marked = dynamic_bitset<>(N);

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses;
        graph->getOutNeighbours(i, ses);
        outDegreePerNode.push_back( make_pair(i , ses.size() ));
    }

    sort(outDegreePerNode.begin(), outDegreePerNode.end(), sort_pred() );
    for(int i = 0; i < N; i++)
    {
        VertexID x = outDegreePerNode[i].first;
        if( marked[x] == 1 )
        {
            continue;
        }

        queue< VertexID > q;
        q.push( x );

        while( q.empty() == false )
        {
            VertexID y = q.front();
            q.pop();

            if( marked[y] == 1 )
            {
                continue;
            }
            marked[y] = 1;
            ordering.push_back( y );

            SmallEdgeSet ses;
            graph->getInNeighbours(y,ses);

            for(int j = 0; j < ses.size(); j++)
            {
                VertexID z = ses[j].first;
                if( marked[z] == 0 )
                {
                    q.push( z );
                }
            }
        }
    }

    cout << "LandmarkedIndex ordering.size()=" << ordering.size() << ",N=" << N << endl;

    if( coverPercentage == 0 )
        cout << "LandmarkedIndex::determineLandmarks noOfLandmarks=" << noOfLandmarks << " ,method=" << method << endl;
    else if( doIndexOthers == false )
        cout << "LandmarkedIndex::determineLandmarks noOfLandmarks=" << noOfLandmarks << " ,method=" << method << " ,cP=" << coverPercentage << endl;
    else
        cout << "LandmarkedIndex::determineLandmarks noOfLandmarks=" << noOfLandmarks << " ,method=" << method << " ,cP=" << coverPercentage
            << " bO=" << this->budgetOthers << endl;
}

void LandmarkedIndex::buildIndex(int continueCode)
{
    constStartTime = getCurrentTimeInMilliSec();

    // change the name
    updateName();
    this->didComplete = false;
    cout << "buildIndex name=" << this->name << ",continueCode=" << continueCode << endl;

    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();

    long size = 0; // estimated size of the index while being under construction
    if( continueCode == 0 )
    {
        this->landmarkTime = 0.0;
        this->othersTime = 0.0;
        this->totalConstTime = 0.0;

        hasBeenIndexed = dynamic_bitset<>(N);
        determineLandmarks();
        initializeIndex(N);

        seqEntries = vector< vector< SequenceEntry > >();
        for(int i = 0; i < noOfLandmarks; i++)
        {
            seqEntries.push_back( vector< SequenceEntry >() );
        }
    }
    if( continueCode == 1 )
    {
        // more landmarks have been included
        size = getIndexSizeInBytes();

        this->othersTime = 0.0;
        this->totalConstTime = 0.0;
    }
    if( continueCode == 2 )
    {
        // include others, only rebuild others
        size = getIndexSizeInBytes();
        this->othersTime = 0.0;
        this->totalConstTime = 0.0;
    }

    // Builds a full index for each landmark
    // and indexes the remaining vertices with paths to (a subset of) all landmarks
    int quotum = max(noOfLandmarks/20, 1);

    int I = 0;
    bool didNotComplete = false;

    // Build all (or more) landmarks
    if( continueCode == 0 || continueCode == 1 )
    {
        int progress = 0;
        double timePassed = getCurrentTimeInMilliSec()-constStartTime + landmarkTime;
        cout << this->name << "::buildIndex build landmarks, quotum=" << quotum << endl;

        for(int x = 0; x < N; x++)
        {
            VertexID i = ordering[x];
            bool isLandmark = (vToLandmark[i] != -1);
            if( isLandmark == false )
                continue;

            /* TIMEOUT is 6 hours. Or if we have not built 12% of the landmarks within
             0.25 of the TIMEOUT we also stop. */
            if(
              ( (timePassed) >= TIMEOUT ) ||
              ( (progress*100 / noOfLandmarks) < 12 && (timePassed) >= TIMEOUT/4  ) )
            {
                cout << this->name << "::buildIndex (landmarks) times out ,time(s)=" << (timePassed) << endl;
                I = i;
                didNotComplete = true;
                break;
            }

            if( size > MEM_LIMIT )
            {
                // quit, the index size is too large
                cout << this->name << "::size (landmarks) above MEM_LIMIT" << endl;
                I = i;
                didNotComplete = true;
                break;
            }

            if( hasBeenIndexed[ i ] == 1 && continueCode != 0 )
            {
                size += getIndexSizeInBytesM( i );
                continue;
            }

            int a = 0;

            // build index for landmark
            hasBeenIndexed[ i ] = 0;

            a = labelledBFSPerLM( i , this->propagateCode == 0 , false );

            hasBeenIndexed[ i ] = 1;
            size += getIndexSizeInBytesM( i );

            progress++;
            timePassed = getCurrentTimeInMilliSec()-constStartTime + landmarkTime;

            if( ((progress+1)%quotum) == 0 && progress > 0 )
            {
                double perc = progress;
                perc /= (noOfLandmarks);
                perc *= 100.0;

                cout << this->name << "::buildIndex (landmarks) progress=" << progress << " " << perc << "% , " << ",roundNo=" << a << ",Esize(kB)=" << size/1000
                    << ",time(s)=" << (timePassed) << endl;
            }

        }

        if( doExtensive == true )
        {
            for(int i = 0; i < N; i++)
            {
                if( vToLandmark[i] == -1 )
                    continue;

                for(int j = 0; j < N; j++)
                {
                    if( vToLandmark[j] == -1 || i == j )
                        continue;

                    LabelSets lss;
                    getLabelSetsPerPair(i,j,lss);

                    for(int k = 0; k < lss.size(); k++)
                    {
                        LabelSet ls1 = lss[k];
                        int id1 = findSeqEntryId(ls1, i);
                        int id2 = findSeqEntryId(ls1, j);

                        if( id1 != -1 && id2 != -1 )
                        {
                            seqEntries[ vToLandmark[i] ][ id1 ].V |= seqEntries[ vToLandmark[j] ][ id2 ].V;
                        }
                    }
                }
            }
        }

        cout << this->name << "::buildIndex built all landmarks size(kB)=" << size/1000 << " ,time(s)=" << landmarkTime << endl;
        this->landmarkTime += (getCurrentTimeInMilliSec()-constStartTime);
    }

    // In case the landmark process exceeded either its time or memory limit
    if( didNotComplete == true )
    {
        for(int i = I; i < N; i++)
        {
            bool isLandmark = vToLandmark[i] != -1;
            if( isLandmark == true )
            {
                vToLandmark[i] = -1;
                this->noOfLandmarks--;
            }
        }

        cout << this->name << "::buildIndex did not build all landmarks, noOfLandmarks=" << noOfLandmarks;
        cout << ",size(MB)=" << size/1000 << " ,time=" << (landmarkTime) << endl;

    }
    else
    {
        quotum = max( (N - noOfLandmarks)/20 , 1);
        cout << this->name << "::buildIndex build others, quotum=" << quotum << endl;

        constStartTime = getCurrentTimeInMilliSec();
        int progress = 0;
        double timePassed = getCurrentTimeInMilliSec()-constStartTime;

        if( budgetOthers > 0 && doIndexOthers == true )
        {
            for(int x = 0; x < N; x++)
            {
                VertexID i = ordering[x];
                bool isLandmark = (vToLandmark[i] != -1);
                if( isLandmark == true )
                    continue;

                /* TIMEOUT is 6 hours. Or if we have not built 12% of the landmarks within
                 0.25 of the TIMEOUT we also stop. */
                if(
                  ( (timePassed) >= TIMEOUT ) ||
                  ( (progress*100 / (N-noOfLandmarks)) < 12 && (timePassed) >= TIMEOUT/4  ) )
                {
                    cout << this->name << "::buildIndex (others) times out ,time(s)=" << (timePassed) << endl;
                    didNotComplete = true;
                    break;
                }

                if( size > MEM_LIMIT )
                {
                    // quit, the index size is too large
                    cout << this->name << "::size (others) above MEM_LIMIT" << endl;
                    I = i;
                    break;
                }

                int a = 0;
                // build index for normal node, having <budgetOthers> paths to landmarks
                hasBeenIndexed[ i ] = 0;

                a = labelledBFSPerNonLM( i , this->propagateCode == 0 );

                hasBeenIndexed[ i ] = 1;
                size += getIndexSizeInBytesM( i );
                progress++;
                timePassed = getCurrentTimeInMilliSec()-constStartTime;

                if( ((progress+1)%quotum) == 0 && progress > 0 )
                {
                    double perc = progress;
                    perc /= (N-noOfLandmarks);
                    perc *= 100.0;

                    cout << this->name << "::buildIndex (others) (o) progress=" << progress << " " << perc << "% , " << ",roundNo=" << a << ",Esize(kB)=" << size/1000
                        << ",time(s)=" << (timePassed) << endl;
                }
            }
        }

        othersTime = getCurrentTimeInMilliSec() - constStartTime;
    }

    this->didComplete = !didNotComplete;
    constEndTime = getCurrentTimeInMilliSec();
    totalConstTime = landmarkTime + othersTime;

    cout << "LandmarkedIndex construction time(s)=" << getIndexConstructionTimeInSec() << endl;
    cout << "LandmarkedIndex size=" << getIndexSizeInBytes() << ",isBlockedMode=" << this->isBlockedMode << ", noOfLandmarks="
        << this->noOfLandmarks << " ,propagateCode=" << propagateCode << endl;
};


int LandmarkedIndex::labelledBFSPerLM(VertexID w, bool doPropagate, bool isMaintenance)
{
    /* labelledBFSPerLM uses a heap  with the Triplet-struct s.t.
    whenever w hits a node v with a path with
    labelset LS it has not been visited before with a subset of LS
    */

    // start with a queue containing pair (w,0)
    priority_queue< BitEntry, vector<BitEntry>, PQBitEntries > q;

    BitEntry t;
    t.x = w;
    t.ls = 0;
    t.dist = 0;
    t.id = -1;

    q.push( t );

    labelledBFSPerLM(w, doPropagate, isMaintenance, q );
};

int LandmarkedIndex::labelledBFSPerLM(VertexID w, bool doPropagate, bool isMaintenance,
    priority_queue< BitEntry, vector<BitEntry>, PQBitEntries >& q)
{
    int roundNo = 0;
    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();

    int MAXDIST = L/4 + 1; // MAXDIST is the maximal number of labels for each SequenceEntry in ls (L=8 -> 3)
    int minReachLength = min(50 + (int) sqrt(N)/2, N); // minReachLength is the minimal number of vertices
    // that needs to be covered by a SequenceEntry

    while( q.empty() == false )
    {
        roundNo++;
        BitEntry tr = q.top();
        VertexID v1 = tr.x;
        VertexID ls1 = tr.ls;
        int id1 = tr.id;
        q.pop();

        //cout << "v1=" << v1 << ",ls1=" << ls1 << endl;

        if( this->doExtensive == true )
        {
            if( v1 != w && tr.dist > 0 && tr.dist <= MAXDIST)
            {
                if( id1 == -1 ) // tr.ls might have been created already
                {
                    id1 = findSeqEntryId(tr.ls, w);
                }
                if( id1 == -1 ) // tr.ls needs to be added, it has not been created yet
                {
                    id1 = insertSeqEntry(w, tr.ls);
                }

                if( hasBeenIndexed[v1] == 0 ) // v1 is always a landmark in this case
                {
                    seqEntries[ vToLandmark[w] ][ id1 ].V[ v1 ] = 1;
                }
                else
                {
                    int idx = findSeqEntryId(ls1, v1);
                    if( idx != -1 )
                    {
                        seqEntries[ vToLandmark[w] ][ id1 ].V |= seqEntries[ vToLandmark[v1] ][ idx ].V;
                    }
                }
            }
        }

        // we try to insert the entry (v1,ls1) to cIn[w]
        // b is false if and only if inserting (v1,ls1) would create
        // a conflict in cIn[w], that is adding a superset of an existing entry
        if( w != v1 )
        {
            bool b = tryInsert(w, v1, ls1);
            if( b == false )
            {
                if( isMaintenance == true && hasBeenIndexed[v1] == 1 && vToLandmark[v1] != -1 )
                {

                }
                else
                {
                    continue;
                }
            }
        }

        if( doPropagate == true )
        {
            if( hasBeenIndexed[v1] == 1 && vToLandmark[v1] != -1 )
            {
                // v1 has been indexed and is a landmark
                // we can copy all entries from v1 and do not need to get further here
                VertexID x;

                if( doIndexOthers == false )
                {
                    x = vToLandmark[v1];
                }
                else
                {
                    x = v1;
                }

                for(int i = 0; i < tIn[x].size(); i++)
                {
                    VertexID y = tIn[x][i].first;
                    if( y == w )
                        continue;

                    for(int j = 0; j < tIn[x][i].second.size(); j++)
                    {
                        tryInsert(w, y, joinLabelSets(tIn[x][i].second[j],ls1) );
                    }
                }

                continue;
            }
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
            int id2 = id1;

            if( v2 == w )
            {
                continue;
            }

            int dist = tr.dist;
            if( ls3 != ls1 || ls3 != ls2 )
            {
                dist += 1; // labels are added one by one
                id2 = -1;
            }

            BitEntry tr2;
            tr2.x = v2;
            tr2.ls = ls3;
            tr2.dist = dist;
            tr2.id = id2;

            q.push( tr2 );
        }
    }

    if( this->doExtensive == false )
    {
        return roundNo;
    }

    for(int j = 0; j < seqEntries[ vToLandmark[w] ].size(); j++)
    {
        seqEntries[ vToLandmark[w] ][j].count = seqEntries[ vToLandmark[w] ][j].V.count();
    }

    /*
        First we need to merge the entries
    */
    int i = 0;
    while( i < 2 ) // It could be done in a single iteration if the order of the labelsets were from small to large
    {
        for(int j = 0; j < seqEntries[ vToLandmark[w] ].size(); j++)
        {
            LabelSet ls1 = seqEntries[ vToLandmark[w] ][j].ls;

            for(int k = 0; k < seqEntries[ vToLandmark[w] ].size(); k++)
            {
                LabelSet ls2 = seqEntries[ vToLandmark[w] ][k].ls;

                if( j != k && isLabelSubset(ls1, ls2) == true )
                {
                    seqEntries[ vToLandmark[w] ][k].V |= seqEntries[ vToLandmark[w] ][j].V;
                }
            }
        }
        i++;
    }

    for(int j = 0; j < seqEntries[ vToLandmark[w] ].size(); j++)
    {
        seqEntries[ vToLandmark[w] ][j].count = seqEntries[ vToLandmark[w] ][j].V.count();
    }

    /*
        Remove those entries that not add minPathLength more than any of their subsets
    */
    for(int j = 0; j < seqEntries[ vToLandmark[w] ].size(); j++)
    {
        LabelSet ls1 = seqEntries[ vToLandmark[w] ][j].ls;
        int c1 = (double) seqEntries[ vToLandmark[w] ][j].count;

        for(int k = 0; k < seqEntries[ vToLandmark[w] ].size(); k++)
        {
            LabelSet ls2 = seqEntries[ vToLandmark[w] ][k].ls;
            int c2 = (double) seqEntries[ vToLandmark[w] ][k].count;

            if( j != k && isLabelSubset(ls1, ls2) == true )
            {
                if( (c2-c1) <= minReachLength )
                {
                    seqEntries[ vToLandmark[w] ].erase( seqEntries[ vToLandmark[w] ].begin() + k );
                    k--;
                }
            }
        }
    }

    /* We wish to only retain those SequenceEntry with at most MAXDIST labels
    * and at least minPathLength bits set.
    */
    vector< pair< LabelSet, int > > Lv = vector< pair< LabelSet, int > >();

    struct sort_pred
    {
        bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right)
        {
            return left.second > right.second;
        }
    };

    for(int i = 0; i < seqEntries[ vToLandmark[w] ].size(); i++)
    {
        LabelSet ls = seqEntries[ vToLandmark[w] ][i].ls;
        int c = seqEntries[ vToLandmark[w] ][i].count;
        if( c < minReachLength )
        {
            seqEntries[ vToLandmark[w] ].erase( seqEntries[ vToLandmark[w] ].begin() + i );
            i--;
            continue;
        }
        Lv.push_back( make_pair(ls, c) );
    }

    // sort on the count
    sort(Lv.begin(), Lv.end(), sort_pred() );
    double avg = 0.0;
    for(int i = 0; i < Lv.size(); i++)
    {
        int ID = findSeqEntryId(Lv[i].first, w);

        // swap the two
        SequenceEntry r = seqEntries[ vToLandmark[w] ][ i ];
        seqEntries[ vToLandmark[w] ][ i ] = seqEntries[ vToLandmark[w] ][ ID ];
        seqEntries[ vToLandmark[w] ][ ID ] = r;

        //avg += seqEntries[ vToLandmark[w] ][i].count;

        //cout << "*w=" << w << ",ls=" << seqEntries[ vToLandmark[w] ][ i ].ls << ",c=" << seqEntries[ vToLandmark[w] ][ i ].count << endl;
    }

    //avg /= seqEntries[ vToLandmark[w] ].size();
    //cout << "*w=" << w << " #seqEntries=" << seqEntries[ vToLandmark[w] ].size() << " ,avg=" << avg << endl;

    return roundNo;
};

int LandmarkedIndex::labelledBFSPerNonLM(VertexID w, bool doPropagate)
{
    int roundNo = 0;
    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();

    //vector< vector< int > > cPerVertex = vector< vector< int > >(N);
    dynamic_bitset<> marked = dynamic_bitset<>(N);

    Quadret t;
    t.x = w;
    t.y = w;
    t.ls = 0;
    t.dist = 0;

    priority_queue< Quadret, vector<Quadret>, priorityQueueQuadrets > q;
    q.push( t );

    int entriesLeft = budgetOthers;
    int MAXDIST = L/4 + 1;

    while( q.empty() == false && entriesLeft > 0 )
    {
        Quadret tr = q.top();
        q.pop();

        if( marked[tr.x] == 1 || tr.dist > MAXDIST )
        {
            continue;
        }
        marked[tr.x] = 1;

        // we have a landmark, insert it
        if( vToLandmark[tr.x] > -1 )
        {
            bool b = tryInsert(w, tr.x, tr.ls);
            if( b )
                entriesLeft--;
        }

        if( doPropagate == true )
        {
            if( hasBeenIndexed[tr.x] == 1 )
            {
                // v1 has been indexed
                // we can copy all entries from v1 and do not need to get further here
                VertexID x = tr.x;

                for(int i = 0; i < tIn[x].size(); i++)
                {
                    // y is a landmark and not equal to w
                    VertexID y = tIn[x][i].first;
                    if( y == w || vToLandmark[y] == -1 )
                        continue;

                    for(int j = 0; j < tIn[x][i].second.size(); j++)
                    {
                        LabelSet ls3 = joinLabelSets(tIn[x][i].second[j],tr.ls);
                        int dist3 = getNumberOfLabelsInLabelSet( ls3 );
                        if( dist3 > MAXDIST )
                            continue;

                        bool b = tryInsert(w, y, ls3 );
                        if( b )
                            entriesLeft--;
                    }

                    if( entriesLeft <= 0 )
                    {
                        break;
                    }

                }

                /*if( vToLandmark[x] > -1 )
                    continue;*/
            }
        }

        // no landmark, just push the out-edges
        SmallEdgeSet ses;
        graph->getOutNeighbours(tr.x, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            Quadret qr;
            qr.x = ses[j].first;
            qr.y = tr.x;
            qr.ls = joinLabelSets( tr.ls, ses[j].second );
            qr.dist = tr.dist;
            if( qr.ls != tr.ls )
            {
                qr.dist += 1;
            }

            q.push( qr );
        }

        roundNo++;
    }

    return roundNo;
}

int LandmarkedIndex::findSeqEntryId(LabelSet ls, VertexID w)
{
    // assumes w is a landmark
    if( ls == 0 )
    {
        return -1;
    }

    for(int i = 0; i < seqEntries[ vToLandmark[w] ].size(); i++)
    {
        LabelSet ls1 = seqEntries[ vToLandmark[w] ][i].ls;

        if( ls == ls1 )
        {
            return i;
        }
    }

    return -1;
}

int LandmarkedIndex::insertSeqEntry(VertexID w, LabelSet ls)
{
    // assumes w is a landmark
    int N = graph->getNumberOfVertices();

    SequenceEntry se;
    se.ls = ls;
    se.V = dynamic_bitset<>(N);
    se.count = 0;
    seqEntries[ vToLandmark[w] ].push_back( se );

    return seqEntries[ vToLandmark[w] ].size() - 1;
};

bool LandmarkedIndex::tryInsert(VertexID w, VertexID v, LabelSet ls)
{
    //cout << "tryInsert w=" << w << " ,v=" << v << " ,ls=" << ls << endl;

    if( doIndexOthers == false )
    {
        if( vToLandmark[w] == -1 )
        {
            return false;
        }

        if( w == v )
        {
            return false;
        }

        bool b2 = tryInsertLabelSetToIndex(ls, vToLandmark[w], v);

        return b2;
    }
    else
    {
        if( w == v )
        {
            return false;
        }

        bool b2 = tryInsertLabelSetToIndex(ls, w, v);

        return b2;
    }
}

bool LandmarkedIndex::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "LandmarkedIndex::query source=" << to_string(source) << ",target=" << to_string(target)
    //    << ",ls=" << ls << ",vToLandmark[source]=" << vToLandmark[source] << endl;

    queryStart = getCurrentTimeInMilliSec();
    bool b;
    if( doIndexOthers == false )
    {
        if( vToLandmark[source] == -1 )
        {
            b = queryShell(source, target, ls);
        }
        else
        {
            b = queryDirect(source, target, ls);
        }
    }
    else
    {
        if( vToLandmark[source] == -1 )
        {
            b = queryShellAdapted(source, target, ls);
        }
        else
        {
            b = queryDirect(source, target, ls);
        }
    }

    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "LandmarkedIndex::query answer =" << b << endl;
    return b;
}

void LandmarkedIndex::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{
    if( doIndexOthers == false )
    {
        cerr << "queryAll doIndexOthers == false not supported." << endl;
        return;
    }

    queryStart = getCurrentTimeInMilliSec();

    queue< VertexID > q;
    q.push( source );

    int N = graph->getNumberOfVertices();
    int frequency = 3;
    int count = 0;
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

        if( vToLandmark[x] > -1 )
        {
            if( count%frequency == 0 )
            {
                // seqEntries[ vSource ][i].V;
                for(int i = 0; i < tIn[x].size(); i++)
                {
                    VertexID x1 = tIn[x][i].first;
                    if( canReach[x1] == 1 )
                    {
                        continue;
                    }

                    for(int j = 0; j < tIn[x][i].second.size(); j++)
                    {
                        if( isLabelSubset(tIn[x][i].second[j],ls) == true )
                        {
                            canReach[x1] = 1;
                            break;
                        }
                    }
                }

                count++;
                continue;
            }
            else
            {
                count++;
            }
        }

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

bool LandmarkedIndex::queryShellAdapted(VertexID source, VertexID target, LabelSet ls)
{
    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    int N = graph->getNumberOfVertices();
    int roundNo = 0;
    int count = 0;
    int noOfQueries = 0;
    int FREQ = 3;

    dynamic_bitset<> visited = dynamic_bitset<>(N);

    // freq determines how often we will do an extensive direct query
    int cc = 0;
    if( this->doExtensive == true )
        cc = 2;

    // try each reachable landmark from source
    for(int i = 0; i < tIn[source].size(); i++)
    {
        VertexID x = tIn[source][i].first; // x is a landmark

        if( visited[x] == 1 )
            continue;

        for(int j = 0; j < tIn[source][i].second.size(); j++)
        {
            LabelSet ls1 = tIn[source][i].second[j];

            //cout << "x=" << x << " ,vToLandmark[x]=" << vToLandmark[x] << " ,ls1=" << ls1 << endl;

            if( isLabelSubset(ls1, ls) == true )
            {
                if( cc > 0 )//&& lSize > 4 )
                {
                    cc--;
                    if( extensiveQueryDirect(x, target, ls, visited) == true )
                    {
                        return true;
                    }
                }
                else
                {
                    // x can be reached from source
                    if( queryDirect(x, target, ls) == true )
                    {
                        return true;
                    }
                }

                // x does not need to be considered any time later
                visited[x] = 1;
                break;
            }
        }
    }

    // the remaining part is similar to query shell
    queue< VertexID > q;
    q.push( source );

    while( q.empty() == false )
    {
        VertexID v1 = q.front();
        q.pop();

        if( visited[v1] == 1 )
        {
            continue;
        }
        visited[v1] = 1;
        roundNo++;

        //cout << "v1=" << v1 << endl;

        if( v1 == target )
        {
            return true;
        }

        if( vToLandmark[v1] != -1 )
        {
            count++;
            if( count%FREQ == 0 )
            {
                /*
                We can either run a "extensive" direct query or just a direct
                query.

                An extensive direct query not only looks for w in the index of
                landmark v1, but to all other nodes as well. Any node that can be
                reached from v1 does not have to be taken into account in the BFS
                any more.
                */

                if( extensiveQueryDirect(v1, target, ls, visited) )
                {
                    return true;
                }

                continue;
            }
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
}

bool LandmarkedIndex::queryShell(VertexID source, VertexID target, LabelSet ls)
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
    int count = 0;
    int noOfQueries = 0;

    while( q.empty() == false )
    {
        VertexID v1 = q.front();
        q.pop();

        if( visited[v1] == 1 )
        {
            continue;
        }
        visited[v1] = 1;
        roundNo++;

        //cout << "v1=" << v1 << endl;

        if( v1 == target )
        {
            return true;
        }

        if( vToLandmark[v1] != -1 )
        {
            count++;
            if( count%frequency == 0 )
            {
                /*noOfQueries++;
                if( noOfQueries % freq == 0 )
                {
                    if( extensiveQueryDirect(v1, target, ls, visited) )
                    {
                        //cout << "noOfQueries=" << noOfQueries << endl;
                        return true;
                    }
                }
                else
                {*/
                    if( queryDirect(v1, target, ls) )
                    {
                        //cout << "noOfQueries=" << noOfQueries << endl;
                        return true;
                    }
                //}

                continue;
            }
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

bool LandmarkedIndex::queryDirect(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "LandmarkedIndex::queryDirect source=" << source << " ,target=" << target << " ,ls=" << labelSetToString(ls) << ", vToLandmark[source]=" << vToLandmark[source] << endl;

    if(source == target)
        return true;

    if( ls == 0 )
        return false;

    if( doIndexOthers == false )
    {
        if( isBlockedMode == true )
        {
            for(int i = 0; i < cIn[vToLandmark[source]][target].size(); i++)
            {
                LabelSet ls2 = cIn[vToLandmark[source]][target][i];

                if( isLabelSubset(ls2,ls) == true )
                    return true;
            }
        }
        else
        {
            int pos = 0;
            bool b = findTupleInTuples(target, tIn[ vToLandmark[source] ], pos);
            if( b == true )
            {
                for(int i = 0; i < tIn[vToLandmark[source]][pos].second.size(); i++)
                {
                    LabelSet ls2 = tIn[vToLandmark[source]][pos].second[i];

                    if( isLabelSubset(ls2,ls) == true )
                        return true;
                }
            }
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

bool LandmarkedIndex::extensiveQueryDirect(VertexID source, VertexID target, LabelSet ls,
    dynamic_bitset<>& marked)
{
    //cout << "s=" << source << " ,t=" << target << " ,ls=" << ls << " ,vID=" << vToLandmark[source] << endl;

    // first try direct
    if( queryDirect(source, target, ls) == true )
        return true;

    // then prune as much as possible
    //int L = graph->getNumberOfLabels();
    int vSource = vToLandmark[source];
    /*for(int i = 0; i < L; i++)
    {
        LabelSet ls1 = (1 << i) & ls;
        if( ls1 == 0 )
            continue;

        for(int k = 0; k < landmarkPaths[ vSource ][ mapping[i] ].size(); k++)
        {
            VertexID x = landmarkPaths[ vSource ][ mapping[i] ][k];
            //cout << "lID=" << lID << " ,x=" << x << endl;
            marked[ x ] = 1;
        }

    }*/

    for(int i = 0; i < seqEntries[ vSource ].size(); i++)
    {
        if( isLabelSubset(seqEntries[ vSource ][i].ls,ls) == true )
        {
            marked |= seqEntries[ vSource ][i].V;

            break;
        }
    }

    return false;
}

void LandmarkedIndex::printIndexStats()
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

double LandmarkedIndex::computeQueryFilterProbability(LabelSet ls)
{
    double p = 0.0;
    int L = graph->getNumberOfLabels();
    /*long M = max(graph->getNumberOfEdges(),1);
    long mass = 0;
    for(int i = 0; i < L; i++)
    {
        int bit = ls & (1 << i);
        //cout << "ls=" << ls << ", (1 << i)=" << (1 << i) << ", bit=" << bit << endl;
        if( bit  != 0 )
        {
            mass += graph->getCountPerLabel(i);
            cout << "mass=" << mass << endl;
        }
    }

    p = ((double) mass) / ((double) M);*/
    p = ((double) getNumberOfLabelsInLabelSet(ls) ) / ((double) L);
    //cout << "p=" << p << endl;
    return p;
};

void LandmarkedIndex::addNode()
{
    this->graph->addNode();
};

void LandmarkedIndex::removeNode(VertexID v)
{
};

void LandmarkedIndex::addEdge(VertexID v, VertexID w, LabelID lID)
{
    double start = getCurrentTimeInMilliSec();
    int N = graph->getNumberOfVertices();

    hasBeenIndexed.reset();
    hasBeenIndexed[w] = 1;

    queue< VertexID > q;
    q.push(v);
    this->graph->addEdge(v,w,lID);

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();

        if( hasBeenIndexed[ x ] == 1 )
        {
            continue;
        }

        bool isLandmark = (vToLandmark[x] != -1);
        int a = 0;
        if( isLandmark == true )
        {
            // build index for landmark
            a = labelledBFSPerLM( x , this->propagateCode == 0, true );
        }
        else
        {
            // build index for normal node, having <budgetOthers> paths to landmarks
            //a = labelledBFSPerNonLM( x , this->propagateCode == 0 );
        }
        hasBeenIndexed[ x ] = 1;

        SmallEdgeSet ses;
        graph->getInNeighbours(x, ses);

        for(int j = 0; j < ses.size(); j++)
        {
            q.push( ses[j].first );
        }
    }

    double end = getCurrentTimeInMilliSec();

    cout << "addEdge time(s)=" << (end-start) << ",size(byte)=" << getIndexSizeInBytes() << endl;
};

void LandmarkedIndex::removeEdge(VertexID v, VertexID w, LabelID lID)
{
    double start = getCurrentTimeInMilliSec();
    int N = graph->getNumberOfVertices();

    // An edge (v,w,lID) is about to be deleted

    // w is the only vertex guaranteed to not be affected here
    hasBeenIndexed.reset();
    hasBeenIndexed[w] = 1;

    LabelSet ls = labelIDToLabelSet( this->graph->getLabelID(v,w) );
    this->graph->removeEdge(v,w);

    // promote w to a landmark if w is not
    if( vToLandmark[ w ] == -1 )
    {
        vToLandmark[ w ] = this->noOfLandmarks;
        this->noOfLandmarks++;

        tIn[w].clear();
        labelledBFSPerLM(w, true , false );
        hasBeenIndexed[ w ] = 1;
    }

    // the same for v
    if( vToLandmark[ v ] == -1 )
    {
        vToLandmark[ v ] = this->noOfLandmarks;
        this->noOfLandmarks++;
    }

    tIn[v].clear();
    labelledBFSPerLM(v, true , false );
    hasBeenIndexed[ v ] = 1;

    // find all labelsets between v and w
    LabelSets lss1,lss2;
    getLabelSetsPerPair(v,w,lss1);

    // if ls is one of the lss, we can stop
    // as v can still reach w through ls
    bool skipRest = false;
    for(int i = 0; i < lss1.size(); i++)
    {
        //cout << "ls=" << ls << " lss1[i]=" << lss1[i] << endl;

        if( ls == lss1[i] )
        {
            skipRest = true;
            break;
        }
    }

    if( skipRest == false )
    {
        for(int x = 0; x < N; x++)
        {
            if( x == w )
                continue;

            //cout << "removeEdge x=" << x << endl;

            bool isLandmark = vToLandmark[x] != -1;
            dynamic_bitset<> marked = dynamic_bitset<>(N);

            for(int i = 0; i < tIn[w].size(); i++)
            {
                VertexID y = tIn[w][i].first;

                int pos = 0;
                bool b1 = findTupleInTuples(y, tIn[x], pos);

                if( b1 == false )
                    continue;

                if( y == w )
                {
                    tIn[x][pos].second.clear();
                    marked[y] = 1;
                }
                else
                {
                    for(int j = 0; j < tIn[w][i].second.size(); j++)
                    {
                        LabelSet ls2 = joinLabelSets( tIn[w][i].second[j] , ls );
                        //cout << "- removeEdge ls2=" << ls2 << endl;

                        for(int k = 0; k < tIn[x][pos].second.size(); k++)
                        {
                            LabelSet ls3 = tIn[x][pos].second[k];

                            if( isLabelSubset(ls2,ls3) == true )
                            {
                                tIn[x][pos].second.erase( tIn[x][pos].second.begin() + k );
                                k--;

                                //cout << "- removeEdge y=" << y << ",ls=" << ls2 << endl;

                                marked[y] = 1;
                            }
                        }
                    }
                }
                //cout << "- removeEdge y=" << y << endl;


            }

            if( isLandmark )
            {
                priority_queue< BitEntry, vector<BitEntry>, PQBitEntries > q;
                dynamic_bitset<> hasBeenPushed = dynamic_bitset<>(N);
                for(int y = 0; y < N; y++)
                {
                    if( marked[y] == 0 )
                        continue;

                    // push all of y's in-neighbours to the q
                    SmallEdgeSet ses;
                    this->graph->getInNeighbours(y, ses);

                    for(int j = 0; j < ses.size(); j++)
                    {
                        if( hasBeenPushed[ ses[j].first ] == 1 )
                            continue;

                        int pos;
                        bool b2 = findTupleInTuples(ses[j].first, tIn[x], pos);

                        if( b2 == false )
                            continue;

                        hasBeenPushed[ ses[j].first ] = 1;

                        for(int k = 0; k < tIn[x][pos].second.size(); k++)
                        {
                            //cout << "- z=" << ses[j].first << ",ls=" << tIn[x][pos].second[k] << endl;

                            BitEntry t;
                            t.x = y;
                            t.ls = joinLabelSets(tIn[x][pos].second[k] , ses[j].second);
                            t.dist = getNumberOfLabelsInLabelSet( t.ls );
                            t.id = -1;

                            q.push(t);
                        }
                    }

                    BitEntry t;
                    t.x = x;
                    t.ls = 0;
                    t.dist = 0;
                    t.id = 1;
                    q.push(t);

                    //cout << "q.size()=" << q.size() << endl;

                    labelledBFSPerLM(x, true, false, q);
                }

                hasBeenIndexed[ x ] = 1;
            }
        }
    }

    double end = getCurrentTimeInMilliSec();

    cout << "removeEdge (" << v << "," << w << "," << labelSetToLabelID( ls ) << "==" << ls << "), time(s)=" << (end-start) << ",size(byte)=" << getIndexSizeInBytes() << endl;
};

void LandmarkedIndex::changeLabel(VertexID v, VertexID w, LabelID lID)
{
};

void LandmarkedIndex::setDoExtensive(bool doExtensive)
{
    cout << this->doExtensive << " changed to doExtensive=" << doExtensive << endl;
    this->doExtensive = doExtensive;

    updateName();
};

void LandmarkedIndex::updateName()
{
    this->name = "LI";

    if( doIndexOthers == true )
    {
        this->name += "+OTH";
    }

    if( doExtensive == true )
    {
        this->name += "+EXTv2-b=" + to_string(this->budgetOthers);
    }

    this->name += "-k=" + to_string(this->noOfLandmarks);
}

void LandmarkedIndex::setOthersBudget(int budgetOthers)
{
    cout << this->budgetOthers << " changed to budgetOthers=" << budgetOthers << endl;
    this->budgetOthers = budgetOthers;
    updateName();
};

void LandmarkedIndex::setNoOfLandmarks(int pnoOfLandmarks)
{
    if( this->noOfLandmarks >= pnoOfLandmarks )
    {
        cerr << "setNoOfLandmarks new value should be > than old value" << endl;
        return;
    }

    cout << this->name << " from noOfLandmarks=" << this->noOfLandmarks << " to noOfLandmarks=" << pnoOfLandmarks << endl;
    this->noOfLandmarks = pnoOfLandmarks;
    updateName();

    // TO DO:
    // not the most beautiful solution, I admit. Only supports method 1 and only
    // works if you increase the number of landmarks
    int N = graph->getNumberOfVertices();
    vector< pair< VertexID, int > > totDegreePerNode;
    landmarks.clear();

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

    dynamic_bitset<> marked = dynamic_bitset<>(N);
    for(int i = 0; i < noOfLandmarks; i++)
    {
        VertexID x = totDegreePerNode[i].first;
        if( vToLandmark[ x ] == -1 )
        {
            hasBeenIndexed[ x ] = 0;
            tIn[x].clear(); // clear entries to landmarks only
        }

        landmarks.push_back( x );
        vToLandmark[ x ] = i;

        marked[ x ] = 1;
    }

    // remove old landmarks
    for(int i = 0; i < N; i++)
    {
        if( marked[i] == 1 )
        {
            continue;
        }

        if( vToLandmark[ i ] != -1 )
        {
            vToLandmark[i] = -1;
            tIn[i].clear(); // clear entries to landmarks only
        }
    }

    cout << this->name << " number of landmarks successfully changed to noOfLandmarks=" << this->noOfLandmarks  << endl;
};
