#pragma once
#include <iostream>
#include <string>
#include <list>
#include <unordered_set>
using namespace std;
class LabelSet
{
private:

	unordered_set<string> sets;

public:
	LabelSet();
	void add_label(string);
	friend bool operator== (LabelSet & lhs, unordered_set<string>* sets);
	bool check_similarity(unordered_set<string>);
	unordered_set<string>* get_labels();
	int label_set_size();
	~LabelSet();
};

