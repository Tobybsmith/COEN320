#ifndef AIRCRAFT_H
#define AIRCRAFT_H
#include "CUtils.h"

class Aircraft {
public:
	Aircraft();
	virtual ~Aircraft();
	int mEntryTime = -1;
	int mId = -1;
	static int mIndexCounter = 0;
	int mIndex = 0;
	Vec3f mSpeed;
	Vec3f mEntryPos;

	std::string ToString();
};

#endif
