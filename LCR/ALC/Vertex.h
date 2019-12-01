#pragma once
#include <iostream>
#include <list>
#include <string>
#include <unordered_set>
using namespace std;
class Edge;
struct vertex_label {
	string label;
	int correspond;
};
class Vertex

{
private:
	string label;
	int label_int;
	vertex_label lb;
	list<Edge*> edgelist;
	unordered_set <string> correspond_label;
	bool visited;
	int crs_scc; //coresond scc
	int degree; 

public:

	Vertex(string label);

	Vertex();
	string getlabel();
	list<Edge*> getedge();
	void add_edge(Edge*);
	void add_correspondLabel(string);
	bool get_status(); //return visit status
	unordered_set <string>* getcl();
	void set_status(bool); //return visit status
	int get_intlabel();
	void set_scc(int s);
	int get_scc();
	int get_degree();
	void set_degree(int);
	int add_degree(); // add one to the vertex's degree
	~Vertex();
};
