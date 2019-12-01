#include "pch.h"
#include "Graph.h"
#include <unordered_set>
#include <iterator>
#include <chrono>
Graph::Graph()
{
}

void Graph::addvertex(string label)
{

	Vertex* newvertex = new Vertex(label);
	newvertex->set_degree(0);
	unordered_map <string, Vertex*> ::iterator itr;
	ver_map.insert({ label, newvertex });
	for (itr = ver_map.begin(); itr == ver_map.find(label); itr++) {
		if (itr == ver_map.end()) {
			ver_map.insert({ label, newvertex });

		}
	}
}

int Graph::vertex_size()
{
	return ver_map.size();
}

int Graph::edge_size()
{
	return edge_map.size();
}


void Graph::addedge(string v1, string v2, string label)
{

	static int var = 0;
	unordered_map <string, Vertex*> ::iterator itr;
	Vertex* from = NULL;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++) {

		if (itr == ver_map.find(v1)) {
			from = const_cast <Vertex*> (itr->second);
			break;
		}

	}

	unordered_map <string, Vertex*> ::iterator itr2;
	Vertex* to = NULL;
	for (itr2 = ver_map.begin(); itr2 != ver_map.end(); itr2++) {
		if (itr2 == ver_map.find(v2)) {
			to = const_cast <Vertex*> (itr2->second);
			break;
		}
	}

	Edge* newedge = new Edge(from, to, label, var);
	
	edge_map.insert({ var, newedge });
	from->add_edge(newedge);
	from->add_degree();                // add on to degree of vertex 
	add_label(label);
	var++;

}

void Graph::add_label(string lab)
{
	labelset.insert(lab);
}

Vertex* Graph::find_vertex(string v)
{
	unordered_map <string, Vertex*> ::iterator itr;
	Vertex* specific = NULL;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		if (itr == ver_map.find(v)) {
			specific = const_cast <Vertex*> (itr->second);
			break;
		}
	}
	return specific;
}

string Graph::print()
{
	string s = "";
	//cout << "this our graph" << endl;
	unordered_map<int, Edge*> ::iterator itr;
	unordered_map<int, string*> ::iterator coritr;
	int from = 0;
	int to = 0;

	for (itr = edge_map.begin(); itr != edge_map.end(); itr++) {
		Edge *e = itr->second;
		s += e->getSorceVertex()->getlabel();
		s += " ";
		s += (e->getDestVertex())->getlabel();
		s += " ";
		s += e->getlabel();
		s += "\n";
	}
	return s;
}

void Graph::BFS(string s)
{
	unordered_map <string, Vertex*> ::iterator itr;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		itr->second->set_status(false);
	}
	//set s visited
	Vertex* svertex = find_vertex(s);
	svertex->set_status(true);
	list<Vertex*> queue;
	queue.push_back(svertex);


	while (!queue.empty())
	{
		svertex = queue.front();
		cout << svertex->getlabel() << " " << endl;
		queue.pop_front();
		cout << svertex->getedge().size() << endl;
		for (auto const& it : svertex->getedge())
		{
			cout << it->getSorceVertex()->getlabel() << " " << it->getlabel() << " " << it->getDestVertex()->getlabel() << endl;
			Vertex* a = it->getDestVertex();
			if (!a->get_status())
			{
				a->set_status(true);
				queue.push_back(a);
			}

		}

	}

}

bool Graph::check_vertices_labelset(string src, string to)
{

	Vertex* sr = this->find_vertex(src);
	Vertex* t = this->find_vertex(to);
	unordered_set <string>* srcCL = sr->getcl();
	unordered_set <string>* tCL = t->getcl();
	for (auto itr : *srcCL) {
		for (auto itr2 : *tCL) {
			if (itr.compare(itr2) == 0)
				return true;
		}
	}
	return false;
}

bool Graph::BFS(string s, string to)
{
	cout << "runing inner bfs" << endl;
	cout << s << "------>" << to << endl;
	unordered_map <string, Vertex*> ::iterator itr;
	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now();

	//if (!check_vertices_labelset(s, to)) {
	//	end = chrono::steady_clock::now();

	//	double elpased_time_ns = double(chrono::duration_cast <chrono::microseconds> (end - start).count());
	//	cout << "false" << endl;
	//	//cout << "Elpased time: " << elpased_time_ns << "------>>>>>>"  << endl;
	//	return false;
	//}
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{

		itr->second->set_status(false);
	}
	//set s visited
	Vertex* svertex = find_vertex(s);
	svertex->set_status(true);
	list<Vertex*> queue;
	queue.push_back(svertex);


	while (!queue.empty())
	{
		svertex = queue.front();
		//cout << svertex->getlabel() << " " << endl;
		queue.pop_front();
		//cout << svertex->getedge().size() << endl;
		for (auto const& it : svertex->getedge())
		{
			//cout << it->getSorceVertex()->getlabel() << " " << it->getlabel() << " " << it->getDestVertex()->getlabel() << endl;
			Vertex* a = it->getDestVertex();
			if (!a->get_status())
			{

				if (a->getlabel().compare(to) == 0)
				{
					cout << "true" << endl;
					end = chrono::steady_clock::now();
					double elpased_time_ns = double(chrono::duration_cast <chrono::microseconds> (end - start).count());
					cout << "Elpased time: " << elpased_time_ns << "------>>>>>>" << to << elpased_time_ns << endl;
					return true;
				}
				a->set_status(true);
				queue.push_back(a);
			}

		}

	}
	end = chrono::steady_clock::now();
	double elpased_time_ns = double(chrono::duration_cast <chrono::microseconds> (end - start).count());
	cout << "false" << endl;

	cout << "Elpased time: " << elpased_time_ns << "------>>>>>>" << to << endl;
	return false;
}
bool Graph::BFS_scc(string s, string to)
{
	Vertex* src = get_vertex(s);
	Vertex* t = get_vertex(to);
	int s_scc = src->get_scc();
	int t_scc = t->get_scc();

	cout << "runing inner bfs" << endl;
	cout << s << "------>" << to << endl;
	unordered_map <string, Vertex*> ::iterator itr;
	auto start = chrono::steady_clock::now();
	auto end = chrono::steady_clock::now();
	if (s_scc == t_scc) {
		end = chrono::steady_clock::now();
		double elpased_time_ns = double(chrono::duration_cast <chrono::microseconds> (end - start).count());
		cout << "Elpased time: " << elpased_time_ns << "------>>>>>>" << to << elpased_time_ns << endl;
		return true;
	}
	
	else {
		cor_scc->BFS(s_scc+"", t_scc+"");
	}
}
void Graph::loadedgefile(string filename)
{
	cout << "filename :" << filename << endl;
	string line;
	static int var = 0; //set the correspond value;
	fstream edge_file;
	edge_file.open((filename));
	if (edge_file.is_open())
	{

		while (getline(edge_file, line))
		{

			stringstream linestream(line);
			string from;
			getline(linestream, from, ' ');
			addvertex(from);
			string to;
			getline(linestream, to, ' ');
			addvertex(to);
			string label;
			getline(linestream, label);
			addedge(from, to, label);
		}
	}
}

