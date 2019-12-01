/* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).
 * Pruned-landmark-LCR-labeling-based indexing of edge-labeled directed graphs.
 * Using bitsets for label sets.
 * October 2014 */

#include <climits>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <queue>
#include <sys/time.h>
#include "pruned_lcr_labeling_bits.h"

using namespace plcrlb;


/* comparison function, for sorting edge set **************************/

bool edgesComp(Edge a, Edge b) {
  if (a.from < b.from)
    return true;
  else if (a.from > b.from)
    return false;
  else if (a.to < b.to)
    return true;
  else if (a.to > b.to)
    return false;
  else
    return a.label < b.label;
}

void load_edge_file(string file_name, plcrlb::EdgeSet &edgeSet) {
    //load graph *.edge file
    edgeSet.clear();
    string line;
    plcrlb::VertexID f, t;
    plcrlb::LabelID l;

    ifstream edge_file (file_name);
    if (edge_file.is_open()) {
        while ( getline (edge_file,line) ) {
            istringstream iss(line);
            string from, label, to;
            iss >> from >> to >> label;
            istringstream (from) >> f;
            istringstream (to) >> t;
            istringstream (label) >> l;
            //cout << "Edge_file: f=" << f << ", t=" << t << ", l=" << l << endl;
            edgeSet.push_back( (plcrlb::Edge) {f, t, l} );
        }
        edge_file.close();
    } else {
        cerr << "Unable to open file " << file_name;
    }
}

/*Public **********************************************************************/

PrunedLcrLabelingB::PrunedLcrLabelingB(EdgeSet & edges) {
  createIndex(edges);
  this->E = edges.size();
}

PrunedLcrLabelingB::~PrunedLcrLabelingB() {
}

size_t PrunedLcrLabelingB::vertexCount() {
  return V;
}

size_t PrunedLcrLabelingB::edgeCount() {
  return this->E;
}

unsigned long long PrunedLcrLabelingB::indexSize() {
  unsigned long long size = PLLSETS_SIZE * 2;
  PllSet::iterator current;

  if (this->outSets.empty() || this->inSets.empty())
    return size;

  for (VertexID v: range(this->V)) {
    if (!this->outSets[v].empty()) {
      size += PLLSET_SIZE;
      size += BUCKET_OVERHEAD_SIZE * this->outSets[v].size();
    }

    if (!this->inSets[v].empty()) {
      size += PLLSET_SIZE;
      size += BUCKET_OVERHEAD_SIZE * this->inSets[v].size();
    }
  }
  return size;
}

double PrunedLcrLabelingB::indexingTime() {
  return this->indexingTimeSeconds;
}

void PrunedLcrLabelingB::dumpGraph() {
  cout << "G " << endl;

  vector<VertexID> rConvertTable(this->V);
  for (VertexID v: range(this->V)) {
    rConvertTable[convertTable[v]] = v;
  }

  for (VertexID v: range(this->V)) {
    for (pair<VertexID, LabelSet>& edge: G[v]) {
      cout << "source " << rConvertTable[v] << " target " << rConvertTable[edge.first] << endl << "labels:";
      for (size_t s: range(PLL_BITS)) {
        if (edge.second & ((VertexID)1 << s))
          cout << " " << s;
      }
      cout << endl;
    }
  }

  cout << "\nrG " << endl;
  for (VertexID v: range(this->V)) {
    for (pair<VertexID, LabelSet>& edge: rG[v]) {
      cout << "source " << rConvertTable[v] << " target " << rConvertTable[edge.first] << endl << "labels:";
      for (size_t s: range(PLL_BITS)) {
        if (edge.second & ((VertexID)1 << s))
          cout << " " << s;
      }
      cout << endl;
    }
  }
}

void PrunedLcrLabelingB::dumpPllSets() {
  cout << "In sets" << endl;
  for (VertexID v: range(this->V)) {
    if (!this->inSets[v].empty()){
      cout << "***vertex " << v << endl;
      for (pair<VertexID, LabelSet>& pIt: this->inSets[v]) {
        cout << "target " << pIt.first << ": ";
        for (size_t s: range(PLL_BITS)) {
          if (pIt.second & ((VertexID)1 << s))
            cout << s << " ";
        }
        cout << endl;
      }
    }
  }

  cout << endl << "Out sets" << endl;
  for (VertexID v: range(this->V)) {
    if (!this->outSets[v].empty()){
      cout << "***vertex " << v << endl;
      for (pair<VertexID, LabelSet>& pIt: this->outSets[v]) {
        cout << "target " << pIt.first << ": ";
        for (size_t s: range(PLL_BITS)) {
          if (pIt.second & ((VertexID)1 << s))
            cout << s << " ";
        }
        cout << endl;
      }
    }
  }
}

