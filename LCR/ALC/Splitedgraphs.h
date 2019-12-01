#pragma once
#include <unordered_map>
#include <unordered_set>
#include <list>
#include "LabelSet.h"
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Edge.h"
#pragma once
#include <unordered_map>
#include <unordered_set>
#include <list>
#include "LabelSet.h"
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Edge.h"
#include <cstring>
#include <string>
#include <chrono>
using namespace std;
class Splitedgraphs
{
private:
	Graph* graph;
	unordered_set<string>* labelset;
	unordered_map<LabelSet*, Graph> splitedgraphs;
	unordered_map<LabelSet*, Graph> SCCgraphs;
	LabelSet *set[256];




public:
	Splitedgraphs();
	Splitedgraphs(Graph*);
	list<LabelSet*>  all_label_compinations(string);
	void printlabels();
	void create_Multi_graph(list<LabelSet*>);
	void create_scc();
	void execute(string);
	int labelset_size();
	bool BFS(string, string, unordered_set<string>);
	bool BFS_SCC(string, string, unordered_set<string>);
	LabelSet* check(unordered_set<string>);
	string print_all_graphs();
	void different_scc();
	void Splited_graphs_size();
	void new_SCC_graphs();
	void run_test();
	~Splitedgraphs();
};

