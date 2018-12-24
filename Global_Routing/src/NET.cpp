#include "NET.h"

NET::NET(int id, std::string name){
	this->id = id;
	this->name = name;
	this->pinNum = 0;
}

void NET::addPin(int x, int y){
	++this->pinNum;

	this->x.push_back(x);
	this->y.push_back(y);
}

