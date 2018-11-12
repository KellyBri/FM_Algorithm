#include <iostream>
#include "BLOCK.h"

BLOCK::BLOCK(const int id){
    this->ID = id;
    this->lchild = NULL;
    this->rchild = NULL;
    this->parent = NULL;
    this->rotate = false;
}

void BLOCK::setWidth(const int w){ this->width = w; }
void BLOCK::setHeight(const int h){ this->height = h; }
void BLOCK::setParent(BLOCK *p){ this->parent = p; }
void BLOCK::setRotate(const bool r){ this->rotate = r; }

void BLOCK::setChild(const bool lr, BLOCK *c){
    if(lr) this->lchild = c;
    else this->rchild = c;
}

void BLOCK::setCoordinate(const int x, const int y){
    this->x = x;
    this->y = y;
}

void BLOCK::calcArea(){ this->area = this->width * this->height; }

int BLOCK::getID(){ return this->ID; }
int BLOCK::getWidth(){ return this->width; }
int BLOCK::getHeight(){ return this->height; }
int BLOCK::getArea(){ return this->area; }
int BLOCK::getX(){ return this->x; }
int BLOCK::getY(){ return this->y; }
bool BLOCK::getRotate(){ return this->rotate; }
BLOCK *BLOCK::getParent(){ return this->parent; }
BLOCK *BLOCK::getChild(const bool lr){
    if(lr) return this->lchild;
    else return this->rchild;
}

void BLOCK::print(){
    std::cout << "sb" << this->ID << "\t";
    std::cout << this->width << "\t" << this->height << "\t";
    // std::cout << "\t" << this->area << std::endl;
    std::cout << "(" << this->x << ", " << this->y << ")" << "\t";
    if(this->rotate) std::cout << "Rotated" << std::endl;
    else std::cout << "Unrotated" << std::endl;
}