bool PrunedLcrLabelingB::query(VertexID sourceID, VertexID targetID,  LabelSet & labels) {
  assert(sourceID < convertTable.size() && targetID < convertTable.size());
  return this->internalQuery(convertTable[sourceID], convertTable[targetID], labels);
}

bool PrunedLcrLabelingB::internalQuery(VertexID sourceID, VertexID targetID,  LabelSet & labels) {
  if (sourceID == targetID)
    return true;
  if (labels == 0)
    return false;
  if (this->outSets.empty() || this->inSets.empty())
    return false;
  if (this->outSets[sourceID].empty() || this->inSets[targetID].empty())
    return false;

  PllSet::iterator sourceOut = this->outSets[sourceID].begin();
  PllSet::iterator targetIn = this->inSets[targetID].begin();
  bool sFlag = false;
  bool tFlag = false;
  VertexID currentLandmark;

  while (sourceOut != this->outSets[sourceID].end() && targetIn != this->inSets[targetID].end()) {
    // advance to next shared landmark
    while (sourceOut != this->outSets[sourceID].end() && sourceOut->first < targetIn->first) {
      sourceOut++;
    }
    if (sourceOut == this->outSets[sourceID].end()) break;

    while (targetIn != this->inSets[targetID].end() && sourceOut->first > targetIn->first) {
      targetIn++;
    }
    if (targetIn == this->inSets[targetID].end()) break;

    if (sourceOut != this->outSets[sourceID].end() && targetIn != (this->inSets)[targetID].end() &&
        sourceOut->first != targetIn->first) {
      continue;
    } else if (sourceOut == this->outSets[sourceID].end() || targetIn == this->inSets[targetID].end()) {
      return false;
    } else {
      // we found a common landmark.
      // walk down sourceOut, looking for label containment on current landmark.  If found, set sFlag to true
      currentLandmark = sourceOut->first;

      // Yuich: why do we need the following two lines?
      sFlag = (labels & sourceOut->second) == sourceOut->second;
      sourceOut++;
      while (!sFlag && sourceOut != this->outSets[sourceID].end() && sourceOut->first == currentLandmark) {
        sFlag = (labels & sourceOut->second) == sourceOut->second;
        sourceOut++;
      }
      // walk down targetIn, looking for label containment on current landmark.  If found, set tFlag to true
      currentLandmark = targetIn->first;
      tFlag = (labels & targetIn->second) == targetIn->second;
      targetIn++;
      while (!tFlag && targetIn != this->inSets[targetID].end() && targetIn->first == currentLandmark) {
        tFlag = (labels & targetIn->second) == targetIn->second;
        targetIn++;
      }
    }

    if (sFlag && tFlag) {
      return true;
    } else {
      sFlag = false;
      tFlag = false;
    }
  }

  return false;
}

/*Private *********************************************************************/

void PrunedLcrLabelingB::createIndex(EdgeSet & edges) {
  // we assume that
  // (1) there is at least one vertex and one edge in the graph,
  // (2) vertex IDs are numbered consecutively, starting from zero, and
  // (3) edges is a *set*, i.e., no duplicates.


  if (edges.empty()) {
    cerr << "Empty graph. Terminating." << endl;
    exit(EXIT_FAILURE);
  }

  // start timing
  this->indexingTimeSeconds = -getCurrentTimeSec();

  // find highest node ID
  VertexID maxID = 0;
  for (Edge& edge: edges) {
    maxID = max({edge.from, edge.to, maxID});
  }
  this->V = maxID + 1;

  // scan edges, and
  // count in- and outdegree of each vertex, placing this in vector "degs", containing (in-degree, out-degree) pairs.
  vector<pair<size_t, size_t>> degs(this->V, make_pair(0, 0));
  for (Edge& edge: edges) {
    degs[edge.from].second++;
    degs[edge.to].first++;
 }

  //scan degs, for each vertex v calculating val = (in-degree(v) +1) * (out-degree(v) + 1),
  //and placing this in a new vector "inOut" containing (val, v) pairs.
  vector<pair<size_t, VertexID>> inOut(this->V, make_pair(0, 0));
  for (VertexID v: range(this->V)) {
    inOut[v] = make_pair((degs[v].first + 1) * (degs[v].second + 1), v);
  }

  //in decreasing order, sort inOut on (val, v)
  sort(inOut.rbegin(), inOut.rend());

  convertTable.clear();
  convertTable.resize(this->V);
  for (size_t v: range(this->V)) {
    convertTable[inOut[v].second] = v;
  }

  // rename vertex IDs;
  for (Edge& edge: edges) {
    edge.from = convertTable[edge.from];
    edge.to = convertTable[edge.to];
  }

  // populate rEdges
  EdgeSet rEdges(edges.size(), (Edge){numeric_limits<VertexID>::max(), numeric_limits<VertexID>::max(), numeric_limits<LabelID>::max()});
  for (size_t it: range(rEdges.size())) {
    rEdges[it] = (Edge){edges[it].to, edges[it].from, edges[it].label};
  }

  //bulk load G and rG.
  bulkLoad(edges, this->G);
  bulkLoad(rEdges, this->rG);

  //build inSets and outSets.
  bfs(this->inSets, this->G, true);
  bfs(this->outSets, this->rG, false);

  // stop timing
  this->indexingTimeSeconds += getCurrentTimeSec();
}

