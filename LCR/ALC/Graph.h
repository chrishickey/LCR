#pragma once
#include <unordered_map>
#include <unordered_set>
#include "Edge.h"
#include "Vertex.h"
#include <iterator>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack> 
class Edge;
using namespace std;


class Graph
{
private:
	unordered_map<string, Vertex*> ver_map; //vertex map
	unordered_map<int, string*> corespond;
	unordered_map<int, Edge*> edge_map;
	unordered_set<string> labelset;
	int number_of_scc;
	Graph* cor_scc;

public:
	Graph();
	int vertex_size();
	int edge_size();
	void addvertex(string);
	void addedge(string, string, string);
	//bool contain_vertex(string);
	void add_label(string);
	Vertex* find_vertex(string);
	string print();
	void BFS(string s);
	bool check_vertices_labelset(string src, string to);
	bool BFS(string s, string to);
	bool BFS_scc(string s, string to);
	void loadedgefile(string);
	unordered_map<int, Edge*>* get_edgelist();
	void maping();
	void printlabelset();
	void DFSutil(Vertex*, int i);
	Vertex* get_vertex(string s);
	Graph* getTranspose();
	void fillorder(Vertex* s, stack<Vertex*> &stack);
	void SCC();
	list<Vertex*> landmarkIndex(int);
	list<Vertex*> sortV();
	void merge(unordered_map<string, Vertex*>, int l, int m, int r);
	int get_num_scc();
	Graph* SCC_graph();
	~Graph();
};
