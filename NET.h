#ifndef NET_H
#define NET_H
#include <vector>

class NET{
public:
    NET(int);

    void setIsCut(bool);

    int getID();
    int getSize();
    int getDistribution(bool);
    bool getIsCut();
    std::vector<int> getCells();

    void setDistribution(int, int);
    void addCell(int);
    void print();

private:
    int ID;
    bool isCut;
    int distribution[2];
    std::vector<int> cells;
};

#endif