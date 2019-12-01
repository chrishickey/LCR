#include "Zou.h"

#include <queue>
#include <iostream>

using namespace zhouns;
using namespace indexns;


Zou::Zou(Graph* mg)
{
    this->graph = mg;
    this->isBlockedMode = false;
    this->indexType = IndexType::ZouType;
    buildIndex();

    cout << "Zou-index size(byte)=" << getIndexSizeInBytes() << ", time(s)=" << getIndexConstructionTimeInSec() << endl;
};

Zou::~Zou()
{

};

unsigned long Zou::getIndexSizeInBytes()
{
    return getIndexSizeInBytesM();
};

void Zou::queryAll(VertexID source, LabelSet ls, dynamic_bitset<>& canReach)
{

};

void Zou::buildIndex()
{
    constStartTime = getCurrentTimeInMilliSec();

    int N = graph->getNumberOfVertices();
    int L = graph->getNumberOfLabels();

    this->didComplete = false;

    // construct tIn or cIn
    initializeIndex();

    // first we divide graph G into a set of SCC's
    // we use a normal SCC for this, i.e. ignoring the edge labels
    SCCs = vector< vector < VertexID > >();
    vToSCCID = vector< int >(2*N, -1);
    this->graph->tarjan(SCCs);

    cout << "Step 1 (SCC-construction): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;
    cout << "Number of SCCs=" << SCCs.size() << endl;

    // map all vertices to a specific id
    for(int i = 0; i < SCCs.size(); i++)
    {
        //cout << "SCCs[i].size()=" << SCCs[i].size() << endl;
        sort( SCCs[i].begin(), SCCs[i].end() );

        for(int j = 0; j < SCCs[i].size(); j++)
        {
            //cout << "SCCs[i][j]=" << SCCs[i][j] << ", i=" << i << endl;
            vToSCCID[ SCCs[i][j] ] = i;
            vToSCCID[ N + SCCs[i][j] ] = i;
        }
    }

    // create a subgraph for each SCC containing only the right edges
    subGraphs = vector< Graph* >();
    vector< int > countPerSCC = vector< int >(SCCs.size(), 0);
    vToSubGraphID = vector< int >(2*N, -1);
    EdgeSet DAGedges = EdgeSet();

    for(int i = 0; i < SCCs.size(); i++)
    {
        EdgeSet* es = new EdgeSet();
        DGraph* ng = new DGraph(es,2*SCCs[i].size(),L);
        subGraphs.push_back( ng );
    }

    for(int i = 0; i < 2*N; i++)
    {
        int iID = vToSCCID[ i ];
        vToSubGraphID[ i ] = countPerSCC[ iID ];

        //cout << "i=" << i << " ,iID=" << iID << " ,countPerSCC[ iID ]=" << countPerSCC[ iID ] << endl;

        countPerSCC[ iID ] += 1;
    }

    for(int i = 0; i < N; i++)
    {
        int iID = vToSCCID[ i ];
        int iSID = vToSubGraphID[ i ];

        //cout << "i=" << i << " ,vToSCCID[i]=" << iID << ", vToSubGraphID[i]=" << iSID << endl;

        SmallEdgeSet ses;
        graph->getOutNeighbours(i, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            int jID = vToSCCID[ ses[j].first ];

            if( iID == jID )
            {
                int jSID = vToSubGraphID[ ses[j].first ];
                //cout << "- ses[j].first =" << ses[j].first << ", jID=" << iID << ", jSID=" << jSID << endl;

                subGraphs[iID]->addEdge( iSID, jSID, labelSetToLabelID(ses[j].second) );
            }
            else
            {
                DAGedges.push_back( make_pair(i, make_pair( ses[j].first, ses[j].second )) );
            }
        }
    }

    cout << "Step 2 (ID-maps and DAGedges found): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // build an index for each SCC first
    for(int i = 0; i < subGraphs.size(); i++)
    {
        buildIndex(i, subGraphs[i]);
    }

    cout << "Step 3 (SCC indices built): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // create DAG D
    EdgeSet* es = new EdgeSet();
    this->D = new DGraph(es,2*N,L,true); // allows multiple edges between any (v,w)
    this->inPortals = vector< vector < VertexID > >(2*SCCs.size());
    this->outPortals = vector< vector < VertexID > >(2*SCCs.size());

    // next find the portals and add these
    for(int i = 0; i < DAGedges.size(); i++)
    {
        VertexID v = DAGedges[i].first;
        VertexID vN = DAGedges[i].first + N;
        VertexID w = DAGedges[i].second.first;
        VertexID wN = DAGedges[i].second.first + N;
        LabelSet ls = DAGedges[i].second.second;

        D->addMultiEdge(vN,w,ls);

        D->addMultiEdge(v, vN, 0);
        D->addMultiEdge(w, wN, 0);

        //cout << "- v=" << v << ", w=" << w << ", ls=" << ls << endl;

        auto it = lower_bound(this->inPortals[ vToSCCID[w] ].begin(), this->inPortals[ vToSCCID[w] ].end(), w);
        if( it == this->inPortals[ vToSCCID[w] ].end() )
        {
            this->inPortals[ vToSCCID[w] ].push_back(w);
        }

        it = lower_bound(this->outPortals[ vToSCCID[vN] ].begin(), this->outPortals[ vToSCCID[vN] ].end(), vN);
        if( it == this->outPortals[ vToSCCID[vN] ].end() )
        {
            this->outPortals[ vToSCCID[vN] ].push_back(vN);
        }
    }

    for(int i = 0; i < SCCs.size(); i++)
    {
        sort( this->inPortals[i].begin(), this->inPortals[i].end() );
        sort( this->outPortals[i].begin(), this->outPortals[i].end() );
    }

    cout << "Step 4 (portals found): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    /*for(int i = 0; i < SCCs.size(); i++)
    {
        cout << "SCC-" << i << ": {";
        for(int j = 0; j < this->inPortals[i].size(); j++)
        {
            cout << this->inPortals[i][j] << " ";
        }
        cout << "}\n{";
        for(int j = 0; j < this->outPortals[i].size(); j++)
        {
            cout << this->outPortals[i][j] << " ";
        }
        cout << "}" << endl;
    }*/


    // next add the bipartite edges
    for(int i = 0; i < SCCs.size(); i++)
    {
        //cout << "DAGedges bipartite i=" << i << endl;

        for(int j = 0; j < this->outPortals[i].size(); j++)
        {
            for(int k = 0; k < this->inPortals[i].size(); k++)
            {
                VertexID v = this->inPortals[i][k];
                VertexID w = this->outPortals[i][j];
                VertexID wN = w % N;

                //cout << "DAGedges bipartite v=" << v << " ,wN=" << wN << endl;

                LabelSets lss;
                getLabelSetsPerPair(v, wN, lss);
                for(int l = 0; l < lss.size(); l++)
                {
                    D->addMultiEdge(v, w, lss[l]);
                    //cout << "- DAGedges bipartite v =" << v << ", w=" << w << " ,ls=" << lss[l] << endl;
                }

            }
        }
    }

    cout << "Step 5 (adding bipartite edges): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // compute reverse topological order
    vector< VertexID > ordering;
    D->topologicalSort(ordering);

    /*cout << "---" << endl;
    cout << D->toString();
    cout << "---" << endl;*/

    cout << "Step 6 (computing topological order): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // topological ordering
    for(int i = ordering.size()-1; i >= 0; i--)
    {
        VertexID v = ordering[i];
        VertexID vN = v % N;
        //cout << "step 7: v=" << v << " ,vN=" << vN << endl;

        SmallEdgeSet ses;
        D->getOutNeighbours(v, ses);
        for(int j = 0; j < ses.size(); j++)
        {
            VertexID w = ses[j].first;
            VertexID wN = w % N;
            LabelSet ls = ses[j].second;

            if( vN == wN || v == w )
            {
                continue;
            }

            tryInsert(vN, wN, ls);

            /* Copy all of w's entries into v */
            LabelSet ls1 = ls;
            for(int m = 0; m < tIn[wN].size(); m++)
            {
                for(int o = 0; o < tIn[wN][m].second.size(); o++)
                {
                    if( vN == tIn[wN][m].first )
                    {
                        continue;
                    }

                    LabelSet ls2 = tIn[wN][m].second[o];
                    LabelSet ls3 = joinLabelSets(ls1, ls2);

                    //cout << "step 7: vN=" << vN << ",wN=" << wN << " ,t=" << tIn[wN][m].first << ",ls1=" << ls1 << ",ls2=" << ls2 << ",ls3=" << ls3 << endl;
                    tryInsert(vN, tIn[wN][m].first , ls3);
                }
            }

        }
    }

    /* If the first 6 steps already take at least an hour, the rest won't complete within 6 hours */
    if(  (getCurrentTimeInMilliSec() - constStartTime) >= TIMEOUT/6 )
    {
        return;
    }

    cout << "Step 7 (topological ordering, only portals): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // outportal
    int progress = 0;
    for(int i = 0; i < SCCs.size(); i++)
    {
        for(int j = 0; j < this->outPortals[i].size(); j++)
        {
            progress++;

            VertexID v = this->outPortals[i][j];
            VertexID vN = v % N;

            for(int k = 0; k < N; k++)
            {
                VertexID w = k;
                VertexID wN = w % N;

                LabelSets lss;
                getLabelSetsPerPair(vN, wN, lss);

                for(int l = 0; l < lss.size(); l++)
                {
                    LabelSet ls1 = lss[l];
                    for(int m = 0; m < tIn[wN].size(); m++)
                    {
                        for(int o = 0; o < tIn[wN][m].second.size(); o++)
                        {
                            VertexID t = tIn[wN][m].first; // the target
                            if( vN == t || wN == t )
                            {
                                continue;
                            }

                            LabelSet ls2 = tIn[wN][m].second[o];
                            LabelSet ls3 = joinLabelSets(ls1, ls2);
                            //cout << "step 8: vN=" << vN << " ,wN=" << wN << " ,t=" << t << ",ls1=" << ls1 << ",ls2=" << ls2 << ",ls3=" << ls3 << endl;

                            tryInsert(vN, t , ls3);
                        }
                    }
                }
            }
        }
    }

    /* If the first 6 steps already take at least half an hour, the rest won't complete within 6 hours */
    if( (getCurrentTimeInMilliSec() - constStartTime) >= TIMEOUT/12 )
    {
        totalConstTime = constEndTime - constStartTime;
        cout << "Zou did not complete, time=" << totalConstTime << endl;
        return;
    }

    cout << "Step 8 (outportal to inner vertices): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    // out-portals
    for(int i = 0; i < SCCs.size(); i++)
    {
        for(int j = 0; j < SCCs[i].size(); j++)
        {
            VertexID v = SCCs[i][j];
            VertexID vN = v % N;

            for(int k = 0; k < this->outPortals[i].size(); k++)
            {
                VertexID w = this->outPortals[i][k];
                VertexID wN = w % N;

                LabelSets lss;
                getLabelSetsPerPair(vN, wN, lss);

                for(int l = 0; l < lss.size(); l++)
                {
                    LabelSet ls1 = lss[l];
                    for(int m = 0; m < tIn[wN].size(); m++)
                    {
                        for(int o = 0; o < tIn[wN][m].second.size(); o++)
                        {
                            VertexID t = tIn[wN][m].first; // the target
                            if( vN == t || wN == t )
                            {
                                continue;
                            }

                            LabelSet ls2 = tIn[wN][m].second[o];
                            LabelSet ls3 = joinLabelSets(ls1, ls2);
                            //cout << "step 9: vN=" << vN << " ,wN=" << wN << " ,t=" << t << ",ls1=" << ls1 << ",ls2=" << ls2 << ",ls3=" << ls3 << endl;

                            tryInsert(vN, t , ls3);
                        }
                    }
                }
            }
        }
    }

    /* If the first 7 steps already take at least two hours, the rest won't complete within 6 hours */
    if( (getCurrentTimeInMilliSec() - constStartTime) >= TIMEOUT/3 )
    {
        totalConstTime = constEndTime - constStartTime;
        cout << "Zou did not complete, time=" << totalConstTime << endl;
        return;
    }

    cout << "Step 9 (innervertex to outportal): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;

    for(int i = 0; i < SCCs.size(); i++)
    {
        // v must not be in SCC-i
        for(int v = 0; v < N; v++)
        {
            // u must be an inner-vertex in SCC-i
            for(int o = 0; o < SCCs[i].size(); o++)
            {
                VertexID u = SCCs[i][o] % N;

                // inP is an inner portal in SCC-i
                for(int j = 0; j < this->inPortals[i].size(); j++)
                {
                    VertexID inP = this->inPortals[i][j];
                    VertexID inPN = inP % N;

                    LabelSets lss1,lss2;
                    getLabelSetsPerPair(v, inPN, lss1);
                    getLabelSetsPerPair(inPN, u, lss2);

                    for(int m = 0; m < lss1.size(); m++)
                    {
                        for(int p = 0; p < lss2.size(); p++)
                        {
                            LabelSet ls3 = joinLabelSets(lss1[m],lss2[p]);
                            //cout << "step 10: v=" << v << " ,u=" << u << ",lss1[m]=" << lss1[m] << ",lss2[p]=" << lss2[p] << endl;
                            tryInsert(v, u, ls3);
                        }
                    }
                }
            }
        }
    }

    cout << "Step 10 (last step): " << print_digits( getCurrentTimeInMilliSec()-constStartTime, 2 ) << endl;


    //cout << toString() << endl;
    cout << "D->M=" << D->getNumberOfEdges() << endl;

    this->didComplete = true;
    constEndTime = getCurrentTimeInMilliSec();
    totalConstTime = constEndTime - constStartTime;

};

