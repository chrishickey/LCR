#!/usr/bin/python2.7

import sys, random, math, snap;
from snap import *;
from operator import itemgetter

### Generates a random directed graph with edge labels using preferential attachment
### The labels can follow either a uniform or normal distribution

def exitMain():
    sys.exit(1);

if(len(sys.argv) < 6):
    print("usage python2.7 " + sys.argv[0] + " |V| <avg. degree per node> |L| {uni|norm|exp} {pa|ff|pl|er}");
    exitMain();

try:
    V = int(sys.argv[1]);
    degree = int(sys.argv[2]);
    L = int(sys.argv[3]);
    dist = sys.argv[4];
    model = sys.argv[5];
except:
    print("first three params need to be integers");
    exitMain();

E = degree * V;
if( E > (V*(V-1)) ):
    print("too many edges for the number of nodes");
    exitMain();

Rnd = TRnd()
name = "";
if( model == "er" ):
    UGraph = GenRndGnm(snap.PNGraph, V, E, True, Rnd); isDirected = True; # We use Erdos-Renyi
    name = model + "V" + str(V/1000) + "kD" + str(degree) + "L" + str(L) + dist + ".edge";

if( model == "pa" ):
    name = "V" + str(V/1000) + "kD" + str(degree) + "L" + str(L) + dist + ".edge";
    UGraph = GenPrefAttach(V, degree, Rnd); isDirected = False; # We use preferential attachment

if( model == "ff" ):
    name = model + "V" + str(V/1000) + "k" + str(degree) + "L" + str(L) + dist + ".edge";
    UGraph = GenForestFire(V, 0.4, 0.2); isDirected = True; # We use forest fire

if( model == "pl" ):
    alpha = 1.95;
    UGraph = GenRndPowerLaw (V, alpha); isDirected = False;
    name = model + "V" + str(V/1000) + "ka" + str(alpha) + "L" + str(L) + dist + ".edge";

f = open(name, 'w');

if( L == 0 ):
    exitMain();

random.seed();
mean = int(math.floor(L/2));
sd = int( max(1, math.floor(L/4)) );

dgraph = [];
for EI in UGraph.Edges():
    if dist == "norm":
        label = random.normalvariate(mean, sd);
    if dist == "uni":
        label = random.uniform(0,L);
    if dist == "exp":
        label = random.expovariate( 1.0 / L/1.7 )
        

    label = max(0,label);
    label = min(L-1,label);
    label = int(math.floor(label));

    #label = 2**label;
    #print(label);
    
    if( isDirected == False ):
        direction = random.uniform(0,1);
    else:
        direction = 0.0;
        
    if( direction < 0.5 ):
        triple = (EI.GetSrcNId(), EI.GetDstNId(), label);
    else:
        triple = (EI.GetDstNId(), EI.GetSrcNId(), label);
    dgraph.append(triple);

dgraph = sorted(dgraph,key=lambda x: x[0], reverse=False);

for triple in dgraph:
    (s,t,l) = triple;
    base = 0;
    ss = str(base+s) + " " + str(base+t) + " " + str(l);
    f.write(ss + "\n");

f.close();
