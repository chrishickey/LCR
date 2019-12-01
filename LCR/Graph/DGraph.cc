#include "DGraph.h"

#include <climits>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <queue>
#include <map>
#include <sys/time.h>
#include <limits>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <unistd.h>

using namespace std;

void DGraph::construct(EdgeSet* edgeSet, int pN, int pL, bool allowMultipleEdges)
{
    this->constStartTime = getCurrentTimeInMilliSec();

    this->allowMultipleEdges = allowMultipleEdges;
    if( pN == -1 || pL == -1 )
    {
        loadEdgeStats(edgeSet);
    }
    else
    {
        this->N = pN;
        this->L = pL;
        this->M = edgeSet->size();
    }

    buildGraph(edgeSet);
    this->constEndTime = getCurrentTimeInMilliSec();
    this->allowMultipleEdges = false;
    //cout << "DGraph |V|=" << N << ",|E|=" << M << ",|L|=" << L << endl;
    //cout << "DGraph size(byte)=" << getGraphSizeInBytes() << ", time(s)=" << getGraphConstructionTime() << endl;
}

DGraph::DGraph(EdgeSet* edgeSet, int pN, int pL, bool allowMultipleEdges)
{
    construct(edgeSet, pN, pL, allowMultipleEdges);
};

DGraph::DGraph(EdgeSet* edgeSet, int pN, int pL)
{
    construct( edgeSet, pN, pL, false );
};

DGraph::DGraph(EdgeSet* edgeSet)
{
    construct( edgeSet, -1, -1, false );
};

DGraph::DGraph(string fileName)
{
    construct( loadEdgeFile(fileName), -1, -1, false );
};

DGraph::~DGraph()
{

};

EdgeSet* DGraph::loadEdgeFile(string fileName)
{
    cout << "DGraph fileName=" << fileName << endl;

    EdgeSet* edgeSet = new EdgeSet;

    string line;
    VertexID f, t;
    LabelID l2;

    ifstream edge_file (fileName);
    if (edge_file.is_open())
    {
        while ( getline (edge_file,line) )
        {
            istringstream iss(line);
            string from, label, to;
            iss >> from >> to >> label;
            istringstream (from) >> f;
            istringstream (to) >> t;
            istringstream (label) >> l2;

            l2 = labelIDToLabelSet(l2);

            //cout << "loadEdgeFile: (from,to,label) = " << to_string(f) << ","
            //    << to_string(t) << "," << to_string(l2) << endl;

            Edge edge = make_pair(f , make_pair(t, l2));
            edgeSet->push_back( edge );
        }
        edge_file.close();
    }
    else
    {
        cerr << "loadEdgeFile: Unable to open file " << fileName;
    }

    return edgeSet;
};

bool DGraph::findInsertablePosition(graphns::VertexID w, SmallEdgeSet& ses, int& pos)
{
    if( ses.size() == 0 )
    {
        return false;
    }

    int low = 0;
    int high = ses.size() - 1;
    int mid = floor( (low + high) / 2 );

    while( high >= low )
    {
        mid = floor( (low + high) / 2 );
        //cout << "findTupleInTuples loop mid=" << mid << " low=" << low << " high=" << high << " tus[mid].first=" << tus[mid].first << endl;

        if( ses[mid].first == w )
        {
            pos = mid;
            return true;
        }

        if( ses[mid].first > w )
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }

    }

    pos = mid;
    if( ses[mid].first < w)
        pos++;

    //cout << "findTupleInTuples i=" << i << endl;
    return false;
};

void DGraph::insertEdge(graphns::VertexID v, graphns::VertexID w, graphns::LabelID newLabel, SmallEdgeSet& ses)
{
    // inserts an edge (v,w,newLabel) into ses keeping ses sorted
    int pos = 0;
    bool b1 = findInsertablePosition(w, ses, pos);

    if( b1 == false )
    {
        LabelSet ls = newLabel;
        ses.insert( ses.begin() + pos , make_pair(w, ls) );
    }
};