void Zou::buildIndex(int SCCID, Graph* sG)
{
    int N = sG->getNumberOfVertices();
    int q = max(N/10, 10);
    //cout << "buildIndex sG->N=" << N << " ,SCCID=" << SCCID << endl;

    for(int i = 0; i < N; i++)
    {
        /*if( ((i%q == 0) || i == N-1) && i > 0 )
            cout << "buildIndex sG->i=" << i << endl;*/
        eDijkstra(SCCID, i, sG);
    }
};

bool Zou::tryInsert(VertexID w, VertexID v, LabelSet ls)
{
    if( w == v )
    {
        return false;
    }

    bool b2 = tryInsertLabelSetToIndex(ls, w, v);
    //cout << "tryInsert: w=" << w << ",v=" << v << ",ls=" << labelSetToString(ls) << ",b2=" << b2 << endl;

    return b2;
}

void Zou::eDijkstra(int SCCID, VertexID v, Graph* sG)
{
    priority_queue< NeighTriplet, vector<NeighTriplet>, CompareTriplets > H;
    NeighTriplets nts, RS;

    // initialize first neighbour triplet
    NeighTriplet nt;
    nt.ls = 0;
    nt.path = vector< VertexID >();
    nt.path.push_back(v);
    H.push(nt);

    //cout << "eDijkstra v=" << v << endl;

    // loop
    while( H.empty() == false )
    {
        NeighTriplet nt2 = H.top();
        H.pop();

        int pos = 0;
        if( isNTCovered(nt2, RS, pos) == false )
        {
            //cout << "- eDijkstra nt2=" << NTToString(nt2) << ",H.size()=" << H.size() << endl;
            insertNT(nt2, RS, pos);
            produceNeighTriplets(nt2, sG, nts);

            for(int j = 0; j < nts.size(); j++)
            {
                H.push(nts[j]);
            }
        }
    }

    // use RS to build index
    for(int i = 0; i < RS.size(); i++)
    {
        NeighTriplet nt = RS[i];
        VertexID w = nt.path[nt.path.size()-1];
        LabelSet ls = nt.ls;

        if( w == v )
        {
            continue;
        }

        // translate local id's of v and w into global id's
        VertexID vG = SCCs[SCCID][v];
        VertexID wG = SCCs[SCCID][w];

        //cout << "eDijkstra: vG=" << vG << " ,wG=" << wG << endl;

        //tryInsert(ls,vG,wG);
        tryInsert(vG,wG,ls);
    }
};

