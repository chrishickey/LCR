 #include "ClusteredExactIndex.h"

using namespace indexns;

void ClusteredExactIndex::construct(Graph* mg, IndexType pIndexType, int noOfClusters, string clusterFile)
{
    if( noOfClusters < 1 )
    {
        cerr << "ClusteredExactIndex noOfClusters < 1" << endl;
        return;
    }

    this->indexType = IndexType::Clustered;
    this->clusterType = pIndexType;
    this->noOfClusters = noOfClusters;
    this->graph = mg;
    this->indexDirection = OUTINDEX;
    this->countPerCluster = vector< int >(noOfClusters, 0);

    constStartTime = getCurrentTimeInMilliSec();
    if( clusterFile == "" )
    {
        createClustering();
    }
    else
    {
        createClustering(clusterFile);
    }

    buildIndex();
    constEndTime = getCurrentTimeInMilliSec();

    cout << "ClusteredExactIndex noOfClusters=" << noOfClusters << ",quality=" << clusterQuality() << endl;
};

ClusteredExactIndex::ClusteredExactIndex(Graph* mg, IndexType pIndexType, int noOfClusters,
    string clusterFile)
{
    construct(mg, pIndexType, noOfClusters, clusterFile);
};

ClusteredExactIndex::ClusteredExactIndex(Graph* mg, string clusterFile)
{
    construct(mg, IndexType::DoubleBFSenum, max( (int) ( mg->getNumberOfVertices() / MAX_VERTICES + 1), 1), clusterFile);
};

ClusteredExactIndex::ClusteredExactIndex(Graph* mg, int noOfClusters)
{
    construct(mg, IndexType::DoubleBFSenum, noOfClusters, "");
};

ClusteredExactIndex::ClusteredExactIndex(Graph* mg)
{
    construct(mg, IndexType::DoubleBFSenum, max( (int) ( mg->getNumberOfVertices() / MAX_VERTICES + 1), 1), "");
};

ClusteredExactIndex::~ClusteredExactIndex()
{
    for(int i = 0; i < noOfClusters; i++)
    {
        indices[i]->deconstruct();
    }
}

void ClusteredExactIndex::createClustering(string s)
{
    int N = graph->getNumberOfVertices();
    vTocID = vector< int >(N);
    vToLocalID = vector< int >(N);

    ifstream clusterFile (s);
    VertexID v;
    int vID;
    string line;

    if (clusterFile.is_open())
    {
        while ( getline (clusterFile,line) )
        {
            istringstream iss(line);
            string sV, sVID;
            iss >> sV >> sVID;

            istringstream (sV) >> v;
            istringstream (sVID) >> vID;

            countPerCluster[vID]++;

            vTocID[v] = vID;
            vToLocalID[v] = countPerCluster[vID];
        }
        clusterFile.close();
    }
    else
    {
        cerr << "createClustering: Unable to open file " << s;
    }
}

