#ifndef BLOCK_H
#define BLOCK_H
#include <vector>

class BLOCK{
public:
    BLOCK(int);

    void setVertexNum(int);
    void setWidth(int);
    void setHeight(int);
    void addVertex(int, int);
    void calcWidthHeight();

    int getVertex();
    int getWidth();
    int getHeight();

    void print();

private:
    int ID;
    int vertex;
    int width, height;
    std::vector<int> x, y;
};

#endif