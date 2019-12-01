#include "../../Index/UnboundedLCR/alternative/pruned_lcr_labeling_bits.cc"

#include <string>

using namespace std;
using namespace plcrlb;

void test(bool a, int& t)
{
    if( a == true )
    {
        cout << "testID=" << to_string(t) << " passed" << endl;
    }
    else
    {
        cout << "testID=" << to_string(t) << " failed" << endl;
    }

    t++;
}

void runQuery(PrunedLcrLabelingB* index, VertexID s, VertexID ta, LabelSet ls, bool answer, int& t)
{
    test(index->query(s, ta, ls) == answer, t);
}

int main(int argc, char *argv[])
{
    int t = 0;
    string s = "tests/graphs/testGraph7.edge";
    EdgeSet es;
    load_edge_file(s, es);
    PrunedLcrLabelingB* index = new PrunedLcrLabelingB(es);

    cout << "time (s)=" << index->indexingTime() << endl;
    cout << "size (byte)=" << index->indexSize() << endl;

    runQuery(index, 2, 11, 136, true, t);
    runQuery(index, 2, 11, 1, false, t);

    cout << "-----" << endl;

    s = "experiments/graphs/real/jmd/graphs/jmd.edge";
    es.clear();
    load_edge_file(s, es);
    index = new PrunedLcrLabelingB(es);

    cout << "time (s)=" << index->indexingTime() << endl;
    cout << "size (byte)=" << index->indexSize() << endl;

    cout << "-----" << endl;
    s = "experiments/graphs/real/yagoFacts-small/yagoFacts-graphs/small.edge";
    es.clear();
    load_edge_file(s, es);
    index = new PrunedLcrLabelingB(es);

    cout << "time (s)=" << index->indexingTime() << endl;
    cout << "size (byte)=" << index->indexSize() << endl;

    return 0;
}
