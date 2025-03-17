#ifndef AIRCRAFT_H
#define AIRCRAFT_H
#include "CUtils.h"
#include <string>

class Aircraft {
public:
	Aircraft();
	Aircraft(int aTime, int aId, Vec3f aPos, Vec3f aSpeed);
	virtual ~Aircraft();
	int mEntryTime = -1;
	int mId = -1;
	static int mIndexCounter;
	int mIndex = 0;
	Vec3f mSpeed;
	Vec3f mEntryPos;

	std::string ToString();
};

#endif
