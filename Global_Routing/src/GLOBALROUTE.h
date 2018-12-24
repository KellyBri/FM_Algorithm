#ifndef GLOBALROUTE_H
#define GLOBALROUTE_H
#include "NET.h"

class GLOBALROUTE{
public:
	GLOBALROUTE(int);
	void setGrid(int, int);
	void setCapacity(int, int);
	void addNet(NET);

private:
	int gridxNum, gridyNum;
	int verticalCap, horizontalCap;
	int netNum;
	std::vector<NET> nets;
};

#endif