void DGraph::buildGraph(graphns::EdgeSet* edges)
{
    // the edgeset does not have to be sorted but for performance this
    // is recommended

    // first initialize outE and inE
    outE.clear();
    inE.clear();

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet ses1 = SmallEdgeSet();
        SmallEdgeSet ses2 = SmallEdgeSet();
        outE.push_back(ses1);
        inE.push_back(ses2);
    }

    // add edges to outE and inE
    int i = 0;
    while( i < edges->size() )
    {
        Edge edge = edges->at(i);
        VertexID v = edge.first;
        VertexID w = edge.second.first;
        LabelID lID = edge.second.second;

        //cout << "buildGraph v=" << v << ",w=" << w << ",lID=" << lID << endl;

        insertEdge(v,w,lID, outE[v]);
        insertEdge(w,v,lID, inE[w]);

        i++;
    }

    //cout << "buildGraph completed" << endl;
};

void DGraph::loadEdgeStats(EdgeSet* edgeSet)
{
    M = edgeSet->size();

    VertexID last = 0;
    VertexID maxvID = 0;
    set< LabelID > labels;

    int i = 0;
    countPerLabel = vector< long >();

    while( i < edgeSet->size() )
    {
        Edge e = edgeSet->at(i);
        VertexID v = e.first;
        VertexID w = e.second.first;
        LabelID label = e.second.second;

        if( v > maxvID || w > maxvID )
        {
            maxvID = std::max(v,w);
        }

        labels.insert(label);
        while( countPerLabel.size() < label+1 )
        {
            countPerLabel.push_back( 0 );
        }
        countPerLabel[ label ] += 1;

        i++;
    }

    N = maxvID + 1;
    L = labels.size();

};

int DGraph::getGraphSizeInBytes()
{
    int size = 0;
    for(int i = 0; i < N; i++)
    {
        //cout << "getGraphSizeInBytes outE[i].size()=" << outE[i].size() << ",i=" << i << endl;
        for(int j = 0; j < outE[i].size(); j++)
        {
            //cout << "getGraphSizeInBytes size=" << size << ",i=" << i << ",j=" << j << endl;
            size += sizeof(outE[i][j].first);
            size += sizeof(outE[i][j].second);
        }
    }

    size *= 2; // inE and outE have the same 'size'

    return size;
};

double DGraph::getGraphConstructionTime()
{
    return max(constEndTime - constStartTime, 0.00000001);
};

void DGraph::getOutNeighbours(graphns::VertexID w, SmallEdgeSet& outNeighbours)
{
    outNeighbours.clear();
    outNeighbours = outE[w];
};

void DGraph::getInNeighbours(graphns::VertexID w, SmallEdgeSet& inNeighbours)
{
    inNeighbours.clear();
    inNeighbours = inE[w];
};

void DGraph::getAllNeighbours(graphns::VertexID w, SmallEdgeSet& allNeighbours)
{
    allNeighbours.clear();
    allNeighbours = inE[w];
    allNeighbours.insert(  allNeighbours.end(), outE[w].begin(), outE[w].end() );
}

// prints stats of the graph
std::string DGraph::toString()
{
    string output = "";
    output += "|V| = " + to_string(N) + "\n";
    output += "|E| = " + to_string(M) + "\n";
    output += "|L| = " + to_string(L) + "\n";
    output += "size in bytes = " + to_string(getGraphSizeInBytes()) + "\n";
    output += "--------------------------\n";

    //cout << "toString output=" << output << endl;

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet outN;
        getOutNeighbours(i, outN);

        output += "out(" + to_string(i) + ")= { \n";

        //cout << "i=" << i << ",outN.size()=" << outN.size() << endl;
        for(int j = 0; j < outN.size(); j++)
        {
            output += "(" + to_string(outN[j].first) + ",<" + labelSetToLetter(outN[j].second)
                + "==" + labelSetToString(outN[j].second) + ">)\n";
        }

        output += "} \n";
    }

    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet inN;
        getInNeighbours(i, inN);

        output += "in(" + to_string(i) + ")= { \n";

        //cout << "i=" << i << ",outN.size()=" << outN.size() << endl;
        for(int j = 0; j < inN.size(); j++)
        {
            output += "(" + to_string(inN[j].first) + ",<" + labelSetToLetter(inN[j].second)
                + "==" + labelSetToString(inN[j].second) + ">)\n";
        }

        output += "} \n";
    }

    //cout << "toString2 output=" << output << endl;

    return output;
};

