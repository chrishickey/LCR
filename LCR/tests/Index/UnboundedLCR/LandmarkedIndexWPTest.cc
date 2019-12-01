#include "../../Index/UnboundedLCR/LandmarkedIndexWP.cc"
#include "../../Graph/DGraph.cc"

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

void runQuery(IndexWP* index, VertexID v, VertexID w, LabelSet ls, Path& p, bool answer, int eSize, int& t)
{
    p.clear();
    test(index->queryWP(v,w,ls,p) == answer, t);
    cout << "query(" << v << "," << w << "," << ls << ") took time=" << index->getLastQueryTime() << endl;
}

int main(int argc, char *argv[])
{
    int t = 1;
    string s = "tests/graphs/testGraph1.edge";
    DGraph* g = new DGraph(s);
    Path p;

    LandmarkedWP* liWP = new LandmarkedWP(g);

    runQuery(liWP, 0, 1, 25, p, true, 1, t);
    runQuery(liWP, 3, 5, 3, p, true, 1, t);

    runQuery(liWP, 0, 1, 1, p, false, 0, t);
    runQuery(liWP, 3, 5, 1, p, false, 0, t);

    //cout << liWP->toString(0) << endl;
    s = "tests/graphs/V100D5L8uni.edge";
    g = new DGraph(s);
    liWP = new LandmarkedWP(g);

    s = "tests/graphs/V2kD3L8exp.edge";
    g = new DGraph(s);
    liWP = new LandmarkedWP(g);

    return 0;
}
