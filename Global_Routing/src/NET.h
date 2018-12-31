#ifndef NET_H
#define NET_H
#include <string>
#include <vector>

class NET{
public:
	NET(int, std::string);


	int getOverflow();
	void subOverflow(int);
	void addOverflow(int);

	float getCongestion();
	void addCongestion(float);
	void subCongestion(float);

	void addPin (int, int);
	void addPath(int, int);
	void ripoff();

	int getPinX(int);
	int getPinY(int);
	int getPinNum();

	int getPathX(int);
	int getPathY(int);
	int getPathNum();

	int getID();
	
	std::string getName();
	

private:
	int id;
	std::string name;

	int pinNum, pathNum;
	std::vector<int> pinX,  pinY;
	std::vector<int> pathX, pathY;

	int overflow;
	float congestion;
};

#endif