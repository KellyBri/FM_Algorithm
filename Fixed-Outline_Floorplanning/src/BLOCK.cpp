#include <iostream>
#include "BLOCK.h"

BLOCK::BLOCK(int id){
    this->ID = id;
    this->lchild = NULL;
    this->rchild = NULL;
    this->parent = NULL;
    this->place = false;
}

void BLOCK::setWidth(int w){ this->width = w; }
void BLOCK::setHeight(int h){ this->height = h; }
void BLOCK::setPlace(bool p){ this->place = p; }
void BLOCK::setParent(BLOCK *p){ this->parent = p; }

void BLOCK::setChild(bool lr, BLOCK *c){
    if(lr) this->lchild = c;
    else this->rchild = c;
}

void BLOCK::setCoordinate(int x, int y){
    this->x_coordinate = x;
    this->y_coordinate = y;
}

void BLOCK::calcArea(){ this->area = this->width * this->height; }

int BLOCK::getID(){ return this->ID; }
int BLOCK::getWidth(){ return this->width; }
int BLOCK::getHeight(){ return this->height; }
int BLOCK::getArea(){ return this->area; }
int BLOCK::getX(){ return this->x_coordinate; }
int BLOCK::getY(){ return this->y_coordinate; }
bool BLOCK::getPlace(){ return this->place; }
BLOCK *BLOCK::getParent(){ return this->parent; }
BLOCK *BLOCK::getChild(bool lr){
    if(lr) return this->lchild;
    else return this->rchild;
}

void BLOCK::print(){
    std::cout << "sb" << this->ID << "\t";
    std::cout << this->width << "\t" << this->height << "\t" << this->area <<std::endl;
}