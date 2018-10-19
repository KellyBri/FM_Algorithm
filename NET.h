#ifndef NET_H
#define NET_H
#include <vector>

class NET{
public:
    NET(int);

    void setIsCut(bool);

    int getID();
    int getSize();
    bool getIsCut();
    std::vector<int> getCells();

    void addCell(int);
    void print();

private:
    int ID;
    bool isCut;
    std::vector<int> cells;
};

#endif