void ClusteredExactIndex::createClustering()
{
    int N = graph->getNumberOfVertices();
    vTocID = vector< int >(N);

    // create initial clustering
    int quotum = N / noOfClusters;
    int leftover = N % noOfClusters;

    unsigned int seed = time(NULL)*time(NULL) % 1000;
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<VertexID> vertexDistribution(0, N - 1);

    for(int i = 0; i < noOfClusters; i++)
    {
        VertexID start = vertexDistribution(generator);

        dynamic_bitset<> visited = dynamic_bitset<>(N);
        queue< VertexID > q;
        q.push(start);

        int desiredSize;
        int actualSize = 0;
        if( i == 0 )
        {
            desiredSize = quotum + leftover;
        }
        else
        {
            desiredSize = quotum;
        }

        while( q.empty() == false && actualSize < desiredSize )
        {
            VertexID x = q.front();
            q.pop();
            if( visited[x] == 1 )
            {
                continue;
            }
            visited[x] = 1;
            vTocID[x] = i;
            actualSize++;

            SmallEdgeSet ses;
            graph->getAllNeighbours(x, ses);
            for(int j = 0; j < ses.size(); j++)
            {
                q.push(ses[j].first);
            }
        }
    }

    // optimize on the initial clustering by swapping nodes s.t. overall
    // quality improves
    vector< int > scorePerNode = vector< int >(N, 0);
    SmallEdgeSet ses;

    for(int i = 0; i < N; i++)
    {
        graph->getAllNeighbours(i, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( vTocID[i] != vTocID[ses[j].first] )
            {
                scorePerNode[i]++;
            }
            else
            {
                scorePerNode[i]--;
            }
        }
    }

    for(int i = 0; i < N; i++)
    {
        if( scorePerNode[i] == 0 )
            continue; // perfect, no need to swap

        int minScoreI = N, minScoreJ = N;
        int bestJ = -1;
        SmallEdgeSet ses1, ses2;
        graph->getAllNeighbours(i, ses1);
        for(int j = 0; j < N; j++)
        {
            if( i == j || vTocID[i] == vTocID[j] )
                continue;

            // swap vertex i with j and estimate the effect on the quality
            int scoreI = 0, scoreJ = 0;
            graph->getAllNeighbours(j, ses2);

            for(int k = 0; k < ses1.size(); k++)
            {
                if( vTocID[j] != vTocID[ses1[k].first] )
                {
                    scoreJ++;
                }
            }

            for(int k = 0; k < ses2.size(); k++)
            {
                if( vTocID[i] != vTocID[ses2[k].first] )
                {
                    scoreI++;
                }
            }

            if( scoreI+scoreJ < minScoreI+minScoreJ )
            {
                minScoreI = scoreI;
                minScoreJ = scoreJ;
                bestJ = j;
            }
        }

        if( bestJ >= 0 )
        {
            if(minScoreI < scorePerNode[i] && minScoreJ < scorePerNode[bestJ])
            {
                int newCID = vTocID[bestJ];
                vTocID[bestJ] = vTocID[i];
                vTocID[i] = newCID;
            }
        }
    }

    // populate local id's
    vToLocalID = vector< int >(N);

    for(int i = 0; i < N; i++)
    {
        int vID = vTocID[i];
        countPerCluster[vID]++;
        vToLocalID[i] = countPerCluster[vID]-1;

        //cout << "i=" << i << " ,vID=" << vID << " ,countPerCluster[vID]=" << countPerCluster[vID] << endl;
    }

    // print clustering
    printClustering();
}

void ClusteredExactIndex::buildIndex()
{
    int N = graph->getNumberOfVertices();

    // build edge lists
    vector< EdgeSet > edgeSets = vector< EdgeSet >(noOfClusters);
    outPortsPerCluster = vector< vector < VertexID > >(noOfClusters);

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses;
        graph->getOutNeighbours(i, ses);

        for(int j = 0; j < ses.size(); j++)
        {
            LabelSet ls = ses[j].second ;/// 2;
            VertexID w = vToLocalID[ses[j].first];
            //cout << "i=" << i << " ,vToLocalID[i]=" << vToLocalID[i] << " ,ses[j].first=" << ses[j].first << " ,w=" << w << ",ls=" << ls << endl;

            if( vTocID[i] == vTocID[ses[j].first] )
            {
                Edge edge = make_pair( vToLocalID[i], make_pair( w, ls ) );
                edgeSets[vTocID[i]].push_back(edge);
            }
            else
            {
                outPortsPerCluster[vTocID[i]].push_back(i);
            }
        }
    }

    // build indices
    int L = graph->getNumberOfLabels();
    for(int i = 0; i < noOfClusters; i++)
    {
        cout << "building cluster " << i << ", |E|=" << edgeSets[i].size() << ", |V|=" << countPerCluster[i] << endl;
        DGraph* g1 = new DGraph(&edgeSets[i], countPerCluster[i], L);

        Index* index;
        //if( this->clusterType == IndexType::DoubleBFSenum )
        //{
            index = new DoubleBFS(g1);
        //}

        indices.push_back(index);
    }

    printClustering();
}

void ClusteredExactIndex::printClustering()
{
    cout << "printClustering: " << endl;
    int N = graph->getNumberOfVertices();
    vector< vector< VertexID > > listPerCluster;
    for(int i = 0; i < N; i++)
    {
        int vID = vTocID[i];
        while( vID >= listPerCluster.size() )
        {
            listPerCluster.push_back( vector< VertexID >() );
        }
        listPerCluster[vID].push_back( i );
    }

    for(int i = 0; i < noOfClusters; i++)
    {
        cout << "cluster-" << i << "{\n";
        for(int j = 0; j < listPerCluster[i].size(); j++)
        {
            cout << listPerCluster[i][j] << ", ";
        }
        cout << "\n}\n";
    }
    cout << endl;
}

