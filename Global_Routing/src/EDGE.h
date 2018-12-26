#ifndef EDGE_H
#define EDGE_H
#include <vector>

class EDGE{
public:
    EDGE(int);
    void addNet(int);
private:
    int demand;
    const int supply;
    std::vector<int> netID; //net pass this edge
};


#endif