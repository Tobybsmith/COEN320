
#ifndef DD_CUTILS_H_
#define DD_CUTILS_H_

#include <string>

struct Vec3 {
	int x = 0;
	int y = 0;
	int z = 0;
};

enum DISP {
	BLANK = 0,
	HSTR = 1,
	VSTR = 2
};

struct ListEntry {
	int id;
	int xpos;
	int ypos;
	int timetoexit;
	std::string Stringify()
	{
		return std::to_string(id) + "       " + std::to_string(xpos).substr(0, 2) + ", " + std::to_string(ypos).substr(0, 2) +
				"       " + std::to_string(timetoexit) + "     "; //literally the worst code in human history
	}
};

int Remap(int val, int oldMax, int oldMin, int newMax, int newMin)
{
	return (val - oldMin) * (newMax - newMin) / (oldMax - oldMin) + newMin;
}

#endif /* CUTILS_H_ */