long DGraph::getCountPerLabel(LabelID l)
{
    if( l < 0 || l >= L )
    {
        cerr << "getCountPerLabel l out of bounds" << endl;
        return -1;
    }

    return countPerLabel[l];
};

int DGraph::getNumberOfVertices()
{
    return N;
};

int DGraph::getNumberOfLabels()
{
    return L;
};

int DGraph::getNumberOfEdges()
{
    return M;
};

void DGraph::addNode()
{
    N += 1;
    outE.push_back( SmallEdgeSet() );
    inE.push_back( SmallEdgeSet() );
};

void DGraph::removeNode(graphns::VertexID w)
{
    if( w < 0 || w > N )
    {
        cerr << " DGraph::removeNode out of bounds w" << w << endl;
        return;
    }

    // remove w and decrease id's of all successors
    for(int i = 0; i < N; i++)
    {
        if( i == w )
            continue;

        for(int j = 0; j < outE[i].size(); j++)
        {
            VertexID w1 = outE[i][j].first;

            if( w1 > w )
            {
                w1--;
            }
            else if( w1 == w )
            {
                outE[i].erase( outE[i].begin() + j );
                j--;
            }
        }

        for(int j = 0; j < inE[i].size(); j++)
        {
            VertexID w1 = inE[i][j].first;

            if( w1 > w )
            {
                w1--;
            }
            else if( w1 == w )
            {
                inE[i].erase( inE[i].begin() + j );
                j--;
            }
        }
    }

    // delete edges of w
    M -= outE[w].size() + inE[w].size();

    outE.erase( outE.begin() + w );
    inE.erase( inE.begin() + w );
    N -= 1;
};

void DGraph::addEdge(VertexID v, VertexID w, LabelID newLabel)
{
    if( v < 0 || v > N || w < 0 || w > N )
    {
        cerr << " DGraph::addEdge v or w out of bounds w=" << w  << ",v=" << v << endl;
        return;
    }

    int pos1 = 0;
    int pos2 = 0;

    bool b1 = findInsertablePosition(w, outE[v], pos1);
    bool b2 = findInsertablePosition(v, inE[w], pos2);

    if( (b1 == true || b2 == true) && allowMultipleEdges == false )
    {
        cerr << " DGraph::addEdge vw edge already exists w=" << w  << ",v=" << v << endl;
        return;
    }

    LabelSet ls = labelIDToLabelSet( newLabel );
    outE[v].insert( outE[v].begin() + pos1, make_pair(w, ls));
    inE[w].insert( inE[w].begin() + pos2, make_pair(v, ls));
    M += 1;
};

void DGraph::addMultiEdge(graphns::VertexID v, graphns::VertexID w, graphns::LabelSet newLabelSet)
{
    if( v < 0 || v > N || w < 0 || w > N )
    {
        //cerr << " DGraph::addMultiEdge v or w out of bounds w=" << w  << ",v=" << v << endl;
        return;
    }

    if( hasMultiEdge(v,w, newLabelSet ) == true )
    {
        //cerr << " DGraph::addMultiEdge newLabelSete already exists newLabelSet=" << newLabelSet << endl;
        return;
    }

    int pos1 = 0;
    int pos2 = 0;

    bool b1 = findInsertablePosition(w, outE[v], pos1);
    bool b2 = findInsertablePosition(v, inE[w], pos2);

    if( (b1 == true || b2 == true) && allowMultipleEdges == false )
    {
        //cerr << " DGraph::addMultiEdge vw edge already exists w=" << w  << ",v=" << v << endl;
        return;
    }

    outE[v].insert( outE[v].begin() + pos1, make_pair(w, newLabelSet));
    inE[w].insert( inE[w].begin() + pos2, make_pair(v, newLabelSet));
    M += 1;
};