unordered_map<int, Edge*>* Graph::get_edgelist()
{
	return &edge_map;
}

void Graph::maping()
{
	unordered_map <string, Vertex*> ::iterator itr;
	int n = 1;
	for (itr = ver_map.begin(); itr == ver_map.end(); itr++) {
		//string s = itr->first;
		//string* ps = &s;
		//corespond.insert({ n,ps });
		//cout << n;
		//n++;
	}
}

void Graph::printlabelset()
{
	for (auto const& itr : labelset) {
		cout << itr << endl;
	}
}

void Graph::DFSutil(Vertex* v, int i)
{
	//cout << v->getlabel() << " ";
	//stck.push(v);
	v->set_status(true);
	v->set_scc(i);
	for (auto const& itr : v->getedge()) {
		if (!itr->getDestVertex()->get_status())
			DFSutil(itr->getDestVertex(), i);
	}

}

Vertex * Graph::get_vertex(string s)
{
	unordered_map <string, Vertex*> ::iterator itr;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		if (itr->second->getlabel().compare(s) == 0)
			return itr->second;
	}
	return nullptr;
}

Graph * Graph::getTranspose()
{
	Graph* g = new Graph();
	unordered_map <string, Vertex*> ::iterator itr;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		Vertex* from = itr->second;
		string f = from->getlabel();
		for (auto const& itr2 : from->getedge()) {
			Vertex* to = itr2->getDestVertex();
			string t = to->getlabel();
			g->addvertex(f);
			g->addvertex(t);
			string l = itr2->getlabel();
			g->addedge(t, f, l);
		}
			
	}
	return g;
}

void Graph::fillorder(Vertex* v, stack<Vertex*>& stack)
{
	v->set_status(true);
	for (auto const& itr : v->getedge()) {
		if (!itr->getDestVertex()->get_status())
			fillorder(itr->getDestVertex(), stack);
	}
	stack.push(v);
}

void Graph::SCC()
{
	stack<Vertex*> stck;
	unordered_map <string, Vertex*> ::iterator itr;
	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		itr->second->set_status(false);
	}

	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		if (!itr->second->get_status())
			fillorder(itr->second, stck);
	}

	Graph* gr = getTranspose();

	for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
	{
		itr->second->set_status(false);
	}
	
	int i = 0;
	while (stck.empty() == false) {
		Vertex* v = stck.top();
		stck.pop();
		
		if (!v->get_status())
		{
			
			gr->DFSutil(v, i);
			
			i++;
			//cout << endl;
		}
		
	}
	/*	unordered_map<int, stack<Vertex*>> :: iterator itr2;
		Graph* g = new Graph();
		for (itr2 = mymap.begin(); itr2 != mymap.end(); itr2++)
		{
			g->addvertex(itr->first);
		}
		return g;
	*/
	number_of_scc = i;
	cout << i << " ";
}

int Graph::get_num_scc()
{
	return number_of_scc;
}

Graph * Graph::SCC_graph()
{
	//create New Graph based on SCCs
	Graph* g = new Graph();

	unordered_map <string, Vertex*> ::iterator itr;
		int i = get_num_scc();
		int j = 0;
		while (j < i) {
			if (itr->second->get_scc() == j)
				g->addvertex(j+"");
		}
		for (itr = ver_map.begin(); itr != ver_map.end(); itr++)
		{
			int i = get_num_scc();
			int j = 0;
			while (j < i) {
				if (itr->second->get_scc() == j)
					for (auto const& itr2 : itr->second->getedge()) {
						if (itr2->getDestVertex()->get_scc() != j)
							g->addedge(j + "", itr2->getDestVertex()->get_scc() +"", itr2->getlabel());
					}
			}
		}
		cor_scc = g;
	return g;
}

list<Vertex*> Graph::landmarkIndex(int k) {
	list<Vertex*> sorted;

	return list<Vertex*>();
}

list<Vertex*> Graph::sortV()
{
	list<Vertex*> sorted;

	return list<Vertex*>();
}

void Graph::merge(unordered_map<string, Vertex*>, int l, int m, int r)
{
	cout << "";
}



Graph::~Graph()
{

}