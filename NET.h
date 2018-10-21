#ifndef NET_H
#define NET_H
#include <set>

class NET{
public:
    NET(int);

    void setIsCut(bool);
    void setDistribution(int, int);
    void setDistribution(bool, int);

    int getID();
    int getSize();
    int getDistribution(bool);
    bool getIsCut();
    std::set<int> getCells();

    
    void addCell(int);
    void updateCut();
    void print();

private:
    int ID;
    bool isCut;
    int distribution[2];
    std::set<int> cells;
};

#endif