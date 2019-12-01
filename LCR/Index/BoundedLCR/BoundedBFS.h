/* Lucien Valstar (l dot d dot j dot valstar@student.tue.nl) 2016 Master Thesis
* under supervision of
* George Fletcher (g.h.l.fletcher@tue.nl) and Yuichi Yoshida (yyoshida@nii.ac.jp).



*/

#ifndef BBFSINDEX_H
#define BBFSINDEX_H

#include "../../Graph/Graph.h"
#include "BoundedIndex.h"

using namespace graphns;
using namespace boost;
using namespace boundedindexns;

namespace boundedbfsindexns
{
}

class BoundedBFS : public BoundedIndex
{
    private:
        long roundNo;

    public:
        BoundedBFS(Graph* g);
        ~BoundedBFS();

        bool queryShell(B12Query& q);
        bool queryShell(B2Query& q);

        long getLastRoundNo();
};
#endif
