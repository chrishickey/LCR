#include "pch.h"
#include "Vertex.h"


Vertex::Vertex(string lab)
{
	label = lab;

}
Vertex::Vertex()
{
	label = "";
}

string Vertex::getlabel()
{
	return label;
}

list<Edge*> Vertex::getedge()
{
	return edgelist;
}

void Vertex::add_edge(Edge * e)
{
	edgelist.push_back(e);
}

void Vertex::add_correspondLabel(string s)
{
	correspond_label.insert(s);

}

bool Vertex::get_status()
{
	return visited;
}

unordered_set<string>* Vertex::getcl()
{
	return &correspond_label;
}

void Vertex::set_status(bool stat)
{
	visited = stat;
}

int Vertex::get_intlabel()
{
	return label_int;

}

void Vertex::set_scc(int s)
{
	crs_scc = s;
}

int Vertex::get_scc()
{
	return crs_scc;
}

int Vertex::get_degree()
{
	return degree;
}

void Vertex::set_degree(int d)
{
	degree = d;
}

int Vertex::add_degree()
{
	return degree++;
}


Vertex::~Vertex()
{

}
