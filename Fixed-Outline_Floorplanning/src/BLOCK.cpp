#include <iostream>
#include "BLOCK.h"

BLOCK::BLOCK(int id){
    this->ID = id;
    this->vertex = 0;
}

void BLOCK::setVertexNum(int v){ this->vertex = v; }
void BLOCK::setWidth(int w){ this->width = w; }
void BLOCK::setHeight(int h){ this->height = h; }

void BLOCK::addVertex(int x, int y){
    this->x.push_back(x);
    this->y.push_back(y);
}

void BLOCK::calcWidthHeight(){
    this->width = abs( x[2] - x[1] );
    this->height = abs( y[1] - y[0] );
}

int BLOCK::getVertex(){ return this->vertex; }
int BLOCK::getWidth(){ return this->width; }
int BLOCK::getHeight(){ return this->height; }

void BLOCK::print(){
    std::cout << "sb" << this->ID << " hardrectilinear " << this->vertex <<" ";
    std::cout << this->width << "\t" << this->height <<std::endl;
}