#pragma once
#include <iostream>
#include <iostream>

#include <string>
using namespace std;
class Vertex;
class Edge
{
private:
	string label;
	Vertex* destination;
	Vertex* source;
	int id;
public:
	Edge();
	Edge(Vertex* dest, Vertex* src, string label, int);
	Edge(Vertex* dest);
	string getlabel();
	Vertex* getDestVertex();
	Vertex* getSorceVertex();
	void setlabel(string label);

	~Edge();
};