void PrunedLcrLabelingB::bulkLoad(EdgeSet & theEdges, Graph & theGraph) {

  VertexID prevSource, prevTarget, currentSource, currentTarget;
  LabelID currentLabel;
  LabelSet tempLabelSet = 0;

  //initialize theGraph
  theGraph.clear();
  theGraph.resize(this->V);

//  theGraph.clear();
//  theGraph.resize(this->V);

  //sort theEdges on (s, t, l), in increasing order.
  sort(theEdges.begin(), theEdges.end(), edgesComp);

  //scan theEdges and bulk load theGraph.
  EdgeSet::iterator currentEdge = theEdges.begin();
  while (currentEdge != theEdges.end()) {
    currentSource = currentEdge->from;
    currentTarget = currentEdge->to;
    currentLabel = currentEdge->label;

    if (currentEdge != theEdges.begin() && (currentSource != prevSource || currentTarget != prevTarget)) {
      theGraph[prevSource].emplace_back(prevTarget, tempLabelSet);
      tempLabelSet = 0;
    }

    tempLabelSet |= (VertexID)1 << currentLabel;

    ++currentEdge;
    prevSource = currentSource;
    prevTarget = currentTarget;
  }
  if (!theEdges.empty()) {
    theGraph[prevSource].emplace_back(prevTarget, tempLabelSet);
  }
}

void PrunedLcrLabelingB::bfs(PllSets & theSets, Graph & theGraph, bool forward) {
  queue<pair<VertexID, LabelSet>> q;

  // For each vertex v, lastVertexPosition[v] stores the (potential) position i such that theSets[v][i] stores the landmark that previously visited v.
  vector<size_t> lastVertexPosition(this->V, 0);

  theSets.clear();
  theSets.resize(this->V);
  int roundNo = 0;

  for (VertexID v: range(this->V)) {
      if (v > 0 && v%100 == 0) {
		  cout << "v=" << v << ",roundNo=" << roundNo << endl;
	  }
      if( (this->indexingTimeSeconds+getCurrentTimeSec()) >= TIMEOUT )
      {
          cout << "PrunedLcrLabelingB timeout" << endl;
          break;
      }
      roundNo = 0;
    theSets[v].emplace_back(v, 0);
    if (theGraph[v].empty())
      continue;

    // push v's children on q
    for (pair<VertexID, LabelSet>& edge: theGraph[v]) {
      for (LabelSet ls = edge.second; ls;) {
        LabelSet s = ls & (-ls);
        q.push(make_pair(edge.first, s));
        ls -= s;
      }
    }

    while (!q.empty()) {
      //pop q, retrieving (w, L)
      roundNo++;
      pair<VertexID, LabelSet> item = q.front();
      q.pop();

      // check for pruning
      if (forward  && internalQuery(v, item.first, item.second))
        continue;
      if (!forward && internalQuery(item.first, v, item.second))
        continue;

      //update theSet = theSets[item.first] with v and item.second
      //if "filter condition 1" holds, set pushFlag=false
      PllSet& theSet = theSets[item.first];
      bool pushFlag = true;
      size_t it = lastVertexPosition[item.first];
      if (it >= theSet.size() || theSet[it].first != v) {
        lastVertexPosition[item.first] = it = theSet.size();
      }

      while(it < theSet.size()) {
        assert(theSet[it].first == v);
        if ((item.second & theSet[it].second) == theSet[it].second) {
          // item.second contains theSet[it].second
          pushFlag = false;
          break;
        } else if ((item.second & theSet[it].second) == item.second) {
          // item.second is contained in theSet[it].second
          swap(theSet[it], theSet[theSet.size() - 1]);
          theSet.pop_back();
        } else {
          it++;
        }
      }
      if (pushFlag) {
        theSet.emplace_back(v, item.second);
      }

      if (pushFlag && item.first != v) {
        // push item.first's children on q
        // smaller label sets should be pushed earlier.
        for (pair<VertexID, LabelSet>& edge: theGraph[item.first]) {
          for (LabelSet ls = edge.second; ls; ) {
            LabelSet s = ls & (-ls);
            q.emplace(edge.first, item.second | s);
            ls -= s;
          }
        }
      }
    }
  }
}

// timing from Akiba
double PrunedLcrLabelingB::getCurrentTimeSec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}
// the end
