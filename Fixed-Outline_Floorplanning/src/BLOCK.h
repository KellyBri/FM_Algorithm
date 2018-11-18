#ifndef BLOCK_H
#define BLOCK_H

class BLOCK{
public:
    BLOCK(int);

    void setWidth(const int);
    void setHeight(const int);
    void setCoordinate(const int, const int);
    void setParent(BLOCK *);
    void setChild(const bool, BLOCK *);   //true: l, false: r
    void setRotate(const bool);

    void calcArea();
    
    int getID();
    int getWidth();
    int getHeight();
    int getArea();
    int getX();
    int getY();
    bool getRotate();
    BLOCK *getParent();
    BLOCK *getChild(const bool);  //true: l, false: r

    void copy(BLOCK);
    void print();

private:
    int ID;
    int width, height, area;
    int x, y;
    bool rotate;

    BLOCK *lchild, *rchild, *parent;
};

#endif