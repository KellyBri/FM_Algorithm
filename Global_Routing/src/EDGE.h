#ifndef EDGE_H
#define EDGE_H
#include <vector>

class EDGE{
public:
    EDGE(int);
    void addNet(int);
    void subDemand(int, int);

    int getDemand();
    int getSupply();
    float getCongestion();
    void calCongestion();

    int getNetNum();
    int getNetID(int);
    bool overflow();
private:
    int demand;
    const int supply;
    float congestion;
    std::vector<int> netID; //net pass this edge
};


#endif