void DGraph::removeEdge(graphns::VertexID v, graphns::VertexID w)
{
    if( v < 0 || v > N || w < 0 || w > N )
    {
        cout << " DGraph::removeEdge v or w out of bounds w=" << w  << ",v=" << v << endl;
        return;
    }

    int pos1 = 0;
    int pos2 = 0;

    bool b1 = findInsertablePosition(w, outE[v], pos1);
    bool b2 = findInsertablePosition(v, inE[w], pos2);

    if( b1 == false || b2 == false )
    {
        cout << " DGraph::removeEdge vw edge does not exist w=" << w  << ",v=" << v << ",b1=" << b1 << ",b2=" << b2 << endl;
        cout << endl;
        return;
    }

    outE[v].erase( outE[v].begin() + pos1 );
    inE[w].erase( inE[w].begin() + pos2 );
    M -= 1;
    //cout << "removeEdge w=" << w  << ",v=" << v << ",b1=" << b1 << ",b2=" << b2 << ",pos1=" << pos1 << ",pos2=" << pos2 << endl;
};

void DGraph::changeLabel(graphns::VertexID v, graphns::VertexID w, LabelID newLabel)
{
    if( v < 0 || v > N || w < 0 || w > N )
    {
        cerr << " DGraph::changeLabel v or w out of bounds w=" << w  << ",v=" << v << endl;
        return;
    }

    int pos1 = 0;
    int pos2 = 0;

    bool b1 = findInsertablePosition(w, outE[v], pos1);
    bool b2 = findInsertablePosition(v, inE[w], pos2);

    if( b1 == false || b2 == false )
    {
        cerr << " DGraph::changeLabel vw edge does not exist" << endl;
        return;
    }

    LabelSet ls = labelIDToLabelSet( newLabel );
    outE[v][pos1].second = ls;
    inE[w][pos2].second = ls;
};

LabelID DGraph::getLabelID(graphns::VertexID v , graphns::VertexID w)
{
    if( hasEdge(v,w) == false )
    {
        cerr << " DGraph::getLabelID v or w out of bounds w=" << w  << ",v=" << v << endl;
        return 0;
    }

    int pos1 = 0;
    bool b1 = findInsertablePosition(w, outE[v], pos1);

    return labelSetToLabelID( outE[v][pos1].second );
}

bool DGraph::hasMultiEdge(graphns::VertexID v , graphns::VertexID w, graphns::LabelSet ls)
{
    // verifies whether there already exists an edge (v,w,ls') s.t. ls' subset of ls
    if( hasEdge(v,w) == false )
    {
        return false;
    }

    int pos1 = 0;
    bool b1 = findInsertablePosition(w, outE[v], pos1);

    if( pos1 > 0 )
    {
        while(outE[v][pos1-1].first == w)
        {
            pos1 -= 1;
            if( pos1 <= 0 )
            {
                break;
            }
        }
    }

    while(outE[v][pos1].first == w)
    {
        LabelSet lsPrime = outE[v][pos1].second;

        if( isLabelSubset(lsPrime, ls) == true )
        {
            return true;
        }

        pos1 += 1;
        if( pos1 >= outE[v].size() )
        {
            break;
        }
    }

    return false;
}

double DGraph::computeAverageDiameter()
{
    // a simple and dumb way to compute the diameter
    int totalDiameter = 0;
    for(int i = 0; i < N; i++)
    {
        //cout << "computeDiameter: i=" << i << endl;

        dynamic_bitset<> marked = dynamic_bitset<>(N);
        queue< pair<VertexID,int> > q;
        q.push( make_pair(i,0) );

        while( q.empty() == false )
        {
            auto p = q.front();
            q.pop();

            if( marked[p.first] == 1 )
            {
                continue;
            }
            marked[p.first] = 1;
            totalDiameter++;

            SmallEdgeSet ses;
            getOutNeighbours(p.first, ses);
            for(int j = 0; j < ses.size(); j++)
            {
                q.push( make_pair(ses[j].first, p.second + 1) );
            }

        }
    }

    return ( (double) totalDiameter / (double) N );
}

