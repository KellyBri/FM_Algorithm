#ifndef CELL_H
#define CELL_H
#include <vector>

class CELL{
public:
    CELL();
    CELL(int, int);

    void setID(int);
    void setSize(int);
    void setPre(CELL*);
    void setNext(CELL*);
    void setSet(bool);
    void setLock(bool);

    int getID();
    int getSize();
    int getPin();
    CELL *getNext();
    CELL *getPre();
    bool getSet();
    bool getLock();

    void addNet(int);
    void print();

private:
    int ID;
    int size;
    bool set; //0:A, 1:B
    bool lock;
    CELL* pre, *next;
    std::vector<int> nets;
};

#endif