/* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).
 * Pruned-landmark-LCR-labeling-based indexing of edge-labeled directed graphs.
 * Using bitsets for label sets.
 * October 2014 */
#pragma once
#define PLL_BITS 64
#include <vector>
#include <utility>
#include <map>
#include <bitset>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

class range {
private:
  struct I {
    size_t x;
    size_t operator*() { return x ;}
    bool operator!=(I& lhs) { return x<lhs.x; }
    void operator++() { ++x; }
  };
  I i_,n_;
public:
  range(size_t n):i_({0}),n_({n}) { }
  range(size_t i, size_t n):i_({i}),n_({n}) { }
  I& begin() { return i_; }
  I& end() { return n_; }
};

namespace plcrlb {
  // Vertex IDs
  typedef size_t VertexID;
  // Edge label IDs
  typedef unsigned short LabelID;
  // An edge
  // first ID is the source, second ID is the target, LabelID is edge label
  // NOTE: gcc on Mac OS doesn't understand "tuple" (since it doesn't support c++11),
  // i.e., the following doesn't work:
  // typedef std::tuple < VertexID, LabelID, VertexID > Edge;
  // FIX THIS LATER.
  struct Edge {
    VertexID from, to;
    LabelID label;
  };
  // An edge list
  typedef vector<Edge> EdgeSet;
  // Label sets
  typedef uint64_t LabelSet;
//  typedef bitset< PLL_BITS > LabelSet;
  // A graph is a vector V of vertices, numbered sequentially from zero.
  // For each vertex s (giving an offset into V), we store an (ordered) map T of its neighbors.
  // In T, for each neighbor vertex t of s, we store the pair (t, L), where t is the vertex ID
  // and L is a vector of all the labels on edges from s to t.
  // NOTE: gcc on Max OS doesn't understand "unordered_map" (since it doesn't support c++11).
  // We don't need ordered access to neighbors -- otherwise, we could just directly use
  // the sorted edge list as our graph representation, using binary search for access.
  // FIX THIS LATER.
  typedef vector<vector<pair<VertexID, LabelSet>>> Graph;
  // PLL-sets is a vector P of vertices, one for each vertex of the underlying Graph.
  // For each vertex v in the graph (giving an offset into P), we store a PllSet,
  // which is an (ordered) multimap, from vertex IDs to (sorted) vectors of edge labels.
  // PLLSets are used to store inSets and outSets.
  typedef vector<pair<VertexID, LabelSet>> PllSet;
  typedef vector<PllSet> PllSets;

  const double TIMEOUT = 30.0 * 60.0;
}

class PrunedLcrLabelingB {
  private:
    //constants
    static const unsigned long long PLLSETS_SIZE = (unsigned long long)sizeof(vector<plcrlb::PllSet>);
    static const unsigned long long PLLSET_SIZE = (unsigned long long)sizeof(plcrlb::PllSet);
    static const unsigned long long LABEL_SIZE = (unsigned long long)sizeof(plcrlb::LabelID);
    static const unsigned long long BUCKET_OVERHEAD_SIZE =   (unsigned long long)sizeof(plcrlb::LabelSet )
                                                           + (unsigned long long)sizeof(plcrlb::VertexID);
    // the graph G, and its reverse graph rG (i.e., where the direction of each edge is reversed)
    plcrlb::Graph G, rG;
    // number of edges in G
    size_t E;
    // number of vertices
    size_t V;
    // the out- and in-Sets
    // each PllSet is sorted by internal vertex IDs of landmark vertices but may not be sorted by label sets.
    plcrlb::PllSets outSets, inSets;
    // from original vertex ID to internal vertex ID
    // internal vertex IDs are obtained by sorting orignal vertex IDs by InOut ranking (in decreasing order).
    vector<plcrlb::VertexID> convertTable;


    double indexingTimeSeconds;
    double getCurrentTimeSec();
    void createIndex(plcrlb::EdgeSet &);
    void bulkLoad(plcrlb::EdgeSet &, plcrlb::Graph &);
    // BFS.
    // If the boolean parameter is true, then this is BFS on G, and rG otherwise.
    void bfs(plcrlb::PllSets &, plcrlb::Graph &, bool);

  public:
    PrunedLcrLabelingB(plcrlb::EdgeSet &);
    ~PrunedLcrLabelingB();
    size_t vertexCount();
    size_t edgeCount();
    unsigned long long indexSize();

    double indexingTime();
    void dumpGraph();
    void dumpPllSets();

    // issue query after converting IDs to internal IDs
    bool query(plcrlb::VertexID, plcrlb::VertexID, plcrlb::LabelSet &);

    // issue query using internal IDs
    bool internalQuery(plcrlb::VertexID, plcrlb::VertexID, plcrlb::LabelSet &);
};
