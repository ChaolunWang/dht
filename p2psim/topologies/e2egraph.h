#ifndef __E2EGRAPH_H
#define __E2EGRAPH_H

#include <string>
using namespace std;


class E2EGraph : public Topology {
public:
  E2EGraph(vector<string>*);
  ~E2EGraph();
  
  typedef pair<unsigned, unsigned> Coord;
  virtual void parse(ifstream&);
  virtual Time latency(IPAddress, IPAddress);

private:
  unsigned int _num;
  vector<vector<Time> > _pairwise;
};

#endif //  __E2EGRAPH_H