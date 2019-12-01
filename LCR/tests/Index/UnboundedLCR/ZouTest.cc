#include "../../Index/UnboundedLCR/Zou.cc"
#include "../../Graph/DGraph.cc"

#include <string>

using namespace std;
using namespace graphns;
using namespace indexns;

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

void runQuery(Index* index, VertexID v, VertexID w, LabelSet ls, bool answer, int& t)
{
    test(index->query(v,w,ls) == answer, t);
    cout << "query(" << v << "," << w << "," << ls << ") took time(s)=" << index->getLastQueryTime() << endl;
}

int main(int argc, char *argv[])
{
    Graph* g1 = new DGraph("tests/graphs/Zoudummy.edge");
    int t = 1;
    Zou* zI = new Zou(g1);

    cout << zI->toString() << endl;


    return 1;

    //cout << zI->toString() << endl;

    g1 = new DGraph("tests/graphs/ZouGraph.edge");
    zI = new Zou(g1);

    runQuery(zI,0,5,13,true,t);
    runQuery(zI,0,6,21,true,t);
    runQuery(zI,1,0,11,true,t);
    runQuery(zI,1,2,19,true,t);
    runQuery(zI,2,0,7,true,t);
    runQuery(zI,2,1,7,true,t);
    runQuery(zI,2,4,13,true,t);
    runQuery(zI,2,5,7,true,t);
    runQuery(zI,2,6,37,true,t);
    runQuery(zI,3,10,35,true,t);
    runQuery(zI,4,7,25,true,t);
    runQuery(zI,5,6,7,true,t);
    runQuery(zI,6,5,11,true,t);
    runQuery(zI,6,9,7,true,t);
    runQuery(zI,10,8,49,true,t);
    runQuery(zI,12,4,21,true,t);
    runQuery(zI,13,0,13,true,t);
    runQuery(zI,13,0,14,true,t);
    runQuery(zI,13,0,25,true,t);
    runQuery(zI,13,3,11,true,t);

    runQuery(zI,0,4,41,false,t);
    runQuery(zI,1,5,25,false,t);
    runQuery(zI,3,5,41,false,t);
    runQuery(zI,4,0,19,false,t);
    runQuery(zI,4,2,7,false,t);
    runQuery(zI,5,0,11,false,t);
    runQuery(zI,5,0,25,false,t);
    runQuery(zI,6,0,19,false,t);
    runQuery(zI,7,2,41,false,t);
    runQuery(zI,9,0,37,false,t);
    runQuery(zI,9,2,13,false,t);
    runQuery(zI,10,0,42,false,t);
    runQuery(zI,10,0,49,false,t);
    runQuery(zI,10,1,37,false,t);
    runQuery(zI,11,0,38,false,t);
    runQuery(zI,11,1,49,false,t);
    runQuery(zI,11,2,25,false,t);
    runQuery(zI,13,5,26,false,t);
    runQuery(zI,15,0,11,false,t);
    runQuery(zI,16,0,49,false,t);

    cout << "-------------" << endl;

    g1 = new DGraph("tests/graphs/V100D5L8uni.edge");
    zI = new Zou(g1);

    runQuery(zI,4,0,304,true,t);
    runQuery(zI,10,1,28,true,t);
    runQuery(zI,11,1,21,true,t);
    runQuery(zI,21,0,224,true,t);
    runQuery(zI,27,3,290,true,t);
    runQuery(zI,53,0,193,true,t);
    runQuery(zI,59,0,26,true,t);
    runQuery(zI,62,1,131,true,t);
    runQuery(zI,64,3,164,true,t);
    runQuery(zI,65,0,140,true,t);
    runQuery(zI,69,0,112,true,t);
    runQuery(zI,71,4,140,true,t);
    runQuery(zI,75,1,140,true,t);
    runQuery(zI,79,0,97,true,t);
    runQuery(zI,80,7,176,true,t);
    runQuery(zI,81,0,49,true,t);
    runQuery(zI,89,0,42,true,t);
    runQuery(zI,94,1,97,true,t);
    runQuery(zI,97,0,25,true,t);
    runQuery(zI,98,1,100,true,t);

    runQuery(zI,0,1,265,false,t);
    runQuery(zI,6,3,84,false,t);
    runQuery(zI,8,1,296,false,t);
    runQuery(zI,16,5,266,false,t);
    runQuery(zI,22,1,352,false,t);
    runQuery(zI,24,1,352,false,t);
    runQuery(zI,27,3,26,false,t);
    runQuery(zI,39,3,324,false,t);
    runQuery(zI,48,3,22,false,t);
    runQuery(zI,58,0,328,false,t);
    runQuery(zI,60,1,352,false,t);
    runQuery(zI,70,2,274,false,t);
    runQuery(zI,73,4,352,false,t);
    runQuery(zI,75,6,268,false,t);
    runQuery(zI,77,0,448,false,t);
    runQuery(zI,84,3,146,false,t);
    runQuery(zI,84,3,386,false,t);
    runQuery(zI,90,3,259,false,t);
    runQuery(zI,97,3,26,false,t);
    runQuery(zI,99,1,392,false,t);

    g1 = new DGraph("experiments/graphs/real/robots/robots.edge");
    zI = new Zou(g1);

    runQuery(zI,101,17,6,true,t);

    return 0;
}
