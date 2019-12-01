#include "pch.h"
#include "LabelSet.h"


LabelSet::LabelSet()
{
	sets.insert({ "" });
}

void LabelSet::add_label(string lab)
{
	if (lab.compare("") != 0)
	{
		sets.insert({ lab });
	}

}





int LabelSet::label_set_size()
{
	return sets.size();
}

bool LabelSet::check_similarity(unordered_set<string> s)
{
	bool result = false;
	bool check_with_all = false;
	unordered_set<bool> all;
	if (s.size() == this->label_set_size()) {
		for (auto itr : s)
		{
			for (auto this_itr : sets) {
				if (itr.compare(this_itr) == 0) {
					check_with_all = true;
					break;
				}
				else {
					check_with_all = false;
				}

			}
			if (check_with_all == true) {
				all.insert({ true });
			}
			else
				all.insert({ false });


		}

		for (auto al : all) {
			if (al != true) {
				result = false;
				break;
			}
			result = true;
		}

	}
	return result;
}

unordered_set<string>* LabelSet::get_labels()
{
	return &sets;
}




LabelSet::~LabelSet()
{

}

bool operator==(LabelSet & lhs, unordered_set<string>* sets)
{
	if (lhs.get_labels() == sets) {
		return true;
	}

	return false;
}