bool ClusteredExactIndex::queryShell(VertexID source, VertexID target, LabelSet ls)
{
    if( source == target )
        return true;

    if( ls == 0 )
        return false;

    int N = graph->getNumberOfVertices();

    // a node v is marked iff there exists a labeled path from source to v s.t.
    // the label of that path is a subset of ls
    dynamic_bitset<> marked = dynamic_bitset<>(N);
    queue< VertexID > q;
    q.push(source);

    int roundNo = 0;
    int noOfDirectQueries = 0;

    while( q.empty() == false )
    {
        VertexID x = q.front();
        q.pop();

        if( marked[x] == 1 )
        {
            continue;
        }

        roundNo++;
        int xCID = vTocID[x];

        //cout << "queryShell x=" << x << " ,xCID=" << xCID << endl;

        if( xCID == vTocID[target] )
        {
            noOfDirectQueries++;
            // x and target are in the same cluster and hence we attempt to
            // answer the query directly
            //cout << "attempt directly vToLocalID[x]=" << vToLocalID[x] << " ,vToLocalID[target]=" << vToLocalID[target] << endl;
            if( indices[xCID]->query(vToLocalID[x],vToLocalID[target],ls) == true )
            {
                //cout << "ClusteredExactQuery roundNo=" << roundNo << " ,noOfDirectQueries=" << noOfDirectQueries << endl;
                return true;
            }
        }

        // x and target are in different clusters or in the same but not directly
        // connected and hence we need to look outside the cluster
        for(int i = 0; i < outPortsPerCluster[xCID].size(); i++)
        {
            // y is a port leaving the cluster
            VertexID y = outPortsPerCluster[xCID][i];

            if( marked[y] == 1 )
            {
                continue;
            }

            // there needs to exists some edge (y,z) in E s.t. the label of (y,z)
            // is in ls. All such z are pushed onto the stack if (x,y) are connected
            SmallEdgeSet ses;
            graph->getOutNeighbours(y, ses);
            vector< VertexID > zs;
            for(int j = 0; j < ses.size(); j++)
            {
                VertexID z = ses[j].first;
                LabelSet lsZ = ses[j].second;

                if( vTocID[z] == xCID )
                {
                    continue;
                }

                //cout << "port y=" << y << ", z=" << z << ", lsZ=" << lsZ << endl;

                if( isLabelSubset(lsZ, ls) && marked[z] == 0 )
                {
                    zs.push_back( z );
                }
            }

            noOfDirectQueries++;
            bool b1 = indices[xCID]->query(vToLocalID[x], vToLocalID[y], ls);

            if( b1 == true )
            {
                marked[y] = 1;
                for(int j = 0; j < zs.size(); j++)
                {
                    //cout << "push y=" << y << ", zs[j]=" << zs[j] << endl;
                    q.push( zs[j] );
                }
            }
        }
        marked[x] = 1;
    }

    //cout << "ClusteredExactQuery roundNo=" << roundNo << " ,noOfDirectQueries=" << noOfDirectQueries << endl;
    return false;
};

bool ClusteredExactIndex::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "ClusteredExactIndex::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls);
    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "ClusteredExactIndex::query answer=" << b << " ,time(s)=" << getLastQueryTime() << endl;
    return b;
};

double ClusteredExactIndex::clusterQuality()
{
    double quality = 0.0;
    int M = graph->getNumberOfEdges();
    int N = graph->getNumberOfVertices();
    int count = 0;
    SmallEdgeSet ses;

    for(int i = 0; i < N; i++)
    {
        graph->getOutNeighbours(i, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            if( vTocID[i] != vTocID[ses[j].first])
            {
                count++;
                break;
            }
        }
    }

    quality = 1 - ( ((double) count) / ((double) N) );

    return quality;
}

// @Override from Index.h
unsigned long ClusteredExactIndex::getIndexSizeInBytes()
{
    long size = 0;
    for(int i = 0; i < noOfClusters; i++)
    {
        // the graph size needs to be discounted
        size += indices[i]->getIndexSizeInBytes() - graph->getGraphSizeInBytes();
    }

    size += graph->getGraphSizeInBytes();
    size += 2 * sizeof(int) * graph->getNumberOfVertices(); // vTocID and vToLocalID

    for(int i = 0; i < noOfClusters; i++)
    {
        size += outPortsPerCluster[i].size() * sizeof(VertexID);
    }

    return size;
};

void ClusteredExactIndex::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};