int DGraph::computerNumberOfTriangles()
{
    int NoOfTriangles = 0;
    for(int i = 0; i < N; i++)
    {
        //cout << "computerNumberOfTriangles: i=" << i << endl;

        dynamic_bitset<> marked = dynamic_bitset<>(N);
        queue< pair<VertexID,int> > q;
        q.push( make_pair(i,0) );

        while( q.empty() == false )
        {
            auto p = q.front();
            q.pop();

            if( p.first == i && p.second == 3 )
            {
                //cout << "NoOfTriangles=" << NoOfTriangles << endl;
                NoOfTriangles++;
            }

            if( marked[p.first] == 1 || p.second >= 3 )
            {
                continue;
            }
            marked[p.first] = 1;

            SmallEdgeSet ses;
            getOutNeighbours(p.first, ses);
            for(int j = 0; j < ses.size(); j++)
            {
                q.push( make_pair(ses[j].first, p.second + 1) );
            }
        }
    }
    return NoOfTriangles / 3;
}

bool DGraph::hasEdge(VertexID v, VertexID w)
{
    SmallEdgeSet ses;
    getOutNeighbours(v, ses);

    for(int i = 0; i < ses.size();i++)
    {
        if( ses[i].first == w )
        {
            return true;
        }
    }

    return false;
};

double DGraph::computeClusterCoefficient()
{
    double clusterCoefficient = 0.0;
    for(int i = 0; i < N; i++)
    {
        // compute neighbourhood of i
        SmallEdgeSet outSes, inSes;
        getOutNeighbours(i, outSes);
        getInNeighbours(i, inSes);

        int Ki = outSes.size() + inSes.size();
        double localClusterCoefficient = 0.0;

        for(int j = 0; j < inSes.size(); j++)
        {
            VertexID v = inSes[j].first;
            for(int k = 0; k < outSes.size(); k++)
            {
                // we need a (w,v) edge for a triangle
                VertexID w = outSes[k].first;

                if( hasEdge(w,v) == true )
                {
                    localClusterCoefficient += 1.0;
                }
            }
        }

        localClusterCoefficient /= (Ki*(Ki-1)*1.0);
        if( isnormal(localClusterCoefficient) == 0 )
        {
            clusterCoefficient += localClusterCoefficient;
        }
    }

    return max(0.0, clusterCoefficient / N);
}

void DGraph::tarjan(vector< vector<VertexID> >& SCCs)
{
    int index = 0;
    stack<VertexID> q;
    vector< int > indexPerNode = vector< int >(N,-1);
    vector< int > lowlinkPerNode = vector< int >(N,-1);
    vector< bool > onStack = vector< bool >(N,false);

    for(int i = 0; i < N; i++)
    {
        if( indexPerNode[i] == -1 )
        {
            tarjanStrongConnect(i, index, q, indexPerNode, lowlinkPerNode, onStack, SCCs);
        }
    }
}

void DGraph::tarjanStrongConnect(int v, int& index, stack<VertexID>& q, vector< int >& indexPerNode,
    vector< int >& lowlinkPerNode, vector< bool >& onStack, vector< vector<VertexID> >& SCCs)
{
    //cout << "v=" << v << ",index=" << index << endl;

    indexPerNode[v] = index;
    lowlinkPerNode[v] = index;
    index += 1;

    q.push( v );
    onStack[v] = true;

    SmallEdgeSet ses;
    getOutNeighbours(v,ses);
    for(int i = 0; i < ses.size(); i++)
    {
        VertexID w = ses[i].first;

        //cout << "v=" << v << ",w=" << w << ",indexPerNode[w]=" << indexPerNode[w] << ",onStack[w]=" << onStack[w] << ",q.size()=" << q.size() << endl;

        if( indexPerNode[w] == -1 )
        {
            tarjanStrongConnect(w, index, q, indexPerNode, lowlinkPerNode, onStack, SCCs);
            lowlinkPerNode[v] = min(lowlinkPerNode[v], lowlinkPerNode[w]);
        }
        else
        {
            if( onStack[w] == true )
            {
                lowlinkPerNode[v] = min(lowlinkPerNode[v], indexPerNode[w]);
            }
        }
    }

    if( lowlinkPerNode[v] == indexPerNode[v] )
    {
        vector< VertexID > SCC;
        VertexID w = 0;

        do
        {
            w = q.top();
            q.pop();
            //cout << "SCC w = " << w << endl;

            onStack[w] = false;
            SCC.push_back(w);
        }
        while( w != v && q.empty() == false );

        SCCs.push_back(SCC);
    }
};

