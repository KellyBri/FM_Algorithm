#ifndef GLOBALROUTE_H
#define GLOBALROUTE_H
#include "BIN.h"
#include "NET.h"
#include <cstring>


class GLOBALROUTE{
public:
	GLOBALROUTE(int, int, int, int);
	void addNet(NET);
	void route();

	int getNetNum();
	NET getNet(int);

private:
	const int gridxNum, gridyNum;
	const int verticalCap, horizontalCap;
	int netNum;
	std::vector<NET> nets;

	// horizontal:horizontal capacity, vertical: vertical capacuty
	// current: label of current net being route
	std::vector< std::vector<BIN *> > bins;
	int **curLabel;

	void phase1();
	void clearGrid(int **);
};

#endif