#ifndef BLOCK_H
#define BLOCK_H

class BLOCK{
public:
    BLOCK(int);

    void setWidth(int);
    void setHeight(int);
    void setCoordinate(int, int);
    void setPlace(bool);
    void setParent(BLOCK *);
    void setChild(bool, BLOCK *);   //true: l, false: r

    void calcArea();
    
    int getID();
    int getWidth();
    int getHeight();
    int getArea();
    int getX();
    int getY();
    bool getPlace();
    BLOCK *getParent();
    BLOCK *getChild(bool);  //true: l, false: r

    void print();

private:
    int ID;
    int width, height, area;
    int x_coordinate, y_coordinate;
    bool place;

    BLOCK *lchild, *rchild, *parent;
};

#endif