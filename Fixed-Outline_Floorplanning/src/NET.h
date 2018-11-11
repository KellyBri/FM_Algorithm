#ifndef NET_H
#define NET_H
#include <vector>


class NET{
public:
    NET(int);

    void setDegree(int);
    void addNode(bool, int);

    void print();

private:
    int ID, degree;
    std::vector<bool> kind; //0: terminal, 1: block
    std::vector<int> nodeID;
};

#endif