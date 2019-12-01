#include "pch.h"
#include "Edge.h"


Edge::Edge(Vertex* dest)
{
	destination = dest;
}

Edge::Edge() : destination(NULL), label("")
{
}

Edge::Edge(Vertex* src, Vertex* dest, string lab, int idx)
{
	source = src;
	destination = dest;
	label = lab;
	id = idx;
}

void Edge::setlabel(string lab)
{
	label = lab;
}


string Edge::getlabel()
{
	return label;
}

Vertex* Edge::getDestVertex()
{
	return destination;
}
Vertex* Edge::getSorceVertex()
{
	return source;
}


Edge::~Edge()
{
}