void Zou::produceNeighTriplets(NeighTriplet& nt, Graph* sG, NeighTriplets& nts)
{
    // this method produces all neighbour triplets from a given neighbour triplet nt
    // i.e. it looks at all neighbour vertices of the last vertex in nt.path
    // and tries to create a new neighbour triplet nt2
    // nt2 should have a simple path
    nts.clear();
    SmallEdgeSet ses;
    sG->getOutNeighbours(nt.path[nt.path.size()-1], ses);
    //cout << "produceNeighTriplets: nt=" << NTToString(nt) << ",ses.size()=" << ses.size() << endl;

    for(int i = 0; i < ses.size(); i++)
    {
        // create a new path with updated distance and label set
        NeighTriplet nt2;
        nt2.ls = joinLabelSets( nt.ls, ses[i].second );
        nt2.path = nt.path;
        nt2.path.push_back( ses[i].first );

        // the path must be simple
        bool isSimple = find(nt.path.begin(), nt.path.end(), ses[i].first) == nt.path.end();
        //cout << "- produceNeighTriplets: nt2=" << NTToString(nt2) << " ,isSimple=" << isSimple << endl;
        if( isSimple == false )
        {
            continue;
        }

        nts.push_back( nt2 );
    }
};

bool Zou::query(VertexID source, VertexID target, LabelSet ls)
{
    //cout << "Zou::query source=" << to_string(source) << ",target=" << to_string(target) << ",ls=" << labelSetToString(ls) << endl;
    queryStart = getCurrentTimeInMilliSec();
    bool b = queryShell(source, target, ls);
    queryEndTime = getCurrentTimeInMilliSec();
    //cout << "Zou::query answer =" << b << endl;
    return b;
}

bool Zou::queryShell(VertexID source, VertexID target, LabelSet ls)
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
