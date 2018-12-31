#ifndef GLOBALROUTE_H
#define GLOBALROUTE_H
#include "BIN.h"
#include "NET.h"
#include <cstring>
#include <queue>



class GLOBALROUTE{
public:
	GLOBALROUTE(int, int, int, int);
	~GLOBALROUTE();
	void addNet(NET);
	void route();

	int getNetNum();
	NET getNet(int);

private:
	const int gridxNum, gridyNum;
	const int verticalCap, horizontalCap;
	int netNum;
	std::vector<NET> nets;
	enum{ HORIZONTAL, VERTICAL };
	std::vector< std::vector<BIN *> > bins;
	
	void phase1();
	void phase2();
	void ripoff(int);
	void reRoute(int);
	void clearLabel();
	void printOverflow();
};

#endif