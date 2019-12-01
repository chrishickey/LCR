#include "pch.h"
#include "Splitedgraphs.h"


Splitedgraphs::Splitedgraphs()
{
}

Splitedgraphs::Splitedgraphs(Graph* first)
{
	graph = first;

}

list<LabelSet*> Splitedgraphs::all_label_compinations(string filename)
{
	cout << "filename :" << filename << endl;
	string line;
	list<LabelSet*> ls;

	fstream edge_file;
	edge_file.open((filename));
	if (edge_file.is_open())
	{

		while (getline(edge_file, line))
		{
			LabelSet* set = new LabelSet();
			stringstream ss(line);


			char * cstr = new char[line.length() + 1];
			strcpy(cstr, line.c_str());
			char * p = strtok(cstr, " ");

			while (p != 0)
			{

				if (p) {
					set->add_label(p);
				}

				p = strtok(NULL, " ");
			}

			unordered_set<string>* st = set->get_labels();

			ls.push_back(set);
			set = NULL;
		}
	}

	return ls;
}

void Splitedgraphs::printlabels()
{
	unordered_map<LabelSet*, Graph> ::iterator itr;

	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{
		unordered_set<string>* labset = itr->first->get_labels();

		cout << "labels is: " << endl;

		for (auto secenditr : *labset) {

			cout << secenditr << " ";

		}

		cout << endl;

	}

}


void Splitedgraphs::create_Multi_graph(list<LabelSet*> all_labels)
{
	auto start = chrono::steady_clock::now();
	unordered_map<int, Edge*>* graph_edge_list = graph->get_edgelist();

	for (auto const& itr : all_labels) {
		Graph g;
		int i = 0;
		unordered_set<string>* set = itr->get_labels();
		for (auto setitr : *set) {
			string s = setitr;

			unordered_map<int, Edge*> ::iterator edgeitr;

			for (edgeitr = graph_edge_list->begin(); edgeitr != graph_edge_list->end(); edgeitr++) {
				i++;
			}
			int n = 0;
			for (edgeitr = graph_edge_list->begin(); edgeitr != graph_edge_list->end(); edgeitr++) {

				if (edgeitr->second->getlabel().compare(s) == 0) {

					string from = edgeitr->second->getSorceVertex()->getlabel();

					string to = edgeitr->second->getDestVertex()->getlabel();

					g.addvertex(from);
					Vertex* f = g.find_vertex(from);
					f->add_correspondLabel(s);
					g.addvertex(to);
					Vertex* t = g.find_vertex(to);
					t->add_correspondLabel(s);
					g.addedge(from, to, s);
					g.add_label(s);
					n++;
				}
			}
		}
		splitedgraphs.insert({ itr, g });
	}
	auto end = chrono::steady_clock::now();
	double elpased_time_ns = double(chrono::duration_cast <chrono::microseconds> (end - start).count());
	cout << "Create Multiple graph" << "Elpased time: " << elpased_time_ns << "------>>>>>>" << endl;
}

void Splitedgraphs::create_scc()
{
	new_SCC_graphs();
}

void Splitedgraphs::execute(string filename)
{
	list<LabelSet*> ls = all_label_compinations(filename);
	for (auto const& itr : ls) {
		unordered_set<string>*  st = itr->get_labels();
		st->erase("");
	}
	create_Multi_graph(ls);
}



int Splitedgraphs::labelset_size()
{
	return labelset->size();
}

bool Splitedgraphs::BFS(string src, string to, unordered_set<string> a)
{
	cout << "runing bfs" << endl;
	unordered_map<LabelSet*, Graph> ::iterator itr;

	LabelSet* ls = check(a);
	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{
		if (itr == splitedgraphs.find(ls)) {
			return itr->second.BFS(src, to);
		}


	}
	return false;
}

bool Splitedgraphs::BFS_SCC(string src, string to, unordered_set<string> a)
{

	cout << "runing bfs" << endl;
	unordered_map<LabelSet*, Graph> ::iterator itr;

	LabelSet* ls = check(a);
	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{
		if (itr == splitedgraphs.find(ls)) {
			return itr->second.BFS(src, to);
		}


	}
	
}

LabelSet* Splitedgraphs::check(unordered_set<string> given)
{
	unordered_map<LabelSet*, Graph> ::iterator itr;


	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{
		if (itr->first->check_similarity(given)) {
			return itr->first;
		}

	}
	return nullptr;
}



string Splitedgraphs::print_all_graphs()
{
	cout << "printing" << endl;
	string s = "";
	unordered_map<LabelSet*, Graph> ::iterator itr;

	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{

		unordered_set<string>* labset = itr->first->get_labels();
		//s += "labels is: ";
		s += "";

		//cout << "labels is: " << endl;
		for (auto secenditr : *labset) {
			//s += secenditr + ",";
			//cout << secenditr + ","<< endl;
		}

		//cout << endl;
		//s += "\n";
		//cout << "Graph is: " << endl;
		//s += "Graph is: ";
		cout << itr->second.vertex_size() << endl;
		//s += itr->second.vertex_size();
	}
	return s;
}

void Splitedgraphs::different_scc()
{
	unordered_map<LabelSet*, Graph> ::iterator itr;
	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{

		unordered_set<string>* labset = itr->first->get_labels();
		//s += "labels is: ";
		//s += "";

		//cout << "labels is: " << endl;
		for (auto secenditr : *labset) {
			//s += secenditr + ",";
			//cout << secenditr + ","<< endl;
		}

		//cout << endl;
		//s += "\n";
		//cout << "Graph is: " << endl;
		//s += "Graph is: ";
		cout << endl;
		 itr->second.SCC();
	}
	
}

void Splitedgraphs::Splited_graphs_size()
{
	unordered_map<LabelSet*, Graph> ::iterator itr;
	int edge = 0;
	int vertex = 0;
	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{


		edge += itr->second.edge_size();


		vertex += itr->second.vertex_size();

	}

	cout << "edge size:" << endl;
	cout << edge << endl;
	cout << "vertex size:" << endl;
	cout << vertex << endl;

}

void Splitedgraphs::new_SCC_graphs()
{
	unordered_map<LabelSet*, Graph> ::iterator itr;
	for (itr = splitedgraphs.begin(); itr != splitedgraphs.end(); itr++)
	{

		unordered_set<string>* labset = itr->first->get_labels();
		//s += "labels is: ";
		//s += "";

		//cout << "labels is: " << endl;
		for (auto secenditr : *labset) {
			//s += secenditr + ",";
			//cout << secenditr + ","<< endl;
		}

		//cout << endl;
		//s += "\n";
		//cout << "Graph is: " << endl;
		//s += "Graph is: ";
		cout << endl;
		Graph* g = itr->second.SCC_graph();
		LabelSet* l = itr->first;
		//SCCgraphs.emplace( l, g );
	}
}




Splitedgraphs::~Splitedgraphs()
{
}
