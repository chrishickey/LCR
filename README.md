## Code Example
All Label Combinations is a method that we provide to answer label constraint reachability queries. To improve the query response we applied the Strongly connected components technique. he main part of the code is written in C++ and there are scripts in bash/sh and Python (2.7).

The C++-part has (ALC) our implimentation and a definition of a labelled graph (Graph), a set of indices to answer LCR-queries (Index/Unbounded), a number of tests (tests/Index/Unbounded) and to run the experiments or generate queries for the experiments (experiments/Index/Unbounded). (tmp) contains all outputs of each experiments. (ALC) folder contains implemations related to ALC and ALC+SCC.

The Python-part consists of a script to generate a synthetic graph under a given model with a specified number of vertices.

For example

```python
python2.7 genGraph.py 1000 5 8 exp pa
```

generates a graph with 1000 vertices and roughly 5000 edges under the 'Preferential-Attachment' model. Any edge has one of 8 possible labels. The distribution of the edge labels is an exponential one.

## Installation

You need g++ (>=4.5) and python2.7.

The code can be built by running:

```sh
cd LCRIndexing
./rebuild.sh
```

on Mac or Linux. 

 To test ALC and ALC+SCC you have to run (BFSTest) on (build/default/). To check LandmarkIndexing (LandmarkedIndexTest) the binary is in the same location.

For Windows you should first remove the directories: build and .waf* (where * is an arbitary sequence). Then run:

```python
python2.7 waf configure
python2.7 waf build
```
## Advanced Data Mining 
This work has been done under the supervision of Professor U Kang by Mohammadsadegh Najafi and Chris Hicky for Advanced Data Mining Course at Seoul National University.


