#ifndef TERMINAL_H
#define TERMINAL_H

class TERMINAL{
public:
    TERMINAL(int);
    TERMINAL(int, int, int);

    void setCoordinate(int, int);

    void print();
private:
    int ID;
    int x, y;
};

#endif