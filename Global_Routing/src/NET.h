#ifndef NET_H
#define NET_H
#include <string>
#include <vector>

class NET{
public:

	NET(int, std::string);
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
	const int id;
	const std::string name;

	int pinNum, pathNum;
	std::vector<int> pinX,  pinY;
	std::vector<int> pathX, pathY;
};

#endif