// ConsoleApplication2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Graph.h"
#include "Vertex.h"
#include "Splitedgraphs.h"
#include <fstream>
#include <chrono>


int main(int argc, char** argv)
{
	int counter;
	printf("What is the graph?", argv[0]);
	Graph* graph = new Graph();
	cout << "creat" << endl;
	cout << "What is the graph?" << endl;
	string graph_input = argv[0];
	graph->loadedgefile("graph_input.txt");
	//graph->maping();
	//graph->SCC();
	//graphs.print();
	//graphs.BFS("a");
	cout << "Done" << endl;
	cout << "Number of Vertices:" << graph->vertex_size() << endl;
	cout << "Number of Edges:" << graph->edge_size() << endl;
	Splitedgraphs* sp = new Splitedgraphs(graph);
	sp->execute("AllCompination6.txt");
	
	sp->different_scc();
	fstream myfile;
	myfile.open("createdfile4.txt", fstream::in | fstream::out | fstream::app);
	
	cout << sp->print_all_graphs();
	myfile.close();
	//sp->Splited_graphs_size();
	unordered_set<string> a;
	cout << "here" << endl;
}



