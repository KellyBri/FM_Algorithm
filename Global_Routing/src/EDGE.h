#ifndef EDGE_H
#define EDGE_H
#include <vector>
#include <cmath>

class EDGE{
public:
    EDGE(int);
    void addNet(int);
    void subDemand(int, int);

    int getDemand();
    int getSupply();
    int getHistory();
    double getCongestion();
    void calCongestion();
    void addHistory();

    int getNetNum();
    int getNetID(int);
    bool overflow();
private:
    int demand, history;
    const int supply;
    double congestion;
    std::vector<int> netID; //net pass this edge
};


#endif