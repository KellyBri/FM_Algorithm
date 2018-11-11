#ifndef NET_H
#define NET_H
#include <vector>


class NET{
public:
    NET(int);

    void setDegree(int);
    void addNode(bool, int);
    
    int getTerminalSize();
    int getTerminal(int);
    int getBlockSize();
    int getBlock(int);

    void print();

private:
    int ID, degree;
    std::vector<int> blockID;
    std::vector<int> terminalID;
};

#endif