std::string DGraph::getStats()
{
    string s = "";
    s += "N, " + to_string(N) + "\n";
    s += "M, " + to_string(M) + "\n";
    s += "L, " + to_string(L) + "\n";

    int noOfTriangles = computerNumberOfTriangles();
    double ratio = computeClusterCoefficient();
    s += "Number of triangles, " + to_string(noOfTriangles) + "\n";
    s += "Average cluster coefficient, " + to_string(ratio) + "\n";

    cout << "getStats: noOfTriangles=" << noOfTriangles << ",ratio=" << ratio << endl;

    vector< vector<VertexID> > SCCs;
    tarjan(SCCs);
    int maxSCC = 0;
    for(int i = 0; i < SCCs.size(); i++)
    {
        if( SCCs[i].size() > maxSCC )
        {
            maxSCC = SCCs[i].size();
        }
    }

    ratio = ((double) maxSCC) / ((double) N);
    cout << "getStats: SCCs.size()=" << SCCs.size() << ",ratio=" << ratio << endl;

    int diameter = computeDiameter();

    s += "Largest SCC #nodes/N, " + to_string(ratio) + "\n";
    s += "Number of SCCs, " + to_string(SCCs.size()) + "\n";
    s += "Diameter, " + to_string(diameter) + "\n";

    cout << "getStats: diameter=" << diameter << endl;

    s += "\n";
    s += "vertex id, out degree, in degree \n";

    vector< int > outFreqs;
    vector< int > inFreqs;
    vector< int > labelFreqs = vector<int>(L, 0);

    // in- and outdegree per vertex
    // and frequency of in- and outdegree
    // and label frequencies
    for(int i = 0; i < N; i++)
    {
        SmallEdgeSet outSes;
        getOutNeighbours(i, outSes);
        SmallEdgeSet inSes;
        getInNeighbours(i, inSes);
        int outDegree = outSes.size();
        int inDegree = inSes.size();

        //cout << to_string(i) + "," + to_string(outDegree) + "," + to_string(inDegree) + "\n";
        while( outFreqs.size() < outDegree+1 )
        {
            outFreqs.push_back(0);
        }
        outFreqs[ outDegree ]++;

        while( inFreqs.size() < inDegree+1 )
        {
            inFreqs.push_back(0);
        }
        inFreqs[ inDegree ]++;

        for(int j = 0; j < outSes.size(); j++)
        {
            LabelID lID = labelSetToLabelID(outSes[j].second);
            labelFreqs[ lID ]++;
        }

        s += to_string(i) + "," + to_string(outDegree) + "," + to_string(inDegree) + "\n";
    }

    // add frequencies to table
    s += "\n\noutdegree, frequency \n";
    for(int i = 0; i < outFreqs.size(); i++)
    {
        s += to_string(i) + "," + to_string(outFreqs[i]) + "\n";
    }

    s += "\n\nindegree, frequency \n";
    for(int i = 0; i < inFreqs.size(); i++)
    {
        s += to_string(i) + "," + to_string(inFreqs[i]) + "\n";
    }

    s += "\n";
    for(int i = 0; i < L; i++)
    {
        s += to_string(i);
        if( i < L-1 )
        {
            s += ",";
        }
    }
    s += "\n";

    for(int i = 0; i < L; i++)
    {
        s += to_string( labelFreqs[i] );
        if( i < L-1 )
        {
            s += ",";
        }
    }
    s += "\n";

    cout << "getStats: stats generated" << endl;

    return s;
};

int DGraph::computeDiameter()
{
    // Gives an approximate diameter by taking N/SAMPLE_SIZE nodes
    int SAMPLE_SIZE = min(200, N/10);
    vector<int> distances = vector<int>(N);
    int diameter = 0;

    for(int i = 0; i < N; i++)
    {
        if( i % SAMPLE_SIZE != 0 && N >= 100 )
            continue;

        int newDiameter = findLongestShortestPath(i);
        if( newDiameter > diameter )
        {
            //cout << "i=" << i << " ,diameter=" << diameter << endl;
            diameter = newDiameter;
        }


    }

    return diameter + 1;
};
